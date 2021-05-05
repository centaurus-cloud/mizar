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
import json
from mizar.common.common import run_cmd, conf_list_has_max_elements
from mizar.common.constants import *
from mizar.get_interface import get_itf

net_itf = get_itf()
logger = logging.getLogger()


class TrnRpc:
    def __init__(self, ip, mac, itf=net_itf, benchmark=False):
        self.ip = ip
        self.mac = mac
        self.phy_itf = itf

        # transitd cli commands
        self.trn_cli = f'''/trn_bin/transit -s {self.ip} '''
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
        self.trn_cli_update_port = f'''{self.trn_cli} update-port -i {self.phy_itf} -j'''
        self.trn_cli_load_pipeline_stage = f'''{self.trn_cli} load-pipeline-stage -i {self.phy_itf} -j'''
        self.trn_cli_unload_pipeline_stage = f'''{self.trn_cli} unload-pipeline-stage -i {self.phy_itf} -j'''
        self.trn_cli_update_network_policy_ingress = f'''{self.trn_cli} update-net-policy-in -i {self.phy_itf} -j'''
        self.trn_cli_update_network_policy_egress = f'''{self.trn_cli} update-net-policy-out -j'''
        self.trn_cli_delete_network_policy_ingress = f'''{self.trn_cli} delete-net-policy-in -i {self.phy_itf} -j'''
        self.trn_cli_delete_network_policy_egress = f'''{self.trn_cli} delete-net-policy-out -j'''
        self.trn_cli_update_network_policy_enforcement_map_ingress = f'''{self.trn_cli} update-net-policy-enforce-in -i {self.phy_itf} -j'''
        self.trn_cli_update_network_policy_enforcement_map_egress = f'''{self.trn_cli} update-net-policy-enforce-out -j'''
        self.trn_cli_delete_network_policy_enforcement_map_ingress = f'''{self.trn_cli} delete-net-policy-enforce-in -i {self.phy_itf} -j'''
        self.trn_cli_delete_network_policy_enforcement_map_egress = f'''{self.trn_cli} delete-net-policy-enforce-out -j'''
        self.trn_cli_update_network_policy_protocol_port_ingress = f'''{self.trn_cli} update-net-policy-protocol-port-in -i {self.phy_itf} -j'''
        self.trn_cli_update_network_policy_protocol_port_egress = f'''{self.trn_cli} update-net-policy-protocol-port-out -j'''
        self.trn_cli_delete_network_policy_protocol_port_ingress = f'''{self.trn_cli} delete-net-policy-protocol-port-in -i {self.phy_itf} -j'''
        self.trn_cli_delete_network_policy_protocol_port_egress = f'''{self.trn_cli} delete-net-policy-protocol-port-out -j'''
        self.trn_cli_load_transit_agent_xdp = f'''{self.trn_cli} load-agent-xdp'''
        self.trn_cli_unload_transit_agent_xdp = f'''{self.trn_cli} unload-agent-xdp'''
        self.trn_cli_update_agent_metadata = f'''{self.trn_cli} update-agent-metadata'''
        self.trn_cli_get_agent_metadata = f'''{self.trn_cli} get-agent-metadata'''
        self.trn_cli_delete_agent_metadata = f'''{self.trn_cli} delete-agent-metadata'''
        self.trn_cli_update_agent_ep = f'''{self.trn_cli} update-agent-ep'''
        self.trn_cli_get_agent_ep = f'''{self.trn_cli} get-agent-ep'''
        self.trn_cli_delete_agent_ep = f'''{self.trn_cli} delete-agent-ep'''

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

    def update_substrate_ep(self, ip, mac):
        jsonconf = self.get_substrate_ep_json(ip, mac)
        cmd = f'''{self.trn_cli_update_ep} \'{jsonconf}\''''
        logger.info("update_substrate_ep: {}".format(cmd))
        returncode, text = run_cmd(cmd)
        logger.info("returns {} {}".format(returncode, text))

    def update_agent_substrate_ep(self, ep, ip, mac):
        itf = ep.get_veth_peer()
        jsonconf = self.get_substrate_ep_json(ip, mac)
        cmd = f'''{self.trn_cli_update_agent_ep} -i \'{itf}\' -j \'{jsonconf}\''''
        logger.info("update_agent_substrate_ep: {}".format(cmd))
        returncode, text = run_cmd(cmd)
        logger.info(
            "update_agent_substrate_ep returns {} {}".format(returncode, text))

    def delete_agent_substrate_ep(self, ep, ip):
        itf = ep.get_veth_peer()
        jsonconf = {
            "tunnel_id": "0",
            "ip": ip,
        }
        cmd = f'''{self.trn_cli_delete_agent_ep} -i \'{itf}\' -j \'{jsonconf}\''''
        logger.info("delete_agent_substrate_ep: {}".format(cmd))
        returncode, text = run_cmd(cmd)
        logger.info(
            "delete_agent_substrate_ep returns {} {}".format(returncode, text))

    def update_ep(self, ep):
        peer = ""
        droplet_ip = ep.get_droplet_ip()
        # Only detail veth info if the droplet is also a host
        if (droplet_ip and self.ip == droplet_ip):
            peer = ep.get_veth_peer()

        jsonconf = {
            "tunnel_id": ep.get_tunnel_id(),
            "ip": ep.get_ip(),
            "eptype": ep.get_eptype(),
            "mac": ep.get_mac(),
            "veth": ep.get_veth_name(),
            "remote_ips": ep.get_remote_ips(),
            "hosted_iface": peer
        }

        jsonconf = json.dumps(jsonconf)
        cmd = f'''{self.trn_cli_update_ep} \'{jsonconf}\''''
        logger.info("update_ep: {}".format(cmd))
        returncode, text = run_cmd(cmd)
        logger.info("returns {} {}".format(returncode, text))
        remote_ports = ep.get_remote_ports()
        frontend_ports = ep.get_frontend_ports()
        protocols = ep.get_port_protocols()
        for i in range(len(remote_ports)):
            self.update_port(ep.get_tunnel_id(), ep.get_ip(),
                             frontend_ports[i], remote_ports[i], protocols[i])

    def update_port(self, tunid, ip, port, target_port, protocol):
        jsonconf = {
            "tunnel_id": tunid,
            "ip": ip,
            "port": port,
            "target_port": target_port,
            "protocol": protocol,
        }
        jsonconf = json.dumps(jsonconf)
        cmd = f'''{self.trn_cli_update_port} \'{jsonconf}\''''
        logger.info("update_port: {}".format(cmd))
        returncode, text = run_cmd(cmd)
        logger.info("returns {} {}".format(returncode, text))

    def update_agent_metadata(self, ep):
        itf = ep.get_veth_peer()
        jsonconf = {
            "ep": {
                "tunnel_id": ep.get_tunnel_id(),
                "ip": ep.get_ip(),
                "eptype": ep.get_eptype(),
                "mac": ep.get_mac(),
                "veth": ep.get_veth_name(),
                "remote_ips": ep.get_remote_ips(),
                "hosted_iface": ep.droplet_eth
            },
            "net": {
                "tunnel_id": ep.get_tunnel_id(),
                "nip": ep.get_nip(),
                "prefixlen": ep.get_prefix(),
                "switches_ips": ep.get_bouncers_ips()
            },
            "eth": {
                "ip": ep.droplet_ip,
                "mac": ep.droplet_mac,
                "iface": ep.droplet_eth
            }
        }
        jsonconf = json.dumps(jsonconf)
        cmd = f'''{self.trn_cli_update_agent_metadata} -i \'{itf}\' -j \'{jsonconf}\''''
        logger.info("update_agent_metadata: {}".format(cmd))
        returncode, text = run_cmd(cmd)
        logger.info(
            "update_agent_metadata returns {} {}".format(returncode, text))

    def load_transit_agent_xdp(self, veth_peer):
        itf = veth_peer
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

    def load_transit_xdp_pipeline_stage(self, stage, obj_file):
        jsonconf = {
            "xdp_path": obj_file,
            "stage": stage
        }
        jsonconf = json.dumps(jsonconf)
        cmd = f'''{self.trn_cli_load_pipeline_stage} \'{jsonconf}\' '''
        logger.info("load_transit_xdp_pipeline_stage: {}".format(cmd))
        returncode, text = run_cmd(cmd)
        logger.info(
            "load_transit_xdp_pipeline_stage returns {} {}".format(returncode, text))

    def unload_transit_xdp_pipeline_stage(self, stage, obj_file):
        jsonconf = {
            "stage": stage
        }
        jsonconf = json.dumps(jsonconf)
        cmd = f'''{self.trn_cli_unload_pipeline_stage} \'{jsonconf}\' '''
        logger.info("unload_transit_xdp_pipeline_stage: {}".format(cmd))
        returncode, text = run_cmd(cmd)
        logger.info(
            "unload_transit_xdp_pipeline_stage returns {} {}".format(returncode, text))

    def delete_substrate_ep(self, ip):
        jsonconf = {
            "tunnel_id": "0",
            "ip": ip,
        }
        jsonconf = json.dumps(jsonconf)
        cmd = f'''{self.trn_cli_delete_ep} \'{jsonconf}\''''
        logger.info("delete_substrate_ep: {}".format(cmd))
        returncode, text = run_cmd(cmd)
        logger.info(
            "delete_substrate_ep returns {} {}".format(returncode, text))

    def delete_ep(self, ep):
        jsonconf = {
            "tunnel_id": ep.get_tunnel_id(),
            "ip": ep.get_ip(),
        }
        jsonconf = json.dumps(jsonconf)
        cmd = f'''{self.trn_cli_delete_ep} \'{jsonconf}\''''
        log_string = "delete_ep for a {} {}".format(ep.type, ep.ip)
        logger.info(log_string)
        returncode, text = run_cmd(cmd)
        logger.info("returns {} {}".format(returncode, text))

    def unload_transit_agent_xdp(self, ep):
        itf = ep.veth_peer
        jsonconf = '\'{}\''
        cmd = f'''{self.trn_cli_unload_transit_agent_xdp} -i \'{itf}\' -j {jsonconf} '''
        logger.info("unload_transit_agent_xdp: {}".format(cmd))
        returncode, text = run_cmd(cmd)
        logger.info(
            "unload_transit_agent_xdp returns {} {}".format(returncode, text))

    def update_vpc(self, bouncer):
        if len(bouncer.get_divider_ips()) < 1:
            logger.info("Bouncer list of dividers, LEN IS ZERO")
            return
        jsonconf = {
            "tunnel_id": bouncer.vni,
            "routers_ips": bouncer.get_divider_ips()
        }
        jsonconf = json.dumps(jsonconf)
        cmd = f'''{self.trn_cli_update_vpc} \'{jsonconf}\''''
        logger.info("update_vpc: {}".format(cmd))
        returncode, text = run_cmd(cmd)
        logger.info("update_vpc returns {} {}".format(returncode, text))

    def delete_vpc(self, bouncer):
        jsonconf = {
            "tunnel_id": bouncer.vni
        }
        jsonconf = json.dumps(jsonconf)
        cmd = f'''{self.trn_cli_delete_vpc} \'{jsonconf}\''''
        logger.info("delete_vpc: {}".format(cmd))
        returncode, text = run_cmd(cmd)
        logger.info("delete_vpc returns {} {}".format(returncode, text))

    def update_net(self, net):
        if len(net.get_bouncers_ips()) < 1:
            logger.info("net list of bouncers LEN IS ZERO")
            return
        jsonconf = {
            "tunnel_id": net.vni,
            "nip": net.get_nip(),
            "prefixlen": net.get_prefixlen(),
            "switches_ips": net.get_bouncers_ips()
        }
        jsonconf = json.dumps(jsonconf)
        cmd = f'''{self.trn_cli_update_net} \'{jsonconf}\''''
        logger.info("update_net: {}".format(cmd))
        returncode, text = run_cmd(cmd)
        logger.info("update_net returns {} {}".format(returncode, text))

    def delete_net(self, net):
        jsonconf = {
            "tunnel_id": net.vni,
            "nip": net.get_nip(),
            "prefixlen": net.get_prefixlen()
        }
        jsonconf = json.dumps(jsonconf)
        cmd = f'''{self.trn_cli_delete_net} \'{jsonconf}\''''
        logger.info("delete_net: {}".format(cmd))
        returncode, text = run_cmd(cmd)
        logger.info("delete_net returns {} {}".format(returncode, text))

    def update_network_policy_ingress(self, cidr_networkpolicy_list):
        if len(cidr_networkpolicy_list) == 0:
            return
        conf_list = []
        for cidr_networkpolicy in cidr_networkpolicy_list:
            conf = {
                "tunnel_id": cidr_networkpolicy.vni,
                "local_ip": cidr_networkpolicy.local_ip,
                "cidr_prefixlen": str(cidr_networkpolicy.cidr_length),
                "cidr_ip": cidr_networkpolicy.cidr,
                "cidr_type": cidr_networkpolicy.get_cidr_type_int(),
                "bit_value": str(cidr_networkpolicy.policy_bit_value),
            }
            conf_list.append(conf)
            if (conf_list_has_max_elements(conf, conf_list)):
                self.run_cli_update_network_policy_ingress(conf_list)
                conf_list = []
        self.run_cli_update_network_policy_ingress(conf_list)

    def run_cli_update_network_policy_ingress(self, conf_list):
        jsonconf = json.dumps(conf_list)
        cmd = f'''{self.trn_cli_update_network_policy_ingress} \'{jsonconf}\''''
        logger.info("update_network_policy_ingress: {}".format(cmd))
        returncode, text = run_cmd(cmd)
        logger.info("update_network_policy_ingress returns {} {}".format(returncode, text))

    def update_network_policy_egress(self, ep, cidr_networkpolicy_list):
        if len(cidr_networkpolicy_list) == 0:
            return
        itf = ep.get_veth_peer()
        conf_list = []
        for cidr_networkpolicy in cidr_networkpolicy_list:
            conf = {
                "tunnel_id": cidr_networkpolicy.vni,
                "local_ip": cidr_networkpolicy.local_ip,
                "cidr_prefixlen": str(cidr_networkpolicy.cidr_length),
                "cidr_ip": cidr_networkpolicy.cidr,
                "cidr_type": cidr_networkpolicy.get_cidr_type_int(),
                "bit_value": str(cidr_networkpolicy.policy_bit_value),
            }
            conf_list.append(conf)
            if (conf_list_has_max_elements(conf, conf_list)):
                self.run_cli_update_network_policy_egress(conf_list, itf)
                conf_list = []
        self.run_cli_update_network_policy_egress(conf_list, itf)

    def run_cli_update_network_policy_egress(self, conf_list, itf):
        jsonconf = json.dumps(conf_list)
        cmd = f'''{self.trn_cli_update_network_policy_egress} \'{jsonconf}\' -i \'{itf}\''''
        logger.info("update_network_policy_egress: {}".format(cmd))
        returncode, text = run_cmd(cmd)
        logger.info("update_network_policy_egress returns {} {}".format(returncode, text))

    def delete_network_policy_ingress(self, cidr_networkpolicy_list):
        if len(cidr_networkpolicy_list) == 0:
            return
        conf_list = []
        for cidr_networkpolicy in cidr_networkpolicy_list:
            conf = {
                "tunnel_id": cidr_networkpolicy.vni,
                "local_ip": cidr_networkpolicy.local_ip,
                "cidr_prefixlen": str(cidr_networkpolicy.cidr_length),
                "cidr_ip": cidr_networkpolicy.cidr,
                "cidr_type": cidr_networkpolicy.get_cidr_type_int(),
                "bit_value": str(cidr_networkpolicy.policy_bit_value),
            }
            conf_list.append(conf)
            if (conf_list_has_max_elements(conf, conf_list)):
                self.run_cli_delete_network_policy_ingress(conf_list)
                conf_list = []
        self.run_cli_delete_network_policy_ingress(conf_list)

    def run_cli_delete_network_policy_ingress(self, conf_list):
        jsonconf = json.dumps(conf_list)
        cmd = f'''{self.trn_cli_delete_network_policy_ingress} \'{jsonconf}\''''
        logger.info("delete_network_policy_ingress: {}".format(cmd))
        returncode, text = run_cmd(cmd)
        logger.info("delete_network_policy_ingress returns {} {}".format(returncode, text))

    def delete_network_policy_egress(self, ep, cidr_networkpolicy_list):
        if len(cidr_networkpolicy_list) == 0:
            return
        itf = ep.get_veth_peer()
        conf_list = []
        for cidr_networkpolicy in cidr_networkpolicy_list:
            conf = {
                "tunnel_id": cidr_networkpolicy.vni,
                "local_ip": cidr_networkpolicy.local_ip,
                "cidr_prefixlen": str(cidr_networkpolicy.cidr_length),
                "cidr_ip": cidr_networkpolicy.cidr,
                "cidr_type": cidr_networkpolicy.get_cidr_type_int(),
                "bit_value": str(cidr_networkpolicy.policy_bit_value),
            }
            conf_list.append(conf)
            if (conf_list_has_max_elements(conf, conf_list)):
                self.run_cli_delete_network_policy_egress(conf_list, itf)
                conf_list = []
        self.run_cli_delete_network_policy_egress(conf_list, itf)

    def run_cli_delete_network_policy_egress(self, conf_list, itf):
        jsonconf = json.dumps(conf_list)
        cmd = f'''{self.trn_cli_delete_network_policy_egress} \'{jsonconf}\' -i \'{itf}\''''
        logger.info("delete_network_policy_egress: {}".format(cmd))
        returncode, text = run_cmd(cmd)
        logger.info("delete_network_policy_egress returns {} {}".format(returncode, text))

    def update_network_policy_protocol_port_ingress(self, port_networkpolicy_list):
        if len(port_networkpolicy_list) == 0:
            return
        conf_list = []
        for port_networkpolicy in port_networkpolicy_list:
            conf = {
                "tunnel_id": port_networkpolicy.vni,
                "local_ip": port_networkpolicy.local_ip,
                "protocol": port_networkpolicy.get_protocol_int(),
                "port": port_networkpolicy.port,
                "bit_value": str(port_networkpolicy.policy_bit_value),
            }
            conf_list.append(conf)
            if (conf_list_has_max_elements(conf, conf_list)):
                self.run_cli_update_network_policy_protocol_port_ingress(conf_list)
                conf_list = []
        self.run_cli_update_network_policy_protocol_port_ingress(conf_list)

    def run_cli_update_network_policy_protocol_port_ingress(self, conf_list):
        jsonconf = json.dumps(conf_list)
        cmd = f'''{self.trn_cli_update_network_policy_protocol_port_ingress} \'{jsonconf}\''''
        logger.info("update_network_policy_protocol_port_ingress: {}".format(cmd))
        returncode, text = run_cmd(cmd)
        logger.info("update_network_policy_protocol_port_ingress returns {} {}".format(returncode, text))

    def update_network_policy_protocol_port_egress(self, ep, port_networkpolicy_list):
        if len(port_networkpolicy_list) == 0:
            return
        itf = ep.get_veth_peer()
        conf_list = []
        for port_networkpolicy in port_networkpolicy_list:
            conf = {
                "tunnel_id": port_networkpolicy.vni,
                "local_ip": port_networkpolicy.local_ip,
                "protocol": port_networkpolicy.get_protocol_int(),
                "port": port_networkpolicy.port,
                "bit_value": str(port_networkpolicy.policy_bit_value),
            }
            conf_list.append(conf)
            if (conf_list_has_max_elements(conf, conf_list)):
                self.run_cli_update_network_policy_protocol_port_egress(conf_list, itf)
                conf_list = []
        self.run_cli_update_network_policy_protocol_port_egress(conf_list, itf)

    def run_cli_update_network_policy_protocol_port_egress(self, conf_list, itf):
        jsonconf = json.dumps(conf_list)
        cmd = f'''{self.trn_cli_update_network_policy_protocol_port_egress} \'{jsonconf}\' -i \'{itf}\''''
        logger.info("update_network_policy_protocol_port_egress: {}".format(cmd))
        returncode, text = run_cmd(cmd)
        logger.info("update_network_policy_protocol_port_egress returns {} {}".format(returncode, text))

    def delete_network_policy_protocol_port_ingress(self, port_networkpolicy_list):
        if len(port_networkpolicy_list) == 0:
            return
        conf_list = []
        for port_networkpolicy in port_networkpolicy_list:
            conf = {
                "tunnel_id": port_networkpolicy.vni,
                "local_ip": port_networkpolicy.local_ip,
                "protocol": port_networkpolicy.get_protocol_int(),
                "port": port_networkpolicy.port,
                "bit_value": str(port_networkpolicy.policy_bit_value),
            }
            conf_list.append(conf)
            if (conf_list_has_max_elements(conf, conf_list)):
                self.run_cli_delete_network_policy_protocol_port_ingress(conf_list)
                conf_list = []
        self.run_cli_delete_network_policy_protocol_port_ingress(conf_list)

    def run_cli_delete_network_policy_protocol_port_ingress(self, conf_list):
        jsonconf = json.dumps(conf_list)
        cmd = f'''{self.trn_cli_delete_network_policy_protocol_port_ingress} \'{jsonconf}\''''
        logger.info("delete_network_policy_protocol_port_ingress: {}".format(cmd))
        returncode, text = run_cmd(cmd)
        logger.info("delete_network_policy_protocol_port_ingress returns {} {}".format(returncode, text))

    def delete_network_policy_protocol_port_egress(self, ep, port_networkpolicy_list):
        if len(port_networkpolicy_list) == 0:
            return
        itf = ep.get_veth_peer()
        conf_list = []
        for port_networkpolicy in port_networkpolicy_list:
            conf = {
                "tunnel_id": port_networkpolicy.vni,
                "local_ip": port_networkpolicy.local_ip,
                "protocol": port_networkpolicy.get_protocol_int(),
                "port": port_networkpolicy.port,
                "bit_value": str(port_networkpolicy.policy_bit_value),
            }
            conf_list.append(conf)
            if (conf_list_has_max_elements(conf, conf_list)):
                self.run_cli_delete_network_policy_protocol_port_egress(conf_list, itf)
                conf_list = []
        self.run_cli_delete_network_policy_protocol_port_egress(conf_list, itf)

    def run_cli_delete_network_policy_protocol_port_egress(self, conf_list, itf):
        jsonconf = json.dumps(conf_list)
        cmd = f'''{self.trn_cli_delete_network_policy_protocol_port_egress} \'{jsonconf}\' -i \'{itf}\''''
        logger.info("delete_network_policy_protocol_port_egress: {}".format(cmd))
        returncode, text = run_cmd(cmd)
        logger.info("delete_network_policy_protocol_port_egress returns {} {}".format(returncode, text))

    def update_network_policy_enforcement_map_ingress(self, endpointEnforced):
        jsonconf = {
            "tunnel_id": endpointEnforced.vni,
            "ip": endpointEnforced.ip,
        }
        jsonconf = json.dumps(jsonconf)
        cmd = f'''{self.trn_cli_update_network_policy_enforcement_map_ingress} \'{jsonconf}\''''
        logger.info("update_network_policy_enforcement_map_ingress: {}".format(cmd))
        returncode, text = run_cmd(cmd)
        logger.info("update_network_policy_enforcement_map_ingress returns {} {}".format(returncode, text))

    def update_network_policy_enforcement_map_egress(self, ep, endpointEnforced):
        itf = ep.get_veth_peer()
        jsonconf = {
            "tunnel_id": endpointEnforced.vni,
            "ip": endpointEnforced.ip,
        }
        jsonconf = json.dumps(jsonconf)
        cmd = f'''{self.trn_cli_update_network_policy_enforcement_map_egress} \'{jsonconf}\' -i \'{itf}\''''
        logger.info("update_network_policy_enforcement_map_egress: {}".format(cmd))
        returncode, text = run_cmd(cmd)
        logger.info("update_network_policy_enforcement_map_egress returns {} {}".format(returncode, text))

    def delete_network_policy_enforcement_map_ingress(self, endpointEnforced):
        jsonconf = {
            "tunnel_id": endpointEnforced.vni,
            "ip": endpointEnforced.ip,
        }
        jsonconf = json.dumps(jsonconf)
        cmd = f'''{self.trn_cli_delete_network_policy_enforcement_map_ingress} \'{jsonconf}\''''
        logger.info("delete_network_policy_enforcement_map_ingress: {}".format(cmd))
        returncode, text = run_cmd(cmd)
        logger.info("delete_network_policy_enforcement_map_ingress returns {} {}".format(returncode, text))

    def delete_network_policy_enforcement_map_egress(self, ep, endpointEnforced):
        itf = ep.get_veth_peer()
        jsonconf = {
            "tunnel_id": endpointEnforced.vni,
            "ip": endpointEnforced.ip,
        }
        jsonconf = json.dumps(jsonconf)
        cmd = f'''{self.trn_cli_delete_network_policy_enforcement_map_egress} \'{jsonconf}\' -i \'{itf}\''''
        logger.info("delete_network_policy_enforcement_map_egress: {}".format(cmd))
        returncode, text = run_cmd(cmd)
        logger.info("delete_network_policy_enforcement_map_egress returns {} {}".format(returncode, text))
