import logging
import sys
import os
import subprocess
from mizar.proto.interface_pb2 import *
from mizar.proto.interface_pb2_grpc import InterfaceServiceServicer, InterfaceServiceStub
import time
import grpc
import json
import fs
import uuid
from mizar.common.common import *
from logging.handlers import SysLogHandler
from concurrent import futures
import threading
from google.protobuf import empty_pb2
from mizar.common.rpc import TrnRpc
from mizar.common.constants import *
from pyroute2 import IPRoute, NetNS
import queue
import ipaddress

logger = logging.getLogger('mizar_interface_service')
handler = SysLogHandler(address='/dev/log')
logger.addHandler(handler)
logger = logging.getLogger()

CONSUME_INTERFACE_TIMEOUT = 5


class InterfaceServer(InterfaceServiceServicer):

    def __init__(self):
        self.interfaces_q = queue.Queue()  # Used for Produce/Consume sync
        self.iproute = IPRoute()
        self.interfaces = {}  # In-memory tracking for Pod interfaces
        self.queued_pods = set()  # A set of pods, with queued interfaces (to be consumed)
        self.interfaces_lock = threading.Lock()

        cmd = 'ip addr show eth0 | grep "inet\\b" | awk \'{print $2}\' | cut -d/ -f1'
        r = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE)
        self.droplet_ip = r.stdout.read().decode().strip()

        cmd = 'ip addr show eth0 | grep "link/ether\\b" | awk \'{print $2}\' | cut -d/ -f1'
        r = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE)
        self.droplet_mac = r.stdout.read().decode().strip()

        cmd = 'hostname'
        r = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE)
        self.droplet_name = r.stdout.read().decode().strip()

        self.itf = 'eth0'
        self.rpc = LocalTransitRpc('127.0.0.1', self.droplet_mac)

    def __del__(self):
        self.iproute.close()

    def InitializeInterfaces(self, request, context):
        """
        Called by the endpoints operator to initilaize the interface and
        allocate its mac address
        """
        interfaces = request
        for interface in interfaces.interfaces:
            self._CreateInterface(interface)
            logger.debug("Allocated mac {}".format(interface.address.mac))
        return interfaces

    def _CreateInterface(self, interface):
        """
        Type specific interface creation
        """
        if interface.interface_type == InterfaceType.veth:
            return self._CreateVethInterface(interface)

    def _CreateVethInterface(self, interface):
        """
        Creates a veth interface
        """
        veth_name = interface.veth.name
        veth_peer = interface.veth.peer
        veth_index = get_iface_index(veth_name, self.iproute)

        if veth_index == -1:
            self.iproute.link('add', ifname=veth_name,
                              peer=veth_peer, kind='veth')
            veth_index = get_iface_index(veth_name, self.iproute)

        # Update the mac address with the interface address
        address = InterfaceAddress(
            version=interface.address.version,
            ip_address=interface.address.ip_address,
            ip_prefix=interface.address.ip_prefix,
            gateway_ip=interface.address.gateway_ip,
            mac=get_iface_mac(veth_index, self.iproute),
            tunnel_id=interface.address.tunnel_id
        )
        interface.address.CopyFrom(address)

    def ProduceInterfaces(self, request, context):
        """
        Called by the endpoints operator to program the transit Agent and
        prepare the interface for consumption.
        """
        interfaces = request
        for interface in interfaces.interfaces:
            self._QueueInterface(interface)
        return interfaces

    def _QueueInterface(self, interface):
        pod_name = get_pod_name(interface.interface_id.pod_id)
        logger.info("Producing interface {}".format(interface))
        with self.interfaces_lock:
            # Append the interface to the pod's interfaces (important in
            # multi-interfaces case)
            if pod_name not in self.interfaces:
                self.interfaces[pod_name] = []
            self.interfaces[pod_name].append(interface)

            # Move the pod_name into interfaces_q. This tells the consume
            # function that the pod has queued interfaces that can be consumed
            if pod_name not in self.queued_pods:
                self.interfaces_q.put(pod_name)
                self.queued_pods.add(pod_name)

        # Change interface status from init to queued.
        interface.status = InterfaceStatus.queued

    def _ConsumeInterfaces(self, pod_name, cni_params):
        """
        Remove the pod from the interfaces_q and provision the interface
        (program the transit agent)
        """
        with self.interfaces_lock:
            if pod_name in self.queued_pods:
                self.queued_pods.remove(pod_name)
            interfaces = self.interfaces.get(pod_name, [])
            for interface in interfaces:
                if interface.status == InterfaceStatus.queued:
                    self._ProvisionInterface(interface, cni_params)
                    interface.status = InterfaceStatus.consumed

        interfaces = InterfacesList(interfaces=interfaces)
        logger.info("Consumed {}".format(interfaces))
        return interfaces

    def _ProvisionInterface(self, interface, cni_params):
        """
        Provision the interface according to its type
        """
        if interface.interface_type == InterfaceType.veth:
            self._ProvisionVethInterface(interface, cni_params)

    def _ProvisionVethInterface(self, interface, cni_params):
        """
        Provision a veth interface.
        """
        # Finalize all interface configuration. All main interface configuration
        # will be in the CNI. All the peer interface configuration, will be here
        # Network namespace operations (Move these to the CNI)

        veth_peer_index = get_iface_index(interface.veth.peer, self.iproute)
        self.iproute.link('set', index=veth_peer_index, state='up', mtu=9000)

        # Configure the Transit Agent
        self._ConfigureTransitAgent(interface)

    def _ConfigureTransitAgent(self, interface):
        """
        Load the Transit Agent XDP program, program all the bouncer substrate,
        update the agent metadata and endpoint.
        """
        self.rpc.load_transit_agent_xdp(interface)

        for bouncer in interface.bouncers:
            self.rpc.update_agent_substrate_ep(
                interface.veth.peer, bouncer.ip_address, bouncer.mac)
        self.rpc.update_agent_metadata(interface)
        self.rpc.update_ep(interface)
        self.rpc.update_packet_metadata(interface.veth.peer, interface)

    def ConsumeInterfaces(self, request, context):
        """
        Called by the CNI to consume queued interfaces for a specific POD
        """
        cni_params = request
        requested_pod_id = cni_params.pod_id
        requested_pod_name = get_pod_name(requested_pod_id)
        logger.info("Consume Interfaces {}".format(request))
        logger.debug(
            "Consuming interfaces for pod {}".format(requested_pod_name))
        start = time.time()

        # The following is a synchronization mechanism to make sure the
        # CNI calls _ConsumeInterfaces after the interfaces got produced.
        # In Arktos, this may be skipped because the Kubelet will only
        # invoke CNI after the Pod operator marks the pod's network ready

        while True:
            try:
                queued_pod_name = self.interfaces_q.get(
                    timeout=CONSUME_INTERFACE_TIMEOUT)
            except:
                break

            if queued_pod_name == requested_pod_name:
                # Interfaces for the Pod has been produced
                return self._ConsumeInterfaces(queued_pod_name, cni_params)

            # Update the wait time and break the wait if necessary
            self.interfaces_q.put(queued_pod_name)
            now = time.time()

            if now - start >= CONSUME_INTERFACE_TIMEOUT:
                break

        # If we are here, the endpoint operator has not produced any interfaces
        # for the Pod. Typically the CNI will retry to consume the interface.
        logger.error("Timeout, no new interface to consume!")
        return self._ConsumeInterfaces(requested_pod_name, cni_params)

    def _DeleteVethInterface(self, interface):
        """
        Delete a veth interface
        """
        veth_peer_index = get_iface_index(interface.veth.peer, self.iproute)
        self.rpc.unload_transit_agent_xdp(interface)
        self.iproute.link('del', index=veth_peer_index)

    def DeleteInterface(self, request, context):
        """
        Delete network interfaces for a pod
        """
        cni_params = request
        pod_name = get_pod_name(cni_params.pod_id)
        pod_interfaces = self.interfaces.get(pod_name, [])
        iface = cni_params.interface
        logger.info("Deleting interfaces for pod {} with interfaces {}".format(
            pod_name, pod_interfaces))

        for interface in pod_interfaces:
            self.interfaces[pod_name].remove(interface)
            if iface == interface and interface.interface_type == InterfaceType.veth:
                logger.info("Deleting interface: {}".format(iface))
                self._DeleteVethInterface(interface)
                logger.info("Removed {}".format(interface))
        return empty_pb2.Empty()

    def ActivateHostInterface(self, request, context):
        interface = request

        # Provision host veth interface and load transit xdp agent.
        veth_peer_index = get_iface_index(interface.veth.peer, self.iproute)
        self.iproute.link('set', index=veth_peer_index, state='up', mtu=9000)
        self.rpc.load_transit_agent_xdp(interface)

        veth_index = get_iface_index(interface.veth.name, self.iproute)
        # configure and activate interfaces
        self.iproute.link('set', index=veth_index,
                          ifname=interface.veth.name)
        self.iproute.link('set', index=veth_index, state='up')

        # Setting the prefix length of the veth device to a large range adds a route to
        # the host. Since this device has the same IP address as the host's main
        # interface, this will affect the host network causing host traffic to be routed
        # to the wrong interface. Here we set the prefix length to 32.
        self.iproute.addr('add', index=veth_index, address=interface.address.ip_address,
                          prefixlen=int(interface.address.ip_prefix))

        self.iproute.route('add', dst=OBJ_DEFAULTS.default_net_ip,
                           mask=int(OBJ_DEFAULTS.default_net_prefix), oif=veth_index)

        # Disable TSO and checksum offload as xdp currently does not support
        logger.info("Disable tso for host ep")
        cmd = "nsenter -t 1 -m -u -n -i ethtool -K {} tso off gso off ufo off".format(
            interface.veth.name)
        rc, text = run_cmd(cmd)
        logger.info("Disabled tso rc:{} text{}".format(rc, text))
        logger.info("Disable rx tx offload for host ep")
        cmd = "nsenter -t 1 -m -u -n -i ethtool --offload {} rx off tx off".format(
            interface.veth.name)
        rc, text = run_cmd(cmd)
        logger.info(
            "Disabled rx tx offload for host ep rc: {} text: {}".format(rc, text))
        return interface


