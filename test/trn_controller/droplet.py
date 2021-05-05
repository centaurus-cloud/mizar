# Copyright (c) 2019 The Authors.
#
# Authors: Sherif Abdelwahab <@zasherif>
#          Phu Tran          <@phudtran>
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
#    WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
#    License for the specific language governing permissions and limitations
#    under the License.
from test.trn_controller.common import cidr, logger, run_cmd
import os
import docker
import time
import json


class droplet:
    def __init__(self, id, droplet_type='docker', phy_itf='eth0', benchmark=False):
        """
        Models a host that runs the transit XDP program. In the
        functional test this is simply a docker container.
        """
        self.id = id
        self.droplet_type = droplet_type
        self.container = None
        self.ip = None
        self.mac = None
        self.veth_peers = set()
        self.rpc_updates = {}
        self.rpc_deletes = {}
        self.rpc_failures = {}
        self.phy_itf = phy_itf
        self.benchmark = benchmark
        # When droplet is a switch for two different networks in the same vpc
        self.known_nets = set()
        self.vpc_updates = {}
        self.substrate_updates = {}  # When droplet is a host for multiple objects
        self.endpoint_updates = {}  # When droplet is a switch host and ep host
        # We don't need one for net because delete_net takes nip

        if benchmark:
            self.xdp_path = "/trn_xdp/trn_transit_xdp_ebpf.o"
            self.agent_xdp_path = "/trn_xdp/trn_agent_xdp_ebpf.o"
        else:
            self.xdp_path = "/trn_xdp/trn_transit_xdp_ebpf_debug.o"
            self.agent_xdp_path = "/trn_xdp/trn_agent_xdp_ebpf_debug.o"

        # transitd cli commands
        self.trn_cli = f'''/trn_bin/transit'''
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

        self.xdp_path = "/trn_xdp/trn_transit_xdp_ebpf_debug.o"
        self.agent_xdp_path = "/trn_xdp/trn_agent_xdp_ebpf_debug.o"
        self.pcap_file = "eth0_transit_pcap"
        self.agent_pcap_file = {}
        self.main_bridge = 'br0'
        self.bootstrap()

    def bootstrap(self):
        if self.droplet_type == 'docker':
            self._create_docker_container()
            self.load_transit_xdp()
            if not self.benchmark:
                self.start_pcap()
            return

        logger.error("Unsupported droplet type!")

    def load_transit_xdp_pipeline_stage(self, stage, object, expect_fail=False):
        """
        Loads an XDP program at stage!
        """
        log_string = "[DROPLET {}]: load_transit_xdp_pipeline_stage {}".format(
            stage, object)

        jsonconf = {
            "xdp_path": object,
            "stage": stage
        }
        jsonconf = json.dumps(jsonconf)
        cmd = f'''{self.trn_cli_load_pipeline_stage} \'{jsonconf}\' '''
        self.exec_cli_rpc(log_string, cmd, expect_fail)

    def provision_simple_endpoint(self, ep):
        """
        Creates a veth pair and a namespace for the endpoint and loads
        the transit agent program on the veth peer running in the root
        namespace.
        """
        logger.info(
            "[DROPLET {}]: provision_simple_endpoint {}".format(self.id, ep.ip))

        self._create_veth_pair(ep)
        self.load_transit_agent_xdp(ep.veth_peer)
        pcap_file = ep.veth_peer + "_transit_agent_pcap"
        self.agent_pcap_file[ep.veth_peer] = pcap_file

    def provision_host_endpoint(self, ep):
        """
        Creates a veth pair and in the root namespace for the endpoint and loads
        the transit agent program on the veth peer running in the root
        namespace.
        """
        logger.info(
            "[DROPLET {}]: provision_simple_endpoint {}".format(self.id, ep.ip))

        self._create_host_veth_pair(ep)
        self.load_transit_agent_xdp(ep.veth_peer)
        pcap_file = ep.veth_peer + "_transit_agent_pcap"
        self.agent_pcap_file[ep.veth_peer] = pcap_file

    def unprovision_simple_endpoint(self, ep):
        """
        Unloads the transit agent program on the veth peer, and
        deletes the veth pair and the namespace for the endpoint
        """
        logger.info(
            "[DROPLET {}]: unprovision_simple_endpoint {}".format(self.id, ep.ip))
        self.unload_transit_agent_xdp(ep.veth_peer)
        self._delete_veth_pair(ep)
        self.agent_pcap_file.pop(ep.veth_peer)

    def provision_vxlan_endpoint(self, ep):
        logger.info(
            "[DROPLET {}]: provision_vxlan_endpoint {}".format(self.id, ep.ip))
        self._create_veth_pair(ep)
        return self.ovs_add_port(ep.bridge, ep.veth_peer)

    def _create_macvlan_pair(self, ep):
        """
        Creates a veth pair.
        """
        logger.info(
            "[DROPLET {}]: _create_macvlan_pair {}".format(self.id, ep.ip))

        northitf = ep.veth_peer + '_north'
        southitf = ep.veth_peer

        script = (f''' sudo bash -c '\
mkdir -p /tmp/{ep.ns}_{ep.ip} && \
echo {ep.ip} > /tmp/{ep.ns}_{ep.ip}/index.html && \
ip netns add {ep.ns} && \
ip link add {southitf} type veth peer name {northitf} && \
ip link set dev {northitf} up mtu 9000 && \
ip link set dev {southitf} up mtu 9000 && \
ethtool -K {northitf} tso off gso off ufo off && \
ethtool -K {southitf} tso off gso off ufo off && \
ethtool --offload {northitf} rx off tx off && \
ethtool --offload {southitf} rx off tx off && \
sudo ip link add veth0 link {northitf} type macvlan mode passthru && \
ifconfig veth0 hw ether {ep.mac} && \
ip link set veth0 netns {ep.ns} && \
ip netns exec {ep.ns} ip addr add {ep.ip}/{ep.prefixlen} dev veth0 && \
ip netns exec {ep.ns} ip link set dev veth0 up mtu 1500 && \
ip netns exec {ep.ns} sysctl -w net.ipv4.tcp_mtu_probing=2 && \
ip netns exec {ep.ns} route add default gw {ep.gw_ip} &&  \
ip netns exec {ep.ns} ifconfig lo up && \
ip netns exec {ep.ns} ifconfig veth0 hw ether {ep.mac} ' ''')

        self.run(script)
        self.veth_peers.add(ep.veth_peer)

    def _create_veth_pair(self, ep):
        """
        Creates a veth pair. br0 must have been created.
        """
        logger.info(
            "[DROPLET {}]: _create_veth_pair {}".format(self.id, ep.ip))

        script = (f''' bash -c '\
mkdir -p /tmp/{ep.ns}_{ep.ip} && \
echo {ep.ip} > /tmp/{ep.ns}_{ep.ip}/index.html && \
ip netns add {ep.ns} && \
ip link add veth0 type veth peer name {ep.veth_peer} && \
ip link set veth0 netns {ep.ns} && \
ip netns exec {ep.ns} ip addr add {ep.ip}/{ep.prefixlen} dev veth0 && \
ip netns exec {ep.ns} ip link set dev veth0 up && \
ip netns exec {ep.ns} sysctl -w net.ipv4.tcp_mtu_probing=2 && \
ip netns exec {ep.ns} ethtool -K veth0 tso off gso off ufo off && \
ip netns exec {ep.ns} ethtool --offload veth0 rx off tx off && \
ip link set dev {ep.veth_peer} up mtu 9000 && \
ip netns exec {ep.ns} route add default gw {ep.gw_ip} &&  \
ip netns exec {ep.ns} ifconfig lo up &&  \
ip netns exec {ep.ns} ifconfig veth0 hw ether {ep.mac} ' ''')

        self.run(script)
        self.veth_peers.add(ep.veth_peer)

    def _create_host_veth_pair(self, ep):
        """
        Creates a veth pair in the root namespace
        """
        logger.info(
            "[DROPLET {}]: _create_host_veth_pair {}".format(self.id, ep.ip))

        script = (f''' sudo bash -c '\
mkdir -p /tmp/{ep.ns}_{ep.ip} && \
echo {ep.ip} > /tmp/{ep.ns}_{ep.ip}/index.html && \
ip link add {ep.veth_name} type veth peer name {ep.veth_peer} && \
ip link set dev {ep.veth_name} up && \
ip addr add {ep.ip}/{ep.prefixlen} dev {ep.veth_name} && \
ethtool -K {ep.veth_name} tso off gso off ufo off && \
ethtool --offload {ep.veth_name} rx off tx off && \
ip link set dev {ep.veth_peer} up mtu 9000 && \
ip route add {ep.vpc_cidr.ip}/{ep.vpc_cidr.prefixlen} dev {ep.veth_name} && \
ifconfig {ep.veth_name} hw ether {ep.mac} ' ''')

        self.run(script)
        self.veth_peers.add(ep.veth_name)

    def _delete_veth_pair(self, ep):
        """
        Deletes a veth pair.
        """
        logger.info(
            "[DROPLET {}]: _delete_veth_pair {}".format(self.id, ep.ip))

        script = (f''' bash -c '\
rm -rf /tmp/{ep.ns}_{ep.ip} &&
ip link delete {ep.veth_peer} && \
ip netns del {ep.ns} \' ''')

        self.run(script)
        self.veth_peers.remove(ep.veth_peer)

    def load_transit_xdp(self, expect_fail=False):
        log_string = "[DROPLET {}]: load_transit_xdp {}".format(
            self.id, self.ip)
        jsonconf = {
            "xdp_path": self.xdp_path,
            "pcapfile": self.pcap_file
        }
        jsonconf = json.dumps(jsonconf)
        cmd = f'''{self.trn_cli_load_transit_xdp} \'{jsonconf}\' '''
        self.exec_cli_rpc(log_string, cmd, expect_fail)

    def unload_transit_xdp(self, expect_fail=False):
        log_string = "[DROPLET {}]: unload_transit_xdp {}".format(
            self.id, self.ip)
        jsonconf = '\'{}\''
        cmd = f'''{self.trn_cli_unload_transit_xdp} {jsonconf} '''
        self.exec_cli_rpc(log_string, cmd, expect_fail)

    def load_transit_agent_xdp(self, itf, expect_fail=False):
        log_string = "[DROPLET {}]: load_transit_agent_xdp {}".format(
            self.id, itf)
        jsonconf = {
            "xdp_path": self.agent_xdp_path,
            "pcapfile": self.agent_pcap_file
        }
        jsonconf = json.dumps(jsonconf)
        self.rpc_updates[("load", itf)] = time.time()
        cmd = f'''{self.trn_cli_load_transit_agent_xdp} -i \'{itf}\' -j \'{jsonconf}\' '''
        self.exec_cli_rpc(log_string, cmd, expect_fail)

    def unload_transit_agent_xdp(self, itf, expect_fail=False):
        log_string = "[DROPLET {}]: unload_transit_agent_xdp {}".format(
            self.id, itf)
        jsonconf = '\'{}\''
        cmd = f'''{self.trn_cli_unload_transit_agent_xdp} -i \'{itf}\' -j {jsonconf} '''
        self.rpc_deletes[("load", itf)] = time.time()
        self.exec_cli_rpc(log_string, cmd, expect_fail)

    def update_vpc(self, vpc, netid, expect_fail=False):
        log_string = "[DROPLET {}]: update_vpc {}".format(
            self.id, vpc.get_tunnel_id())

        jsonconf = {
            "tunnel_id": vpc.get_tunnel_id(),
            "routers_ips": vpc.get_transit_routers_ips()
        }
        jsonconf = json.dumps(jsonconf)
        if netid not in self.known_nets:
            self.known_nets.add(netid)
        cmd = f'''{self.trn_cli_update_vpc} \'{jsonconf}\''''
        self.exec_cli_rpc(log_string, cmd, expect_fail)

    def get_vpc(self, vpc, expect_fail=False):
        log_string = "[DROPLET {}]: get_vpc {}".format(
            self.id, vpc.get_tunnel_id())
        jsonconf = {
            "tunnel_id": vpc.get_tunnel_id(),
        }
        jsonconf = json.dumps(jsonconf)
        cmd = f'''{self.trn_cli_get_vpc} \'{jsonconf}\''''
        self.exec_cli_rpc(log_string, cmd, expect_fail)

    def delete_vpc(self, vpc, netid, expect_fail=False):
        log_string = "[DROPLET {}]: delete_vpc {}".format(
            self.id, vpc.get_tunnel_id())
        jsonconf = {
            "tunnel_id": vpc.get_tunnel_id(),
        }
        jsonconf = json.dumps(jsonconf)
        key = ("vpc " + self.phy_itf, jsonconf)
        cmd = f'''{self.trn_cli_delete_vpc} \'{jsonconf}\''''
        if netid in self.known_nets:
            self.known_nets.remove(netid)
        self.vpc_updates[key] = len(self.known_nets)
        self.do_delete_decrement(
            log_string, cmd, expect_fail, key, self.vpc_updates)

    def update_net(self, net, expect_fail=False):
        log_string = "[DROPLET {}]: update_net {}".format(self.id, net.netid)
        jsonconf = {
            "tunnel_id": net.get_tunnel_id(),
            "nip": net.get_nip(),
            "prefixlen": net.get_prefixlen(),
            "switches_ips": net.get_switches_ips()
        }
        jsonconf = json.dumps(jsonconf)
        jsonkey = {
            "tunnel_id": net.get_tunnel_id(),
            "nip": net.get_nip(),
            "prefixlen": net.get_prefixlen(),
        }
        self.rpc_updates[("net " + self.phy_itf,
                          json.dumps(jsonkey))] = time.time()
        cmd = f'''{self.trn_cli_update_net} \'{jsonconf}\''''
        self.exec_cli_rpc(log_string, cmd, expect_fail)

    def get_net(self, net, expect_fail=False):
        log_string = "[DROPLET {}]: get_net {}".format(self.id, net.netid)
        jsonconf = {
            "tunnel_id": net.get_tunnel_id(),
            "nip": net.get_nip(),
            "prefixlen": net.get_prefixlen(),
        }
        jsonconf = json.dumps(jsonconf)
        cmd = f'''{self.trn_cli_get_net} \'{jsonconf}\''''
        self.exec_cli_rpc(log_string, cmd, expect_fail)

    def delete_net(self, net, expect_fail=False):
        log_string = "[DROPLET {}]: delete_net {}".format(self.id, net.netid)
        jsonconf = {
            "tunnel_id": net.get_tunnel_id(),
            "nip": net.get_nip(),
            "prefixlen": net.get_prefixlen(),
        }
        jsonconf = json.dumps(jsonconf)
        self.rpc_deletes[("net " + self.phy_itf, jsonconf)] = time.time()
        cmd = f'''{self.trn_cli_delete_net} \'{jsonconf}\''''
        self.exec_cli_rpc(log_string, cmd, expect_fail)

    def update_ep(self, ep, expect_fail=False):
        if ep.host is not None:
            log_string = "[DROPLET {}]: update_ep {} hosted at {}".format(
                self.id, ep.ip, ep.host.id)
        else:
            log_string = "[DROPLET {}]: update_ep for a phantom ep {}".format(
                self.id, ep.ip)
        peer = ""

        # Only detail veth info if the droplet is also a host
        if (ep.host and self.ip == ep.host.ip):
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
        jsonkey = {
            "tunnel_id": ep.get_tunnel_id(),
            "ip": ep.get_ip(),
        }
        key = ("ep " + self.phy_itf, json.dumps(jsonkey))
        cmd = f'''{self.trn_cli_update_ep} \'{jsonconf}\''''
        self.do_update_increment(
            log_string, cmd, expect_fail, key, self.endpoint_updates)

    def get_ep(self, ep, agent=False, expect_fail=False):
        jsonconf = {
            "tunnel_id": ep.get_tunnel_id(),
            "ip": ep.get_ip(),
        }
        jsonconf = json.dumps(jsonconf)
        if agent:
            log_string = "[DROPLET {}]: get_agent_ep {} hosted at {}".format(
                self.id, ep.ip, ep.host.id)
            cmd = f'''{self.trn_cli_get_agent_ep} \'{jsonconf}\''''
        else:
            log_string = "[DROPLET {}]: get_ep {} hosted at {}".format(
                self.id, ep.ip, ep.host.id)
            cmd = f'''{self.trn_cli_get_ep} \'{jsonconf}\''''
        self.exec_cli_rpc(log_string, cmd, expect_fail)

    def delete_ep(self, ep, agent=False, expect_fail=False):

        jsonconf = {
            "tunnel_id": ep.get_tunnel_id(),
            "ip": ep.get_ip(),
        }
        jsonconf = json.dumps(jsonconf)
        if agent:
            log_string = "[DROPLET {}]: delete_agent_ep {} hosted at {}".format(
                self.id, ep.ip, ep.host.id)
            cmd = f'''{self.trn_cli_delete_agent_ep} \'{jsonconf}\''''
            self.exec_cli_rpc(log_string, cmd, expect_fail)
        else:
            cmd = f'''{self.trn_cli_delete_ep} \'{jsonconf}\''''
            key = ("ep " + self.phy_itf, jsonconf)
            if ep.host is not None:
                log_string = "[DROPLET {}]: delete_ep {} hosted at {}".format(
                    self.id, ep.ip, ep.host.id)
            else:
                log_string = "[DROPLET {}]: delete_ep for a phantom ep {}".format(
                    self.id, ep.ip)
            self.do_delete_decrement(
                log_string, cmd, expect_fail, key, self.endpoint_updates)

    def get_agent_ep(self, ep, expect_fail=False):
        self.get_ep(ep, agent=True, expect_fail=expect_fail)

    def delete_agent_ep(self, ep, expect_fail=False):
        self.delete_ep(ep, agent=True, expect_fail=expect_fail)

    def delete_packet_metadata(self, ep, agent=False, expect_fail=False):
        jsonconf = {
            "tunnel_id": ep.get_tunnel_id(),
            "ip": ep.get_ip(),
        }
        jsonconf = json.dumps(jsonconf)
        if agent:
            log_string = "[DROPLET {}]: delete_agent_packet_metadata {} hosted at {}".format(
                self.id, ep.ip, ep.host.id)
            cmd = f'''{self.trn_cli_delete_agent_packet_metadata} \'{jsonconf}\''''
            self.exec_cli_rpc(log_string, cmd, expect_fail)
        else:
            cmd = f'''{self.trn_cli_delete_packet_metadata} \'{jsonconf}\''''
            key = ("packet_metadata " + self.phy_itf, jsonconf)
            if ep.host is not None:
                log_string = "[DROPLET {}]: delete_packet_metadata {} hosted at {}".format(
                    self.id, ep.ip, ep.host.id)
            else:
                log_string = "[DROPLET {}]: delete_packet_metadata for a phantom ep {}".format(
                    self.id, ep.ip)

    def update_substrate_ep(self, droplet, expect_fail=False):
        log_string = "[DROPLET {}]: update_substrate_ep for droplet {}".format(
            self.id, droplet.ip)
        jsonconf = droplet.get_substrate_ep_json()
        jsonkey = {
            "tunnel_id": "0",
            "ip": droplet.ip,
        }
        key = ("ep_substrate " + self.phy_itf,
               json.dumps(jsonkey))
        cmd = f'''{self.trn_cli_update_ep} \'{jsonconf}\''''
        self.do_update_increment(
            log_string, cmd, expect_fail, key, self.substrate_updates)

    def delete_substrate_ep(self, droplet, expect_fail=False):
        log_string = "[DROPLET {}]: delete_substrate_ep for droplet {}".format(
            self.id, droplet.ip)
        jsonconf = droplet.get_substrate_ep_json()
        jsonkey = {
            "tunnel_id": "0",
            "ip": droplet.ip,
        }
        key = ("ep_substrate " + self.phy_itf,
               json.dumps(jsonkey))
        cmd = f'''{self.trn_cli_delete_ep} \'{jsonconf}\''''
        self.do_delete_decrement(
            log_string, cmd, expect_fail, key, self.substrate_updates)

    def update_agent_ep(self, itf, expect_fail=False):
        logger.error(
            "[DROPLET {}]: not implemented, no use case for now!".format(self.id))

    def update_packet_metadata(self, ep, expect_fail=False):
        if ep.host is not None:
            log_string = "[DROPLET {}]: update_packet_metadata {} hosted at {}".format(
                self.id, ep.ip, ep.host.id)
        else:
            log_string = "[DROPLET {}]: update_packet_metadata for a phantom ep {}".format(
                self.id, ep.ip)        

        jsonconf = {
            "tunnel_id": ep.get_tunnel_id(),
            "ip": ep.get_ip(),
            "pod_label_value": ep.get_pod_label_value(),
            "namespace_label_value": ep.get_namespace_label_value()
        }
        jsonconf = json.dumps(jsonconf)
        jsonkey = {
            "tunnel_id": ep.get_tunnel_id(),
            "ip": ep.get_ip(),
        }
        key = ("packet_metadata " + self.phy_itf, json.dumps(jsonkey))
        cmd = f'''{self.trn_cli_update_packet_metadata} \'{jsonconf}\''''

    def update_agent_metadata(self, itf, ep, net, expect_fail=False):
        log_string = "[DROPLET {}]: update_agent_metadata on {} for endpoint {}".format(
            self.id, itf, ep.ip)
        jsonconf = {
            "ep": {
                "tunnel_id": ep.get_tunnel_id(),
                "ip": ep.get_ip(),
                "eptype": ep.get_eptype(),
                "mac": ep.get_mac(),
                "veth": ep.get_veth_name(),
                "remote_ips": ep.get_remote_ips(),
                "hosted_iface": "eth0"
            },
            "net": {
                "tunnel_id": net.get_tunnel_id(),
                "nip": net.get_nip(),
                "prefixlen": net.get_prefixlen(),
                "switches_ips": net.get_switches_ips()
            },
            "eth": {
                "ip": self.ip,
                "mac": self.mac,
                "iface": "eth0"
            }
        }
        jsonconf = json.dumps(jsonconf)
        cmd = f'''{self.trn_cli_update_agent_metadata} -i \'{itf}\' -j \'{jsonconf}\''''
        self.exec_cli_rpc(log_string, cmd, expect_fail)

    def get_agent_metadata(self, itf, ep, expect_fail=False):
        log_string = "[DROPLET {}]: get_agent_metadata on {} for endpoint {}".format(
            self.id, itf, ep.ip)
        jsonconf = {
            "": "",
        }
        jsonconf = json.dumps(jsonconf)
        cmd = f'''{self.trn_cli_get_agent_metadata} -i \'{itf}\' -j \'{jsonconf}\''''
        self.exec_cli_rpc(log_string, cmd, expect_fail)

    def delete_agent_metadata(self, itf, ep, expect_fail=False):
        log_string = "[DROPLET {}]: delete_agent_metadata on {} for endpoint {}".format(
            self.id, itf, ep.ip)
        jsonconf = {
            "": "",
        }
        jsonconf = json.dumps(jsonconf)
        cmd = f'''{self.trn_cli_delete_agent_metadata} -i \'{itf}\' -j \'{jsonconf}\''''
        self.exec_cli_rpc(log_string, cmd, expect_fail)

    def update_agent_substrate_ep(self, itf, droplet, expect_fail=False):
        log_string = "[DROPLET {}]: update_agent_substrate_ep on {} for droplet {}".format(
            self.id, itf, droplet.ip)

        jsonconf = droplet.get_substrate_ep_json()
        cmd = f'''{self.trn_cli_update_agent_ep} -i \'{itf}\' -j \'{jsonconf}\''''
        self.exec_cli_rpc(log_string, cmd, expect_fail)

    def delete_agent_substrate_ep(self, itf, droplet, expect_fail=False):
        log_string = "[DROPLET {}]: delete_agent_substrate_ep on {} for droplet {}".format(
            self.id, itf, droplet.ip)

        jsonconf = droplet.get_substrate_ep_json()
        cmd = f'''{self.trn_cli_delete_agent_ep} -i \'{itf}\' -j \'{jsonconf}\''''
        self.exec_cli_rpc(log_string, cmd, expect_fail)

    def exec_cli_rpc(self, log_string, cmd, expect_fail):
        logger.info(log_string)
        output = self.run(cmd)
        if not expect_fail and output[0] != 0:
            self.rpc_failures[time.time()] = cmd
        return output

    # RPC call is stored as key
    # Will overwrite with latest call if exact same call is made multiple times
    def dump_rpc_calls(self):
        logger.info("{} {}, update commands ran. {}".format(
            '='*20, len(self.rpc_updates.keys()), '='*20))
        for cmd in self.rpc_updates:
            logger.info("[DROPLET {}]: Update command ran: {} at {}".format(
                self.id, cmd, self.rpc_updates[cmd]))
        logger.info("{} {}, delete commands ran. {}".format(
            '='*20, len(self.rpc_deletes.keys()), '='*20))
        for cmd in self.rpc_deletes:
            logger.info("[DROPLET {}]: Delete command ran: {} at {}".format(
                self.id, cmd, self.rpc_deletes[cmd]))

    def ovs_add_bridge(self, br):
        logger.info(
            "[DROPLET {}]: Add ovs bridge {}".format(self.id, br))
        script = (f''' bash -c '\
ovs-vsctl add-br {br} && \
ip link set {br} up ' ''')
        self.run(script)

    def ovs_is_exist(self, br):
        cmd = 'ovs-vsctl br-exists {}'.format(br)
        return self.run(cmd)[0] == 0

    def ovs_add_port(self, br, port):
        logger.info(
            "[DROPLET {}]: ovs_add_port to {}".format(self.id, br))
        cmd = 'ovs-vsctl add-port {} {}'.format(br, port)
        self.run(cmd)

        cmd = f''' ovs-vsctl get Interface {port} ofport'''
        return self.run(cmd)[1].rstrip()

    def ovs_add_transit_flow(self, br, in_port, out_port):
        logger.info("[DROPLET {}]: ovs_add_transit_flow {}, {}, {}".format(
            self.id, br, in_port, out_port))
        cmd = f'''ovs-ofctl add-flow {br} priority=50,in_port={in_port},dl_type=0x800,actions=output:{out_port}'''
        self.run(cmd)

        cmd = f'''ovs-ofctl add-flow {br} priority=50,in_port={in_port},dl_type=0x806,actions=output:{out_port}'''
        self.run(cmd)

    def add_vxlan_ofrule(self, br, in_port, out_port, nw_dst):
        cmd = f'''ovs-ofctl add-flow {br} priority=100,in_port={in_port},dl_type=0x800,nw_dst={nw_dst},actions=output:{out_port}'''
        self.run(cmd)

        cmd = f'''ovs-ofctl add-flow {br} priority=100,in_port={in_port},dl_type=0x806,nw_dst={nw_dst},actions=output:{out_port}'''
        self.run(cmd)

    def ovs_create_vxlan_tun_itf(self, br, itf, vxlan_key, remote_ip):
        logger.info(
            "[DROPLET {}]: create_vxlan_tun_itf {}, {}".format(self.id, itf, remote_ip))
        cmd = f'''ovs-vsctl --may-exist \
add-port {br} {itf} \
-- set interface {itf} \
type=vxlan options:remote_ip={remote_ip} \
options:key={vxlan_key}'''
        self.run(cmd)

        cmd = f''' ovs-vsctl get Interface {itf} ofport'''
        return self.run(cmd)[1].rstrip()

    def ovs_create_geneve_tun_itf(self, br, itf, geneve_key, remote_ip):
        logger.info(
            "[DROPLET {}]: ovs_create_geneve_tun_itf {}, {}".format(self.id, itf, remote_ip))
        cmd = f'''ovs-vsctl --may-exist \
add-port {br} {itf} \
-- set interface {itf} \
type=geneve options:remote_ip={remote_ip} \
options:key={geneve_key}'''
        self.run(cmd)
        cmd = f''' ovs-vsctl get Interface {itf} ofport'''
        return self.run(cmd)[1].rstrip()

    def get_substrate_ep_json(self):
        """
        Get a substrate endpoint data to configure XDP programs to
        send packets to this droplet (no ARP for the moment!)
        """
        jsonconf = {
            "tunnel_id": "0",
            "ip": self.ip,
            "eptype": "0",
            "mac": self.mac,
            "veth": "",
            "remote_ips": [""],
            "hosted_iface": ""
        }
        jsonconf = json.dumps(jsonconf)
        return jsonconf

    def run_from_root(self, cmd):
        ret_value = run_cmd(cmd)
        logger.debug(
            "[DROPLET {}]: running from root\n    command:\n {}, \n    exit_code: {},\n    output:\n {}".format(self.id, cmd, ret_value[0], ret_value[1]))
        return ret_value

    def run(self, cmd, detach=False):
        """
        Runs a command directly on the droplet
        """

        ret_value = None
        if (self.droplet_type == 'docker'):
            out = self.container.exec_run(cmd, detach=detach)
            if not detach:
                ret_value = (out.exit_code, out.output.decode("utf-8"))

        logger.info("[DROPLET {}]: running: {}".format(self.id, cmd))
        if (not detach and ret_value[0] == 1):
            logger.error("[DROPLET {}]: {}".format(self.id, ret_value[1]))

        if not detach:
            logger.debug(
                "[DROPLET {}]: running\n    command:\n {}, \n    exit_code: {},\n    output:\n {}".format(self.id, cmd, ret_value[0], ret_value[1]))

        return ret_value

    def _collect_logs(self):
        cmd = f'''
cp /var/log/syslog /trn_test_out/syslog_{self.ip}
        '''
        self.run(cmd)

    def _create_docker_container(self):
        """ Create and initialize a docker container.
        Assumes "buildbox:v2" image exist and setup on host
        """
        cwd = os.getcwd()

        # get a docker client
        docker_client = docker.from_env()
        docker_image = "buildbox:v2"
        mount_pnt = docker.types.Mount("/mnt/Transit",
                                       cwd,
                                       type='bind')

        mount_modules = docker.types.Mount("/lib/modules",
                                           "/lib/modules",
                                           type='bind')

        # Create the contrainer in previlaged mode
        container = docker_client.containers.create(
            docker_image, '/bin/bash', tty=True,
            stdin_open=True, auto_remove=False, mounts=[mount_pnt, mount_modules],
            privileged=True, cap_add=["SYS_PTRACE"],
            security_opt=["seccomp=unconfined"])
        container.start()
        container.reload()

        # Restart dependancy services
        container.exec_run("/etc/init.d/rpcbind restart")
        container.exec_run("/etc/init.d/rsyslog restart")
        container.exec_run("ip link set dev eth0 up mtu 9000")
        container.exec_run("sysctl -w net.ipv4.tcp_mtu_probing=2")

        # We may need ovs for compatability tests
        container.exec_run("/etc/init.d/openvswitch-switch restart")

        # Create simlinks
        container.exec_run("ln -s /mnt/Transit/build/bin /trn_bin")
        container.exec_run("ln -s /mnt/Transit/build/xdp /trn_xdp")
        container.exec_run("ln -s /sys/fs/bpf /bpffs")
        container.exec_run(
            "ln -s /mnt/Transit/test/trn_func_tests/output /trn_test_out")

        # Run the transitd in the background
        container.exec_run("/trn_bin/transitd >/tmp/transitd.std 2>&1 ",
                           detach=True)

        if not self.benchmark:
            # Enable debug and tracing for the kernel
            container.exec_run(
                "mount -t debugfs debugfs /sys/kernel/debug")
            container.exec_run(
                "echo 1 > /sys/kernel/debug/tracing/tracing_on")

        # Enable core dumps (just in case!!)
        container.exec_run("ulimit -u")
        cmd = "echo '/mnt/Transit/core/core_{}_%e.%p' |\
 tee /proc/sys/kernel/core_pattern ".format(self.ip)
        container.exec_run(cmd)

        self.container = container
        self.ip = self.container.attrs['NetworkSettings']['IPAddress']
        self.mac = self.container.attrs['NetworkSettings']['MacAddress']

    def start_pcap(self):
        # Start a pcap to collect packets on eth0
        cmd = f''' bash -c \
'(/mnt/Transit/tools/xdpcap /bpffs/eth0_transit_pcap \
/trn_test_out/\
droplet_{self.ip}.pcap >/dev/null 2>&1 &\
)' '''
        self.run(cmd)

    def delete_container(self):
        if self.container:
            self.container.stop()
            self.container.remove()

    def __del__(self):
        self.unload_transit_xdp()
        self.run("killall5 -2")
        time.sleep(1)
        if 'NOCLEANUP' in os.environ:
            return
        self.delete_container()

    def clear_update_call_state(self):
        self.rpc_updates = {}

    def do_delete_decrement(self, log_string, cmd, expect_fail, key, update_counts):
        if update_counts[key] > 0:
            update_counts[key] -= 1
            if update_counts[key] == 0:
                self.rpc_deletes[key] = time.time()
                self.exec_cli_rpc(log_string, cmd, expect_fail)

    def do_update_increment(self, log_string, cmd, expect_fail, key, update_counts):
        if key in update_counts.keys():
            update_counts[key] += 1
        else:
            update_counts[key] = 1
        self.rpc_updates[key] = time.time()
        self.exec_cli_rpc(log_string, cmd, expect_fail)

    def dump_num_calls(self):
        for cmd in self.vpc_updates:
            logger.info("[DROPLET {}]: vpc_updates commands ran: {}  {}".format(
                self.id, cmd, self.vpc_updates[cmd]))
        for cmd in self.substrate_updates:
            logger.info("[DROPLET {}]: substrate_updates commands ran: {}  {}".format(
                self.id, cmd, self.substrate_updates[cmd]))
        for cmd in self.endpoint_updates:
            logger.info("[DROPLET {}]: endpoint_updates commands ran: {}  {}".format(
                self.id, cmd, self.endpoint_updates[cmd]))

    def dump_pcap_on_host(self, pcapfile, timeout=5):
        """
        Does tcpdump to a pcap file for "n" seconds on the host veth device
        corresponding to the eth0 interface inside the docker container.
        """
        veth_index = self.run(
            "cat /sys/class/net/eth0/iflink")[1].strip()
        veth = self.run_from_root(
            "grep -l " + veth_index + " /sys/class/net/veth*/ifindex")[1].split("/")[4]
        run_cmd("timeout " + str(timeout) + " tcpdump -nn -A -i " + veth + " -w test/trn_func_tests/output/" +
                self.ip + "_" + pcapfile + "_dump.pcap >/dev/null 2>&1 &")
