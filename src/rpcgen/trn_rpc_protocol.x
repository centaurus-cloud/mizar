/* SPDX-License-Identifier: GPL-2.0-or-later */
/**
* @file      trn_rpc_protocol.x
* @author    Sherif Abdelwahab,  <@zasherif>
*
* @brief Defines an internal protocol to manage the data-plane.
*
* @copyright Copyright (c) 2019 The Authors.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; version 2 of the License.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along
* with this program; if not, write to the Free Software Foundation, Inc.,
* 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*
*/

%// SPDX-License-Identifier: GPL-2.0-or-later
%#pragma GCC diagnostic ignored "-Wunused-variable"
%#include <stdint.h>

/*----- Data types. ----- */

/* Upper limit on maximum number of enpoint hosts */
const RPC_TRN_MAX_REMOTE_IPS = 256;

/* Upper limit on maximum numbber of transit switches in a network */
const RPC_TRN_MAX_NET_SWITCHES = 256;

/* Upper limit on maximum numbber of transit routers in a vpc */
const RPC_TRN_MAX_VPC_ROUTERS = 256;

/* Defines generic codes, 0 is always a success need not to mention! */
const RPC_TRN_WARN = 1;
const RPC_TRN_ERROR = 2;
const RPC_TRN_FATAL = 3;
const RPC_TRN_NOT_IMPLEMENTED = 4;

/* Defines a network (subnet or group) */
struct rpc_trn_network_t {
       string interface<20>;
       uint32_t prefixlen;
       uint64_t tunid;
       uint32_t netip;
       uint32_t switches_ips<RPC_TRN_MAX_NET_SWITCHES>;
};

/* Defines a unique key to get/delete a network (in DP) */
struct rpc_trn_network_key_t {
       string interface<20>;
       uint32_t prefixlen;
       uint64_t tunid;
       uint32_t netip;
};

/* Defines an endpoint (all types) */
struct rpc_trn_endpoint_t {
       string interface<20>;
       uint32_t ip;
       uint32_t eptype;
       uint32_t remote_ips<RPC_TRN_MAX_REMOTE_IPS>;
       unsigned char mac[6];
       string hosted_interface<20>;
       string veth<20>;
       uint64_t tunid;
};

/* Defines a unique key to get/delete an RP (in DP) */
struct rpc_trn_endpoint_key_t {
       string interface<20>;
       uint64_t tunid;
       uint32_t ip;
};

/* Defines an packet metadata */
struct rpc_trn_packet_metadata_t {
       string interface<20>;
       uint32_t ip;
       uint64_t tunid;
       uint32_t pod_label_value;
       uint32_t namespace_label_value;
};

/* Defines a unique key to get/delete an packet metadata */
struct rpc_trn_packet_metadata_key_t {
       string interface<20>;
       uint64_t tunid;
       uint32_t ip;
};

/* Defines a port */
struct rpc_trn_port_t {
       string interface<20>;
       uint32_t ip;
       uint64_t tunid;
       uint16_t port;
       uint16_t target_port;
       uint8_t protocol;
};

/* Defines a unique key to get/delete an RP (in DP) */
struct rpc_trn_port_key_t {
       string interface<20>;
       uint64_t tunid;
       uint32_t ip;
       uint16_t port;
       uint8_t protocol;
};

/* Defines a VPC */
struct rpc_trn_vpc_t {
       string interface<20>;
       uint64_t tunid;
       uint32_t routers_ips<RPC_TRN_MAX_VPC_ROUTERS>;
};

/* Defines a unique key to get/delete a VPC (in DP) */
struct rpc_trn_vpc_key_t {
       string interface<20>;
       uint64_t tunid;
};

/* Defines an interface and a path for xdp prog to load on the interface */
struct rpc_trn_xdp_intf_t {
       string interface<20>;
       string xdp_path<256>;
       string pcapfile<256>;
       uint8_t xdp_flag;
};

/* Defines an interface */
struct rpc_intf_t {
       string interface<20>;
};

/* Defines a tunneling interface (physical) */
struct rpc_trn_tun_intf_t {
       string interface<20>;
       uint32_t ip;
       unsigned char mac[6];
};

/* Defines an endpoint associated with transit agent */
struct rpc_trn_agent_metadata_t {
       string interface<20>;
       rpc_trn_tun_intf_t eth;
       rpc_trn_endpoint_t ep;
       rpc_trn_network_t net;
};

enum rpc_trn_pipeline_stage {
       ON_XDP_TX       = 0,
       ON_XDP_PASS     = 1,
       ON_XDP_REDIRECT = 2,
       ON_XDP_DROP     = 3,
       ON_XDP_SCALED_EP = 4
       /* add stages */
};

/* Defines an XDP program at xdp_path to be loaded at index prog_index */
struct rpc_trn_ebpf_prog_t {
       string interface<20>;
       rpc_trn_pipeline_stage stage;
       string xdp_path<256>;
};

/* Defines an XDP program at stage */
struct rpc_trn_ebpf_prog_stage_t {
       string interface<20>;
       rpc_trn_pipeline_stage stage;
};