class InterfaceServiceClient():
    def __init__(self, ip):
        self.channel = grpc.insecure_channel('{}:50051'.format(ip))
        self.stub = InterfaceServiceStub(self.channel)

    def InitializeInterfaces(self, interfaces_list):
        resp = self.stub.InitializeInterfaces(interfaces_list)
        return resp

    def ProduceInterfaces(self, interfaces_list):
        resp = self.stub.ProduceInterfaces(interfaces_list)
        return resp

    def ConsumeInterfaces(self, pod_id):
        resp = self.stub.ConsumeInterfaces(pod_id)
        return resp

    def DeleteInterface(self, interfaces_list):
        resp = self.stub.DeleteInterface(interfaces_list)
        return resp

    def ActivateHostInterface(self, interface):
        resp = self.stub.ActivateHostInterface(interface)
        return resp


class LocalTransitRpc:
    def __init__(self, ip, mac, itf='eth0', benchmark=False):
        self.ip = ip
        self.mac = mac
        self.phy_itf = itf

        # transitd cli commands
        self.trn_cli = f'''nsenter -t 1 -m -u -n -i /trn_bin/transit -s {self.ip} '''
        self.trn_cli_load_transit_xdp = f'''{self.trn_cli} load-transit-xdp -i {self.phy_itf} -j'''
        self.trn_cli_unload_transit_xdp = f'''{self.trn_cli} unload-transit-xdp -i {self.phy_itf} -j'''
        self.trn_cli_update_vpc = f'''{self.trn_cli} update-vpc -i {self.phy_itf} -j'''
        self.trn_cli_get_vpc = f'''{self.trn_cli} get-vpc -i {self.phy_itf} -j'''
        self.trn_cli_delete_vpc = f'''{self.trn_cli} delete-vpc -i {self.phy_itf} -j'''
        self.trn_cli_update_net = f'''{self.trn_cli} update-net -i {self.phy_itf} -j'''
        self.trn_cli_get_net = f'''{self.trn_cli} get-net -i {self.phy_itf} -j'''
        self.trn_cli_delete_net = f'''{self.trn_cli} delete-net -i {self.phy_itf} -j'''
        self.trn_cli_update_ep = f'''{self.trn_cli} update-ep -i {self.phy_itf} -j'''
        self.trn_cli_get_ep = f'''{self.trn_cli} get-ep -i {self.phy_itf} -j'''
        self.trn_cli_delete_ep = f'''{self.trn_cli} delete-ep -i {self.phy_itf} -j'''
        self.trn_cli_load_pipeline_stage = f'''{self.trn_cli} load-pipeline-stage -i {self.phy_itf} -j'''
        self.trn_cli_unload_pipeline_stage = f'''{self.trn_cli} unload-pipeline-stage -i {self.phy_itf} -j'''

        self.trn_cli_load_transit_agent_xdp = f'''{self.trn_cli} load-agent-xdp'''
        self.trn_cli_unload_transit_agent_xdp = f'''{self.trn_cli} unload-agent-xdp'''
        self.trn_cli_update_agent_metadata = f'''{self.trn_cli} update-agent-metadata'''
        self.trn_cli_get_agent_metadata = f'''{self.trn_cli} get-agent-metadata'''
        self.trn_cli_delete_agent_metadata = f'''{self.trn_cli} delete-agent-metadata'''
        self.trn_cli_update_agent_ep = f'''{self.trn_cli} update-agent-ep'''
        self.trn_cli_get_agent_ep = f'''{self.trn_cli} get-agent-ep'''
        self.trn_cli_delete_agent_ep = f'''{self.trn_cli} delete-agent-ep'''
        self.trn_cli_update_packet_metadata = f'''{self.trn_cli} update-packet-metadata'''
        self.trn_cli_delete_packet_metadata = f'''{self.trn_cli} delete-packet-metadata'''

        if benchmark:
            self.xdp_path = "/trn_xdp/trn_transit_xdp_ebpf.o"
            self.agent_xdp_path = "/trn_xdp/trn_agent_xdp_ebpf.o"
        else:
            self.xdp_path = "/trn_xdp/trn_transit_xdp_ebpf_debug.o"
            self.agent_xdp_path = "/trn_xdp/trn_agent_xdp_ebpf_debug.o"

    def get_substrate_ep_json(self, ip, mac):
        jsonconf = {
            "tunnel_id": "0",
            "ip": ip,
            "eptype": "0",
            "mac": mac,
            "veth": "",
            "remote_ips": [""],
            "hosted_iface": ""
        }
        jsonconf = json.dumps(jsonconf)
        return jsonconf

    def load_transit_agent_xdp(self, interface):
        itf = interface.veth.peer
        agent_pcap_file = itf + '.pcap'
        jsonconf = {
            "xdp_path": self.agent_xdp_path,
            "pcapfile": agent_pcap_file,
            "xdp_flag": CONSTANTS.XDP_GENERIC
        }
        jsonconf = json.dumps(jsonconf)
        cmd = f'''{self.trn_cli_load_transit_agent_xdp} -i \'{itf}\' -j \'{jsonconf}\' '''
        logger.info("load_transit_agent_xdp: {}".format(cmd))
        returncode, text = run_cmd(cmd)
        logger.info(
            "load_transit_agent_xdp returns {} {}".format(returncode, text))

    def unload_transit_agent_xdp(self, interface):
        itf = interface.veth.peer
        jsonconf = '\'{}\''
        cmd = f'''{self.trn_cli_unload_transit_agent_xdp} -i \'{itf}\' -j {jsonconf} '''
        logger.info("unload_transit_agent_xdp: {}".format(cmd))
        returncode, text = run_cmd(cmd)
        logger.info(
            "unload_transit_agent_xdp returns {} {}".format(returncode, text))

    def update_ep(self, interface):
        peer = interface.veth.peer
        droplet_ip = interface.droplet.ip_address
        jsonconf = {
            "tunnel_id": interface.address.tunnel_id,
            "ip": interface.address.ip_address,
            "eptype": str(CONSTANTS.TRAN_SIMPLE_EP),
            "mac": interface.address.mac,
            "veth": interface.veth.name,
            "remote_ips": [droplet_ip],
            "hosted_iface": peer
        }
        jsonconf = json.dumps(jsonconf)
        cmd = f'''{self.trn_cli_update_ep} \'{jsonconf}\''''
        logger.info("update_ep: {}".format(cmd))
        returncode, text = run_cmd(cmd)
        logger.info("returns {} {}".format(returncode, text))

    def update_agent_substrate_ep(self, veth_peer, ip, mac):
        itf = veth_peer
        jsonconf = self.get_substrate_ep_json(ip, mac)
        cmd = f'''{self.trn_cli_update_agent_ep} -i \'{itf}\' -j \'{jsonconf}\''''
        logger.info("update_agent_substrate_ep: {}".format(cmd))
        returncode, text = run_cmd(cmd)
        logger.info(
            "update_agent_substrate_ep returns {} {}".format(returncode, text))

    def update_packet_metadata(self, veth_peer, interface):
        itf = veth_peer
        jsonconf = {
            "tunnel_id": interface.address.tunnel_id,
            "ip": interface.address.ip_address,
            "pod_label_value": interface.pod_label_value,
            "namespace_label_value": interface.namespace_label_value
        }
        jsonconf = json.dumps(jsonconf)
        cmd = f'''{self.trn_cli_update_packet_metadata} -i \'{itf}\' -j \'{jsonconf}\''''
        logger.info("update_packet_metadata: {}".format(cmd))
        returncode, text = run_cmd(cmd)
        logger.info(
            "update_packet_metadata returns {} {}".format(returncode, text))

    def update_agent_metadata(self, interface):
        itf = interface.veth.peer
        netip = str(ipaddress.ip_interface(
            interface.address.ip_address + '/' + interface.address.ip_prefix).network.network_address)
        bouncers = []
        for bouncer in interface.bouncers:
            bouncers.append(bouncer.ip_address)
        jsonconf = {
            "ep": {
                "tunnel_id": interface.address.tunnel_id,
                "ip": interface.address.ip_address,
                "eptype": str(CONSTANTS.TRAN_SIMPLE_EP),
                "mac": interface.address.mac,
                "veth": interface.veth.name,
                "remote_ips": [interface.droplet.ip_address],
                "hosted_iface": 'eth0'
            },
            "net": {
                "tunnel_id": interface.address.tunnel_id,
                "nip":  netip,
                "prefixlen":  interface.address.ip_prefix,
                "switches_ips": bouncers
            },
            "eth": {
                "ip": interface.droplet.ip_address,
                "mac": interface.droplet.mac,
                "iface": 'eth0'
            }
        }
        jsonconf = json.dumps(jsonconf)
        cmd = f'''{self.trn_cli_update_agent_metadata} -i \'{itf}\' -j \'{jsonconf}\''''
        logger.info("update_agent_metadata: {}".format(cmd))
        returncode, text = run_cmd(cmd)
        logger.info(
            "update_agent_metadata returns {} {}".format(returncode, text))
