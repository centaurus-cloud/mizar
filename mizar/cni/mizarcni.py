# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Authors.

# Authors: Sherif Abdelwahab <@zasherif>
#          Phu Tran          <@phudtran>

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:The above copyright
# notice and this permission notice shall be included in all copies or
# substantial portions of the Software.THE SOFTWARE IS PROVIDED "AS IS",
# WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
# TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
# FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
# TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
# THE USE OR OTHER DEALINGS IN THE SOFTWARE.

import subprocess
import sys
import logging
from logging.handlers import SysLogHandler
import json
import os
import fs
from mizar.common.common import *
from mizar.daemon.interface_service import InterfaceServiceClient
from mizar.proto.interface_pb2 import *
from pyroute2 import IPRoute, NetNS

logger = logging.getLogger('mizarcni')
handler = logging.FileHandler('/tmp/mizarcni.log')
logger.addHandler(handler)
logger.setLevel(logging.INFO)


class Cni:
    def __init__(self):
        stdin = ''.join(sys.stdin.readlines())
        self.command = os.environ.get("CNI_COMMAND")  # ADD | DEL | VERSION
        self.container_id = os.environ.get("CNI_CONTAINERID")
        self.netns = os.environ.get("CNI_NETNS")
        self.interface = os.environ.get("CNI_IFNAME")
        self.cni_path = os.environ.get("CNI_PATH")
        self.cni_args = os.environ.get("CNI_ARGS")
        self.cni_args_dict = {}
        if self.command == "VERSION":
            return

        netns_folder = "/var/run/netns/"
        if not self.netns.startswith(netns_folder):
            dst_netns = self.netns.replace('/', '_')
            dst_netns_path = os.path.join(netns_folder, dst_netns)
            if self.command == "ADD":
                errorcode = bindmount_netns(self.netns, dst_netns_path)
                if errorcode != 0:
                    logger.error("failed to bind mount {} to {}: error code {}".format(self.netns, dst_netns_path, errorcode))
                    raise OSError("failed to bind mount netns {} to {}, error code: {}".format(self.netns, dst_netns_path, errorcode))
            self.netns = dst_netns_path

        self.cni_args_dict = dict(i.split("=")
                                  for i in self.cni_args.split(";"))
        self.k8s_namespace = self.cni_args_dict.get('K8S_POD_NAMESPACE', '')
        self.k8s_pod_name = self.cni_args_dict.get('K8S_POD_NAME', '')

        config_json = json.loads(stdin)

        # expected parameters in the CNI specification:
        self.cni_version = config_json["cniVersion"]
        self.network_name = config_json["name"]
        self.plugin = config_json["type"]

        # TODO: parse 'Arktos specific' CNI_ARGS
        self.k8s_pod_tenant = self.cni_args_dict.get('K8S_POD_TENANT', '')

        self.pod_id = PodId(
            k8s_pod_name=self.k8s_pod_name,
            k8s_namespace=self.k8s_namespace,
            k8s_pod_tenant=self.k8s_pod_tenant
        )

        self.interface_id = InterfaceId(
            pod_id=self.pod_id, interface=self.interface)

        self.iproute = IPRoute()

    def __del__(self):
        if self.command != "VERSION":
            logger.info("Closing IPRoute")
            self.iproute.close()

    def run(self):
        if len(self.cni_args_dict) != 0:
            logging.info("CNI ARGS {}".format(self.cni_args_dict))
        val = "Unsuported cni command!"
        switcher = {
            'ADD': self.do_add,
            'DEL': self.do_delete,
            'GET': self.do_get,
            'VERSION': self.do_version
        }

        func = switcher.get(self.command, lambda: "Unsuported cni command")
        if func:
            func()
        print(val)
        exit(1)

    def do_add(self):

        # Construct a CniParameters grpc message
        param = CniParameters(pod_id=self.pod_id,
                              netns=self.netns,
                              interface=self.interface)

        # Consume new (and existing) interfaces for this Pod
        interfaces = InterfaceServiceClient(
            "localhost").ConsumeInterfaces(param).interfaces

        if len(interfaces) == 0:
            logger.error("No interfaces found for {}".format(self.pod_id))
            exit(1)

        result = {
            "cniVersion": self.cni_version,
            "interfaces": [],
            "ips": []
        }

        # Construct the result string
        idx = 0
        for interface in interfaces:
            self.activate_interface(interface)
            itf = {
                "name": interface.interface_id.interface,
                "mac": interface.address.mac,
                "sandbox": self.netns
            }
            ip = {
                "version": interface.address.version,
                "address": "{}/{}".format(interface.address.ip_address, interface.address.ip_prefix),
                "gateway": interface.address.gateway_ip,
                "interface": idx
            }
            result['interfaces'].append(itf)
            result['ips'].append(ip)
            idx += 1

        print(json.dumps(result))
        exit(0)

    def activate_interface(self, interface):
        """
        moves the interface to the CNI netnt, rename it, set the IP address, and
        the GW.
        """

        _, netns = os.path.split(self.netns)
        iproute_ns = NetNS(netns)
        veth_index = get_iface_index(interface.veth.name, self.iproute)

        actived_idxs = iproute_ns.link_lookup(operstate="UP")
        if (veth_index in actived_idxs):
            return

        logger.info("Move interface {}/{} to netns {}".format(
            interface.veth.name, veth_index, netns))
        self.iproute.link('set', index=veth_index, net_ns_fd=netns)

        # configure and activate interfaces inside the netns
        iproute_ns.link('set', index=veth_index, ifname=self.interface)
        lo_idx = iproute_ns.link_lookup(ifname="lo")[0]
        iproute_ns.link('set', index=lo_idx, state='up')

        iproute_ns.link('set', index=veth_index, state='up')

        iproute_ns.addr('add', index=veth_index, address=interface.address.ip_address,
                        prefixlen=int(interface.address.ip_prefix))
        iproute_ns.route('add', gateway=interface.address.gateway_ip)

        # Disable TSO and checksum offload as xdp currently does not support
        logger.info("Disable tso for pod")
        cmd = "ip netns exec {} ethtool -K {} tso off gso off ufo off".format(
            netns, "eth0")
        rc, text = run_cmd(cmd)
        logger.info("Executed cmd {} tso rc: {} text {}".format(cmd, rc, text))
        logger.info("Disable rx tx offload for pod")
        cmd = "ip netns exec {} ethtool --offload {} rx off tx off".format(
            netns, "eth0")
        rc, text = run_cmd(cmd)
        logger.info("Executed cmd {} rc: {} text {}".format(cmd, rc, text))

    def do_delete(self):
        param = CniParameters(pod_id=self.pod_id,
                              netns=self.netns,
                              interface=self.interface)
        InterfaceServiceClient(
            "localhost").DeleteInterface(param)
        _, netns = os.path.split(self.netns)
        if len(netns) > 0:
            subprocess.run(["ip", "netns", "delete", netns])
        exit(0)

    def do_get(self):
        logger.info("CNI get is not implemented")
        print("")
        exit(0)

    def do_version(self):
        val, status = json.dumps({'cniVersion': '0.3.1', "supportedVersions": [
            "0.2.0", "0.3.0", "0.3.1"]}), 0
        logger.info("server's version is {}".format(val))
        print(val)
        exit(status)


cni = Cni()
cni.run()
