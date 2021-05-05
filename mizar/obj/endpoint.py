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

import logging
import ipaddress
from mizar.common.rpc import TrnRpc
from mizar.common.constants import *
from mizar.common.common import *
from mizar.obj.data_networkpolicy import *

logger = logging.getLogger()


class Endpoint:
    def __init__(self, name, obj_api, opr_store, spec=None):
        self.name = name
        self.obj_api = obj_api
        self.store = opr_store
        # Initial values all none
        self.vpc = ""
        self.net = ""
        self.vni = ""
        self.status = ""
        self.gw = ""
        self.ip = ""
        self.prefix = ""
        self.mac = ""
        self.type = ""
        self.droplet = ""
        self.droplet_ip = ""
        self.droplet_mac = ""
        self.droplet_eth = 'eth0'
        self.droplet_obj = None
        self.veth_peer = ""
        self.veth_name = ""
        self.netns = ""
        self.container_id = ""
        self.local_id = ""
        self.veth_index = ""
        self.veth_peer_index = ""
        self.veth_peer_mac = ""
        self.cnidelay = ""
        self.provisiondelay = ""
        self.bouncers = {}
        self.backends = []
        self.ports = []
        self.pod = ""
        self.deleted = False
        self.interface = None
        self.ingress_networkpolicies = []
        self.egress_networkpolicies = []
        self.data_for_networkpolicy = {}
        if spec is not None:
            self.set_obj_spec(spec)

    @property
    def rpc(self):
        return TrnRpc(self.droplet_ip, self.droplet_mac)

    def get_nip(self):
        if self.type == OBJ_DEFAULTS.ep_type_host:
            return OBJ_DEFAULTS.default_net_ip
        ip = ipaddress.ip_interface(self.ip + '/' + self.prefix)
        return str(ip.network.network_address)

    def get_prefix(self):
        return self.prefix

    def get_bouncers_ips(self):
        bouncers = [b.ip for b in self.bouncers.values()]
        return bouncers

    def get_interface(self):
        return self.interface

    def get_obj_spec(self):
        self.obj = {
            "type": self.type,
            "status": self.status,
            "vpc": self.vpc,
            "net": self.net,
            "ip": self.ip,
            "gw": self.gw,
            "mac": self.mac,
            "vni": self.vni,
            "droplet": self.droplet,
            "prefix": self.prefix,
            "itf": self.veth_name,
            "veth": self.veth_peer,
            "netns": self.netns,
            "hostip": self.droplet_ip,
            "hostmac": self.droplet_mac,
            "cnidelay": self.cnidelay,
            "provisiondelay": self.provisiondelay,
            "pod": self.pod
        }

        return self.obj

    def set_obj_spec(self, spec):
        self.type = get_spec_val('type', spec)
        self.status = get_spec_val('status', spec)
        self.vpc = get_spec_val('vpc', spec)
        self.net = get_spec_val('net', spec)
        self.ip = get_spec_val('ip', spec)
        self.gw = get_spec_val('gw', spec)
        self.mac = get_spec_val('mac', spec)
        self.vni = get_spec_val('vni', spec)
        self.droplet = get_spec_val('droplet', spec)
        self.prefix = get_spec_val('prefix', spec)
        self.veth_name = get_spec_val('itf', spec)
        self.veth_peer = get_spec_val('veth', spec)
        self.netns = get_spec_val('netns', spec)
        self.droplet_ip = get_spec_val('hostip', spec)
        self.droplet_mac = get_spec_val('hostmac', spec)
        self.cnidelay = get_spec_val('cnidelay', spec)
        self.provisiondelay = get_spec_val('provisiondelay', spec)
        self.pod = get_spec_val('pod', spec)

    def set_interface(self, interface):
        self.interface = interface

    def set_cnidelay(self, delay):
        self.cnidelay = delay

    def set_provisiondelay(self, delay):
        self.provisiondelay = delay

    def get_name(self):
        return self.name

    def get_plural(self):
        return "endpoints"

    def get_kind(self):
        return "Endpoint"

    def store_update_obj(self):
        if self.store is None:
            return
        self.store.update_ep(self)

    def store_delete_obj(self):
        if self.store is None:
            return
        self.store.delete_ep(self.name)

    # K8s APIs
    # This function does a store update
    def create_obj(self):
        return kube_create_obj(self)

    # This function does a store update
    def update_obj(self):
        return kube_update_obj(self)

    def delete_obj(self):
        if self.deleted:
            return
        self.deleted = True
        return kube_delete_obj(self)

    def watch_obj(self, watch_callback):
        return kube_watch_obj(self, watch_callback)

    # Setters
    def set_vpc(self, vpc):
        self.vpc = vpc

    def set_net(self, net):
        self.net = net

    def set_vni(self, vni):
        self.vni = vni

    def set_status(self, status):
        self.status = status

    def set_gw(self, gw):
        self.gw = gw

    def set_ip(self, ip):
        self.ip = ip

    def set_prefix(self, prefix):
        self.prefix = prefix

    def set_mac(self, mac):
        self.mac = mac

    def set_type(self, eptype):
        self.type = eptype

    def set_droplet(self, droplet):
        self.droplet = droplet

    def set_droplet_ip(self, droplet_ip):
        self.droplet_ip = droplet_ip

    def set_droplet_mac(self, droplet_mac):
        self.droplet_mac = droplet_mac

    def set_droplet_obj(self, droplet_obj):
        self.droplet_obj = droplet_obj

    def set_veth_peer(self, veth_peer):
        self.veth_peer = veth_peer

    def set_veth_name(self, veth_name):
        self.veth_name = veth_name

    def set_netns(self, netns):
        self.netns = netns

    def set_container_id(self, container_id):
        self.container_id = container_id

    def set_local_id(self, local_id):
        self.local_id = local_id

    def set_veth_index(self, veth_index):
        self.veth_index = veth_index

    def set_veth_peer_index(self, veth_peer_index):
        self.veth_peer_index = veth_peer_index

    def set_veth_peer_mac(self, veth_peer_mac):
        self.veth_peer_mac = veth_peer_mac

    def set_pod(self, pod):
        self.pod = pod

    def update_bouncers(self, bouncers, add=True):
        for bouncer in bouncers.values():
            if add:
                self.bouncers[bouncer.name] = bouncer
                self.update_agent_substrate(self, bouncer)
            else:
                self.bouncers.pop(bouncer.name)
                self.droplet_obj.delete_agent_substrate(self, bouncer)
        self.rpc.update_agent_metadata(self)
        self.droplet_obj.update_ep(self.name, self)

    def update_bouncers_list(self, bouncers, add=True):
        for bouncer in bouncers.values():
            if add:
                logger.info("Updating ep {} list with bouncers {}".format(
                    self.name, bouncer.name))
                self.bouncers[bouncer.name] = bouncer
            else:
                self.bouncers.pop(bouncer.name)

    def set_backends(self, backends):
        self.backends = backends

    def set_ports(self, ports):
        self.ports = ports

    def get_veth_peer(self):
        return self.veth_peer

    def get_veth_name(self):
        return self.veth_name

    def get_tunnel_id(self):
        return str(self.vni)

    def get_ip(self):
        return str(self.ip)

    def get_gw(self):
        return str(self.gw)

    def get_eptype(self):
        if self.type == OBJ_DEFAULTS.ep_type_simple or self.type == OBJ_DEFAULTS.ep_type_host:
            return str(CONSTANTS.TRAN_SIMPLE_EP)
        if self.type == OBJ_DEFAULTS.ep_type_scaled:
            return str(CONSTANTS.TRAN_SCALED_EP)
        if self.type == OBJ_DEFAULTS.ep_type_gateway:
            return str(CONSTANTS.TRAN_GATEWAY_EP)

    def get_mac(self):
        return str(self.mac)

    def get_remote_ips(self):
        if self.type == OBJ_DEFAULTS.ep_type_simple or self.type == OBJ_DEFAULTS.ep_type_host:
            remote_ips = [self.droplet_ip]
        if self.type == OBJ_DEFAULTS.ep_type_scaled or self.type == OBJ_DEFAULTS.ep_type_gateway:
            remote_ips = list(self.backends)
        return remote_ips

    def get_remote_ports(self):
        return [str(self.ports[port]) for port in self.ports]

    def get_frontend_ports(self):
        return [port.split(",")[0] for port in self.ports]

    def get_port_protocols(self):
        return [port.split(",")[1] for port in self.ports]

    def get_remote_macs(self):
        remote_macs = [self.droplet_mac]
        return remote_macs

    def get_droplet_ip(self):
        return self.droplet_ip

    def get_droplet_mac(self):
        return self.droplet_mac

    def get_ingress_networkpolicies(self):
        return self.ingress_networkpolicies

    def get_egress_networkpolicies(self):
        return self.egress_networkpolicies

    def add_ingress_networkpolicy(self, policy_name):
        if policy_name in self.ingress_networkpolicies:
            return
        self.ingress_networkpolicies.append(policy_name)
        self.ingress_networkpolicies.sort()
        if len(self.ingress_networkpolicies) == 1 and self.ip != "":
            self.update_network_policy_enforcement_map_ingress()
        self.store_update_obj()

    def add_egress_networkpolicy(self, policy_name):
        if policy_name in self.egress_networkpolicies:
            return
        self.egress_networkpolicies.append(policy_name)
        self.egress_networkpolicies.sort()
        if len(self.egress_networkpolicies) == 1 and self.ip != "":
            self.update_network_policy_enforcement_map_egress()
        self.store_update_obj()

    def remove_ingress_networkpolicy(self, policy_name):
        if policy_name not in self.ingress_networkpolicies:
            return

        self.ingress_networkpolicies.remove(policy_name)
        if len(self.ingress_networkpolicies) == 0:
            self.delete_network_policy_enforcement_map_ingress()
        self.store_update_obj()

    def remove_egress_networkpolicy(self, policy_name):
        if policy_name not in self.egress_networkpolicies:
            return

        self.egress_networkpolicies.remove(policy_name)
        if len(self.egress_networkpolicies) == 0:
            self.delete_network_policy_enforcement_map_egress()
        self.store_update_obj()

    def get_data_for_networkpolicy(self):
        return self.data_for_networkpolicy

    def set_data_for_networkpolicy(self, data_for_networkpolicy):
        self.data_for_networkpolicy = data_for_networkpolicy

    def load_transit_agent(self):
        self.rpc.load_transit_agent_xdp(self.veth_peer)

    def unload_transit_agent_xdp(self):
        self.rpc.unload_transit_agent_xdp(self)

    def update_agent_substrate(self, ep, bouncer):
        self.rpc.update_agent_substrate_ep(ep, bouncer.ip, bouncer.mac)

    def delete_agent_substrate(self, ep, bouncer):
        self.rpc.delete_agent_substrate_ep(ep, bouncer.ip)

    def update_networkpolicy_per_endpoint(self, data):
        if len(data["old"]) > 0:
            self.delete_network_policy_ingress("no_except", data["old"]["ingress"]["cidr_table_no_except"])
            self.delete_network_policy_ingress("with_except", data["old"]["ingress"]["cidr_table_with_except"])            
            self.delete_network_policy_egress("no_except", data["old"]["egress"]["cidr_table_no_except"])
            self.delete_network_policy_egress("with_except", data["old"]["egress"]["cidr_table_with_except"])
            # When deleting policy data, except data should be after cidr data
            self.delete_network_policy_ingress("except", data["old"]["ingress"]["cidr_table_except"])
            self.delete_network_policy_egress("except", data["old"]["egress"]["cidr_table_except"])
            self.delete_network_policy_protocol_port_ingress(data["old"]["ingress"]["port_table"])
            self.delete_network_policy_protocol_port_egress(data["old"]["egress"]["port_table"])

        # When updating policy data, except data should be before cidr data
        self.update_network_policy_ingress("except", data["ingress"]["cidr_table_except"])
        self.update_network_policy_egress("except", data["egress"]["cidr_table_except"])
        self.update_network_policy_ingress("no_except", data["ingress"]["cidr_table_no_except"])
        self.update_network_policy_ingress("with_except", data["ingress"]["cidr_table_with_except"])
        self.update_network_policy_egress("no_except", data["egress"]["cidr_table_no_except"])
        self.update_network_policy_egress("with_except", data["egress"]["cidr_table_with_except"])
        self.update_network_policy_protocol_port_ingress(data["ingress"]["port_table"])
        self.update_network_policy_protocol_port_egress(data["egress"]["port_table"])

    def update_network_policy_ingress(self, cidr_type, cidr_table):
        cidr_networkpolicy_list = []
        for item in cidr_table:
            cidr_networkpolicy = CidrNetworkPolicy(
                item["vni"], item["local_ip"], item["cidr"], item["cidr_length"], item["bit_value"],
                cidr_type)
            cidr_networkpolicy_list.append(cidr_networkpolicy)
        self.rpc.update_network_policy_ingress(cidr_networkpolicy_list)

    def update_network_policy_egress(self, cidr_type, cidr_table):
        cidr_networkpolicy_list = []
        for item in cidr_table:
            cidr_networkpolicy = CidrNetworkPolicy(
                item["vni"], item["local_ip"], item["cidr"], item["cidr_length"], item["bit_value"],
                cidr_type)
            cidr_networkpolicy_list.append(cidr_networkpolicy)
        self.rpc.update_network_policy_egress(self, cidr_networkpolicy_list)

    def update_network_policy_protocol_port_ingress(self, port_table):
        cidr_networkpolicy_list = []
        for item in port_table:
            cidr_networkpolicy = PortNetworkPolicy(
                item["vni"], item["local_ip"], item["protocol"], item["port"], item["bit_value"])
            cidr_networkpolicy_list.append(cidr_networkpolicy)
        self.rpc.update_network_policy_protocol_port_ingress(cidr_networkpolicy_list)

    def update_network_policy_protocol_port_egress(self, port_table):
        cidr_networkpolicy_list = []
        for item in port_table:
            cidr_networkpolicy = PortNetworkPolicy(
                item["vni"], item["local_ip"], item["protocol"], item["port"], item["bit_value"])
            cidr_networkpolicy_list.append(cidr_networkpolicy)
        self.rpc.update_network_policy_protocol_port_egress(self, cidr_networkpolicy_list)

    def delete_network_policy_ingress(self, cidr_type, cidr_table):
        cidr_networkpolicy_list = []
        for item in cidr_table:
            cidr_networkpolicy = CidrNetworkPolicy(
                item["vni"], item["local_ip"], item["cidr"], item["cidr_length"], item["bit_value"],
                cidr_type)
            cidr_networkpolicy_list.append(cidr_networkpolicy)
        self.rpc.delete_network_policy_ingress(cidr_networkpolicy_list)

    def delete_network_policy_egress(self, cidr_type, cidr_table):
        cidr_networkpolicy_list = []
        for item in cidr_table:
            cidr_networkpolicy = CidrNetworkPolicy(
                item["vni"], item["local_ip"], item["cidr"], item["cidr_length"], item["bit_value"],
                cidr_type)
            cidr_networkpolicy_list.append(cidr_networkpolicy)
        self.rpc.delete_network_policy_egress(self, cidr_networkpolicy_list)

    def delete_network_policy_protocol_port_ingress(self, port_table):
        cidr_networkpolicy_list = []
        for item in port_table:
            cidr_networkpolicy = PortNetworkPolicy(
                item["vni"], item["local_ip"], item["protocol"], item["port"], item["bit_value"])
            cidr_networkpolicy_list.append(cidr_networkpolicy)
        self.rpc.delete_network_policy_protocol_port_ingress(cidr_networkpolicy_list)

    def delete_network_policy_protocol_port_egress(self, port_table):
        cidr_networkpolicy_list = []
        for item in port_table:
            cidr_networkpolicy = PortNetworkPolicy(
                item["vni"], item["local_ip"], item["protocol"], item["port"], item["bit_value"])
            cidr_networkpolicy_list.append(cidr_networkpolicy)
        self.rpc.delete_network_policy_protocol_port_egress(self, cidr_networkpolicy_list)

    def update_network_policy_enforcement_map_ingress(self):
        endpointEnforced = EndpointEnforced(self.vni, self.ip)
        self.rpc.update_network_policy_enforcement_map_ingress(endpointEnforced)

    def update_network_policy_enforcement_map_egress(self):
        endpointEnforced = EndpointEnforced(self.vni, self.ip)
        self.rpc.update_network_policy_enforcement_map_egress(self, endpointEnforced)

    def delete_network_policy_enforcement_map_ingress(self):
        endpointEnforced = EndpointEnforced(self.vni, self.ip)
        self.rpc.delete_network_policy_enforcement_map_ingress(endpointEnforced)

    def delete_network_policy_enforcement_map_egress(self):
        endpointEnforced = EndpointEnforced(self.vni, self.ip)
        self.rpc.delete_network_policy_enforcement_map_egress(self, endpointEnforced)