/* Defines a network policy table entry */
struct rpc_trn_vsip_cidr_t {
       string interface<20>;
       uint64_t tunid;
       uint32_t local_ip;
       uint32_t cidr_ip;
       uint32_t cidr_prefixlen;
       int cidr_type;
       uint64_t bit_val;
};

/* Defines a network policy table key */
struct rpc_trn_vsip_cidr_key_t {
       string interface<20>;
       uint64_t tunid;
       uint32_t local_ip;
       uint32_t cidr_ip;
       uint32_t cidr_prefixlen;
       int cidr_type;
};

/* Defines a network policy enforcement table key */
struct rpc_trn_vsip_enforce_t {
       string interface<20>;
       uint64_t tunid;
       uint32_t local_ip;
};

/* Defines a network policy protocol port table entry */
struct rpc_trn_vsip_ppo_t {
       string interface<20>;
       uint64_t tunid;
       uint32_t local_ip;
       uint8_t proto;
       uint16_t port;
       uint64_t bit_val;
};

/* Defines a network policy protocol port table key */
struct rpc_trn_vsip_ppo_key_t {
       string interface<20>;
       uint64_t tunid;
       uint32_t local_ip;
       uint8_t proto;
       uint16_t port;
};

/*----- Protocol. -----*/

program RPC_TRANSIT_REMOTE_PROTOCOL {
        version RPC_TRANSIT_ALFAZERO {
                int UPDATE_VPC(rpc_trn_vpc_t) = 1;
                int UPDATE_NET(rpc_trn_network_t) = 2;
                int UPDATE_EP(rpc_trn_endpoint_t) = 3;
                int UPDATE_PORT(rpc_trn_port_t) = 4;
                int UPDATE_AGENT_EP(rpc_trn_endpoint_t) = 5;
                int UPDATE_AGENT_MD(rpc_trn_agent_metadata_t) = 6;

                int DELETE_VPC(rpc_trn_vpc_key_t) = 7;
                int DELETE_NET(rpc_trn_network_key_t) = 8;
                int DELETE_EP(rpc_trn_endpoint_key_t) = 9;
                int DELETE_AGENT_EP(rpc_trn_endpoint_key_t) = 10;
                int DELETE_AGENT_MD(rpc_intf_t) = 11;

                rpc_trn_vpc_t      GET_VPC(rpc_trn_vpc_key_t) = 12;
                rpc_trn_network_t  GET_NET(rpc_trn_network_key_t) = 13;
                rpc_trn_endpoint_t GET_EP(rpc_trn_endpoint_key_t) = 14;
                rpc_trn_endpoint_t GET_AGENT_EP(rpc_trn_endpoint_key_t) = 15;
                rpc_trn_agent_metadata_t GET_AGENT_MD(rpc_intf_t) = 16;

                int LOAD_TRANSIT_XDP(rpc_trn_xdp_intf_t) = 17;
                int LOAD_TRANSIT_AGENT_XDP(rpc_trn_xdp_intf_t) = 18;

                int UNLOAD_TRANSIT_XDP(rpc_intf_t) = 19;
                int UNLOAD_TRANSIT_AGENT_XDP(rpc_intf_t) = 20;

                int LOAD_TRANSIT_XDP_PIPELINE_STAGE(rpc_trn_ebpf_prog_t) = 21;
                int UNLOAD_TRANSIT_XDP_PIPELINE_STAGE(rpc_trn_ebpf_prog_stage_t) = 22;

                int UPDATE_TRANSIT_NETWORK_POLICY(rpc_trn_vsip_cidr_t) = 23;
                int DELETE_TRANSIT_NETWORK_POLICY(rpc_trn_vsip_cidr_key_t) = 24;
                int UPDATE_TRANSIT_NETWORK_POLICY_ENFORCEMENT(rpc_trn_vsip_enforce_t) = 25;
                int DELETE_TRANSIT_NETWORK_POLICY_ENFORCEMENT(rpc_trn_vsip_enforce_t) = 26;
                int UPDATE_TRANSIT_NETWORK_POLICY_PROTOCOL_PORT(rpc_trn_vsip_ppo_t) = 27;
                int DELETE_TRANSIT_NETWORK_POLICY_PROTOCOL_PORT(rpc_trn_vsip_ppo_key_t) = 28;
                int UPDATE_AGENT_NETWORK_POLICY(rpc_trn_vsip_cidr_t) = 29;
                int DELETE_AGENT_NETWORK_POLICY(rpc_trn_vsip_cidr_key_t) = 30;
                int UPDATE_AGENT_NETWORK_POLICY_ENFORCEMENT(rpc_trn_vsip_enforce_t) = 31;
                int DELETE_AGENT_NETWORK_POLICY_ENFORCEMENT(rpc_trn_vsip_enforce_t) = 32;
                int UPDATE_AGENT_NETWORK_POLICY_PROTOCOL_PORT(rpc_trn_vsip_ppo_t) = 33;
                int DELETE_AGENT_NETWORK_POLICY_PROTOCOL_PORT(rpc_trn_vsip_ppo_key_t) = 34;

                int UPDATE_PACKET_METADATA(rpc_trn_packet_metadata_t) = 35;
                int DELETE_PACKET_METADATA(rpc_trn_packet_metadata_key_t) = 36;                

          } = 1;

} =  0x20009051;
