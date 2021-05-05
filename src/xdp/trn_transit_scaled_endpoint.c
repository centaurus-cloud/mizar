// SPDX-License-Identifier: GPL-2.0-or-later
/**
 * @file trn_transit_xdp.c
 * @author Sherif Abdelwahab (@zasherif)
 *
 * @brief Implements the Transit XDP program (switching and routing logic)
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

#include <linux/bpf.h>
#include <linux/if_arp.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <linux/if_vlan.h>
#include <linux/in.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/pkt_cls.h>
#include <linux/socket.h>
#include <linux/tcp.h>
#include <linux/types.h>
#include <linux/udp.h>
#include <stddef.h>
#include <string.h>

#include "extern/bpf_endian.h"
#include "extern/bpf_helpers.h"

#include "trn_datamodel.h"
#include "trn_kern.h"
#include "trn_transit_xdp_stages_maps.h"

int _version SEC("version") = 1;

static __inline int trn_sep_rewrite_remote_mac(struct transit_packet *pkt,
					       void *endpoints_map)
{
	struct endpoint_t *remote_ep = NULL;
	struct endpoint_key_t epkey = {};

	int map_idx = 0;
	void *interfaces_map =
		bpf_map_lookup_elem(&interfaces_map_ref, &map_idx);
	if (!interfaces_map) {
		return XDP_DROP;
	}

	/* The TTL must have been decremented before this step, Drop the
	packet if TTL is zero */
	if (!pkt->ip->ttl)
		return XDP_DROP;

	epkey.tunip[0] = 0;
	epkey.tunip[1] = 0;
	epkey.tunip[2] = pkt->ip->daddr;

	/* Get the remote_mac address based on the value of the outer dest IP */
	remote_ep = bpf_map_lookup_elem(endpoints_map, &epkey);

	if (!remote_ep) {
		bpf_debug("[Transit:%d:] DROP: "
			  "Failed to find remote MAC address\n",
			  __LINE__);
		return XDP_DROP;
	}

	trn_set_src_mac(pkt->data, pkt->eth->h_dest);
	trn_set_dst_mac(pkt->data, remote_ep->mac);
	return bpf_redirect_map(interfaces_map, pkt->itf_idx, 0);
}

static __inline int trn_sep_decapsulate_and_redirect(struct transit_packet *pkt,
						     int ifindex)
{
	int outer_header_size = sizeof(*pkt->geneve) + pkt->gnv_opt_len +
				sizeof(*pkt->udp) + sizeof(*pkt->ip) +
				sizeof(*pkt->eth);

	int map_idx = 0;
	void *interfaces_map =
		bpf_map_lookup_elem(&interfaces_map_ref, &map_idx);
	if (!interfaces_map) {
		return XDP_DROP;
	}

	if (bpf_xdp_adjust_head(pkt->xdp, 0 + outer_header_size)) {
		return XDP_DROP;
	}

	return bpf_redirect_map(interfaces_map, ifindex, 0);
}

static __inline int trn_sep_router_handle_pkt(struct transit_packet *pkt,
					      __u32 inner_src_ip,
					      __u32 inner_dst_ip,
					      void *endpoints_map)
{
	__be64 tunnel_id = trn_vni_to_tunnel_id(pkt->geneve->vni);
	int map_idx = 0;
	__u32 swidx = 0;
	struct vpc_key_t vpckey = {};
	struct vpc_t *vpc = NULL;
	void *vpc_map = NULL;
	__u32 routeridx = 0;
	struct network_key_t nkey = {};
	struct network_t *net = NULL;

	nkey.prefixlen = 96;
	__builtin_memcpy(&nkey.nip[0], &tunnel_id, sizeof(tunnel_id));
	nkey.nip[2] = inner_dst_ip;

	void *networks_map = bpf_map_lookup_elem(&networks_map_ref, &map_idx);
	if (!networks_map) {
		return XDP_DROP;
	}

	net = bpf_map_lookup_elem(networks_map, &nkey);

	if (net) {
		bpf_debug("[Transit::] LPM found [0x%x:0x%x:0x%x]!\n",
			  net->nip[0], net->nip[1], net->nip[2]);

		if (net->nip[0] != nkey.nip[0] || net->nip[1] != nkey.nip[1]) {
			bpf_debug(
				"[Transit:%d:] DROP (BUG): Network tunnel_id cannot be different from key!\n",
				__LINE__);
			return XDP_DROP;
		}

		swidx = jhash_2words(inner_src_ip, inner_dst_ip,
				     INIT_JHASH_SEED) %
			net->nswitches;

		if (swidx > TRAN_MAX_NSWITCH - 1) {
			bpf_debug(
				"[Transit:%d:] DROP (BUG): hash switch index is greater than maximum number of switches!\n",
				__LINE__);
			return XDP_DROP;
		}

		bpf_debug("[Transit:%d:] Sending packet to switch!\n",
			  __LINE__);

		trn_set_src_dst_ip_csum(pkt, pkt->ip->daddr,
					net->switches_ips[swidx]);
		return trn_sep_rewrite_remote_mac(pkt, endpoints_map);
	}

	/* Now forward the packet to the VPC router */
	vpc_map = bpf_map_lookup_elem(&vpc_map_ref, &map_idx);
	if (!vpc_map) {
		return XDP_DROP;
	}

	vpckey.tunnel_id = tunnel_id;
	vpc = bpf_map_lookup_elem(vpc_map, &vpckey);

	if (!vpc) {
		bpf_debug(
			"[Transit:%d:0x%x] DROP (BUG): Missing VPC router data!\n",
			__LINE__, bpf_ntohl(pkt->itf_ipv4));
		return XDP_DROP;
	}

	routeridx = jhash_2words(inner_src_ip, inner_dst_ip, INIT_JHASH_SEED) %
		    vpc->nrouters;

	if (routeridx > TRAN_MAX_NROUTER - 1) {
		bpf_debug(
			"[Transit:%d:] DROP (BUG): hash router index %u is greater than maximum number of routers %u!",
			__LINE__, routeridx, TRAN_MAX_NROUTER);
		return XDP_DROP;
	}

	bpf_debug("[Transit:%d:] Sending packet to router!\n", __LINE__);
	trn_set_src_dst_ip_csum(pkt, pkt->ip->daddr,
				vpc->routers_ips[routeridx]);
	return trn_sep_rewrite_remote_mac(pkt, endpoints_map);
}

static __inline int trn_sep_switch_handle_pkt(struct transit_packet *pkt,
					      __u32 inner_src_ip,
					      __u32 inner_dst_ip,
					      void *endpoints_map)
{
	__be64 tunnel_id = trn_vni_to_tunnel_id(pkt->geneve->vni);
	struct endpoint_t *ep = NULL;
	struct endpoint_key_t epkey = {};

	__builtin_memcpy(&epkey.tunip[0], &tunnel_id, sizeof(tunnel_id));
	epkey.tunip[2] = inner_dst_ip;

	/* Get the remote_ip based on the value of the inner dest IP and VNI*/
	ep = bpf_map_lookup_elem(endpoints_map, &epkey);

	if (!ep) {
		return trn_sep_router_handle_pkt(pkt, inner_src_ip,
						 inner_dst_ip, endpoints_map);
	}

	/* The packet may be sent first to a gw mac address */
	trn_set_dst_mac(pkt->inner_eth, ep->mac);

	// The destination endpoint is in this host, redirect the packet to it
	if (ep->hosted_iface != -1) {
		return trn_sep_decapsulate_and_redirect(pkt, ep->hosted_iface);
	}

	// Currently a scaled endpoint shall not be a backend to another scaled ep
	if (ep->eptype == TRAN_SCALED_EP) {
		bpf_debug(
			"[Transit:%d:] DROP (BUG): Misconfigured scaled endpoint with a sep backend!\n",
			__LINE__);
		return XDP_DROP;
	}

	if (ep->nremote_ips == 0) {
		bpf_debug(
			"[Transit:%d:] DROP (BUG): Misconfigured endpoint with zero remote_ips!\n",
			__LINE__);
		return XDP_DROP;
	}

	trn_set_src_dst_ip_csum(pkt, pkt->ip->daddr, ep->remote_ips[0]);
	return trn_sep_rewrite_remote_mac(pkt, endpoints_map);
}

static __inline int trn_sep_handle_scaled_ep_modify(struct transit_packet *pkt,
						    void *fwd_flow_mod_cache,
						    void *rev_flow_mod_cache,
						    void *endpoints_map)
{
	struct scaled_endpoint_remote_t out_tuple = {};
	struct ipv4_tuple_t in_tuple = {};

	struct scaled_endpoint_remote_t rev_out_tuple = {};
	struct ipv4_tuple_t rev_in_tuple = {};

	__builtin_memcpy(&out_tuple, &pkt->scaled_ep_opt->scaled_ep_data.target,
			 sizeof(struct scaled_endpoint_remote_t));

	__builtin_memcpy(&in_tuple, &pkt->inner_ipv4_tuple,
			 sizeof(struct ipv4_tuple_t));

	/* First update the forward flow mod cache*/
	bpf_map_update_elem(fwd_flow_mod_cache, &in_tuple, &out_tuple, 0);

	/* Prepare the inner packet for forwarding*/
	//TODO: for now only change the IP addresses (no ports)

	if (pkt->inner_ip + 1 > pkt->data_end) {
		return XDP_ABORTED;
	}

	if (pkt->inner_ipv4_tuple.protocol == IPPROTO_TCP) {
		pkt->inner_tcp = (void *)pkt->inner_ip + sizeof(*pkt->inner_ip);
		if (pkt->inner_tcp + 1 > pkt->data_end) {
			return XDP_ABORTED;
		}
	}

	if (pkt->inner_ipv4_tuple.protocol == IPPROTO_UDP) {
		pkt->inner_udp = (void *)pkt->inner_ip + sizeof(*pkt->inner_ip);

		if (pkt->inner_udp + 1 > pkt->data_end) {
			return XDP_ABORTED;
		}
	}

	trn_set_src_dst_inner_ip_csum(pkt, out_tuple.saddr, out_tuple.daddr);

	/* Prepare the outer packet for forwarding*/
	trn_reset_rts_opt(pkt);
	trn_reset_scaled_ep_opt(pkt);

	/* Now reverse the tuple and update the reverse flow mod */
	rev_in_tuple.saddr = out_tuple.daddr;
	rev_in_tuple.daddr = out_tuple.saddr;
	rev_in_tuple.protocol = in_tuple.protocol;
	rev_in_tuple.sport = out_tuple.dport;
	rev_in_tuple.dport = out_tuple.sport;

	rev_out_tuple.daddr = in_tuple.saddr;
	rev_out_tuple.saddr = in_tuple.daddr;
	rev_out_tuple.sport = in_tuple.dport;
	rev_out_tuple.dport = in_tuple.sport;

	__builtin_memcpy(&rev_out_tuple.h_source, pkt->inner_eth->h_dest,
			 ETH_ALEN * sizeof(pkt->inner_eth->h_dest[0]));

	__builtin_memcpy(&rev_out_tuple.h_dest, pkt->inner_eth->h_source,
			 ETH_ALEN * sizeof(pkt->inner_eth->h_source[0]));

	bpf_map_update_elem(rev_flow_mod_cache, &rev_in_tuple, &rev_out_tuple,
			    0);

	return trn_sep_switch_handle_pkt(pkt, pkt->inner_ip->saddr,
					 pkt->inner_ip->daddr, endpoints_map);
}

static __inline int trn_scaled_ep_decide(struct transit_packet *pkt)
{
	void *endpoints_map = NULL;
	void *port_map = NULL;
	void *fwd_flow_mod_cache = NULL;
	void *rev_flow_mod_cache = NULL;
	struct endpoint_t *ep = NULL;
	struct endpoint_key_t epkey = {};
	struct port_t *port = NULL;
	struct port_key_t portkey = {};
	int map_idx = 0;
	__u32 inhash = 0;
	__u32 remote_idx = 0;
	__be64 tunnel_id = trn_vni_to_tunnel_id(pkt->geneve->vni);

	endpoints_map = bpf_map_lookup_elem(&endpoints_map_ref, &map_idx);
	if (!endpoints_map) {
		bpf_debug("[Scaled_EP:%d:] failed to find endpoints_map\n",
			  __LINE__);
		return XDP_ABORTED;
	}

	port_map = bpf_map_lookup_elem(&port_map_ref, &map_idx);
	if (!port_map) {
		bpf_debug("[Scaled_EP:%d:] failed to find port_map\n",
			  __LINE__);
		return XDP_ABORTED;
	}

	fwd_flow_mod_cache =
		bpf_map_lookup_elem(&fwd_flow_mod_cache_ref, &map_idx);
	if (!fwd_flow_mod_cache) {
		return XDP_DROP;
	}

	rev_flow_mod_cache =
		bpf_map_lookup_elem(&rev_flow_mod_cache_ref, &map_idx);
	if (!rev_flow_mod_cache) {
		return XDP_DROP;
	}

	__builtin_memcpy(&epkey.tunip[0], &tunnel_id, sizeof(tunnel_id));
	epkey.tunip[2] = pkt->inner_ipv4_tuple.daddr;

	__builtin_memcpy(&portkey.tunip[0], &tunnel_id, sizeof(tunnel_id));
	portkey.tunip[2] = pkt->inner_ipv4_tuple.daddr;
	portkey.port = pkt->inner_ipv4_tuple.dport;
	portkey.protocol = pkt->inner_ipv4_tuple.protocol;
	port = bpf_map_lookup_elem(port_map, &portkey);

	if (!port) {
		bpf_debug(
			"[Scaled_EP:%d:] DROP failed to find scaled endpoint port and protocol.\n",
			__LINE__);
		return XDP_DROP;
	}

	/* Get the scaled endpoint configuration */
	ep = bpf_map_lookup_elem(endpoints_map, &epkey);

	if (!ep) {
		bpf_debug(
			"[Scaled_EP:%d:] (BUG) failed to find scaled endpoint configuration\n",
			__LINE__);
		return XDP_ABORTED;
	}

	/* Simple hashing for now! */
	inhash = jhash_2words(pkt->inner_ipv4_tuple.saddr,
			      pkt->inner_ipv4_tuple.sport, INIT_JHASH_SEED);

	if (ep->nremote_ips == 0) {
		bpf_debug(
			"[Scaled_EP] DROP: no backend attached to scaled endpoint 0x%x!\n",
			bpf_ntohl(pkt->inner_ipv4_tuple.daddr));
		return XDP_DROP;
	}

	remote_idx = inhash % ep->nremote_ips;

	if (remote_idx > TRAN_MAX_REMOTES - 1) {
		bpf_debug(
			"[Scaled_EP] DROP (BUG): Selected remote index [%d] "
			"is greater than maximum number of supported remote endpoints [%d]!\n",
			remote_idx, TRAN_MAX_REMOTES);
		return XDP_ABORTED;
	}

	pkt->scaled_ep_opt->opt_class = TRN_GNV_OPT_CLASS;
	pkt->scaled_ep_opt->type = TRN_GNV_SCALED_EP_OPT_TYPE;
	pkt->scaled_ep_opt->length = sizeof(struct trn_gnv_scaled_ep_data) / 4;
	pkt->scaled_ep_opt->scaled_ep_data.msg_type = TRN_SCALED_EP_MODIFY;

	pkt->scaled_ep_opt->scaled_ep_data.target.daddr =
		ep->remote_ips[remote_idx];

	pkt->scaled_ep_opt->scaled_ep_data.target.saddr =
		pkt->inner_ipv4_tuple.saddr;

	pkt->scaled_ep_opt->scaled_ep_data.target.sport =
		pkt->inner_ipv4_tuple.sport;

	pkt->scaled_ep_opt->scaled_ep_data.target.dport = port->target_port;

	__builtin_memcpy(&pkt->scaled_ep_opt->scaled_ep_data.target.h_source,
			 pkt->inner_eth->h_source,
			 ETH_ALEN * sizeof(pkt->inner_eth->h_source[0]));

	__builtin_memcpy(&pkt->scaled_ep_opt->scaled_ep_data.target.h_dest,
			 pkt->inner_eth->h_dest,
			 ETH_ALEN * sizeof(pkt->inner_eth->h_dest[0]));

	/* If the source endpoint is on the same bouncer host process the option*/
	if (pkt->ip->daddr == pkt->ip->saddr) {
		return trn_sep_handle_scaled_ep_modify(pkt, fwd_flow_mod_cache,
						       rev_flow_mod_cache,
						       endpoints_map);
	}

	/*Reset rts for now, todo: add endpoint host in an rts opt to minimize hop counts*/
	trn_reset_rts_opt(pkt);

	trn_set_src_dst_ip_csum(pkt, pkt->ip->daddr, pkt->ip->saddr);

	trn_swap_src_dst_mac(pkt->data);

	bpf_debug("[Scaled_EP:%d:] ** scaled endpoint to 0x%x!!\n", __LINE__,
		  bpf_ntohl(pkt->scaled_ep_opt->scaled_ep_data.target.daddr));
	return XDP_TX;
}

static __inline int trn_sep_process_inner_udp(struct transit_packet *pkt)
{
	pkt->inner_udp = (void *)pkt->inner_ip + sizeof(*pkt->inner_ip);

	if (pkt->inner_udp + 1 > pkt->data_end) {
		bpf_debug("[Scaled_EP:%d:0x%x] ABORTED: Bad offset\n", __LINE__,
			  bpf_ntohl(pkt->itf_ipv4));
		return XDP_ABORTED;
	}

	bpf_debug("[Scaled_EP:%d:0x%x] Process UDP \n", __LINE__,
		  bpf_ntohl(pkt->itf_ipv4));

	pkt->inner_ipv4_tuple.sport = pkt->inner_udp->source;
	pkt->inner_ipv4_tuple.dport = pkt->inner_udp->dest;

	return trn_scaled_ep_decide(pkt);
}

static __inline int trn_sep_process_inner_tcp(struct transit_packet *pkt)
{
	pkt->inner_tcp = (void *)pkt->inner_ip + sizeof(*pkt->inner_ip);

	if (pkt->inner_tcp + 1 > pkt->data_end) {
		bpf_debug("[Scaled_EP:%d:0x%x] ABORTED: Bad offset\n", __LINE__,
			  bpf_ntohl(pkt->itf_ipv4));
		return XDP_ABORTED;
	}

	pkt->inner_ipv4_tuple.sport = pkt->inner_tcp->source;
	pkt->inner_ipv4_tuple.dport = pkt->inner_tcp->dest;

	bpf_debug("[Scaled_EP:%d:0x%x] Process TCP\n", __LINE__,
		  bpf_ntohl(pkt->itf_ipv4));

	return trn_scaled_ep_decide(pkt);
}

static __inline int trn_sep_process_inner_icmp(struct transit_packet *pkt)
{
	bpf_debug(
		"[Scaled_EP:%d:] scaled endpoint 0x%x does not handle ICMP!!\n",
		__LINE__, bpf_ntohl(pkt->inner_ip->daddr));

	// TODO: return XDP_DROP
	pkt->inner_ipv4_tuple.sport = 0;
	pkt->inner_ipv4_tuple.dport = 0;

	return trn_scaled_ep_decide(pkt);
}

static __inline int trn_sep_process_inner_ip(struct transit_packet *pkt)
{
	pkt->inner_ip = (void *)pkt->inner_eth + pkt->inner_eth_off;

	if (pkt->inner_ip + 1 > pkt->data_end) {
		bpf_debug("[Scaled_EP:%d:0x%x] ABORTED: Bad offset\n", __LINE__,
			  bpf_ntohl(pkt->itf_ipv4));
		return XDP_ABORTED;
	}

	pkt->inner_ipv4_tuple.saddr = pkt->inner_ip->saddr;
	pkt->inner_ipv4_tuple.daddr = pkt->inner_ip->daddr;
	pkt->inner_ipv4_tuple.protocol = pkt->inner_ip->protocol;

	if (pkt->inner_ipv4_tuple.protocol == IPPROTO_UDP) {
		return trn_sep_process_inner_udp(pkt);
	}

	if (pkt->inner_ipv4_tuple.protocol == IPPROTO_TCP) {
		return trn_sep_process_inner_tcp(pkt);
	}

	if (pkt->inner_ipv4_tuple.protocol == IPPROTO_ICMP) {
		return trn_sep_process_inner_icmp(pkt);
	}

	bpf_debug("[Scaled_EP:%d:0x%x] Unsupported inner protocol \n", __LINE__,
		  bpf_ntohl(pkt->itf_ipv4));

	return XDP_DROP;
}

static __inline int trn_sep_process_inner_eth(struct transit_packet *pkt)
{
	pkt->inner_eth = (void *)pkt->geneve + pkt->gnv_hdr_len;
	pkt->inner_eth_off = sizeof(*pkt->inner_eth);

	if (pkt->inner_eth + 1 > pkt->data_end) {
		bpf_debug("[Scaled_EP:%d:0x%x] ABORTED: Bad offset\n", __LINE__,
			  bpf_ntohl(pkt->itf_ipv4));
		return XDP_ABORTED;
	}

	if (pkt->eth->h_proto != bpf_htons(ETH_P_IP)) {
		bpf_debug(
			"[Scaled_EP:%d:0x%x] DROP: unsupported inner packet: [0x%x]\n",
			__LINE__, bpf_ntohl(pkt->itf_ipv4),
			bpf_ntohs(pkt->eth->h_proto));
		return XDP_DROP;
	}

	return trn_sep_process_inner_ip(pkt);
}

static __inline int trn_sep_process_geneve(struct transit_packet *pkt)
{
	pkt->geneve = (void *)pkt->udp + sizeof(*pkt->udp);
	if (pkt->geneve + 1 > pkt->data_end) {
		bpf_debug("[Scaled_EP:%d:0x%x] ABORTED: Bad offset\n", __LINE__,
			  bpf_ntohl(pkt->itf_ipv4));
		return XDP_ABORTED;
	}

	pkt->gnv_opt_len = pkt->geneve->opt_len * 4;
	pkt->gnv_hdr_len = sizeof(*pkt->geneve) + pkt->gnv_opt_len;
	pkt->rts_opt = (void *)&pkt->geneve->options[0];

	if (pkt->rts_opt + 1 > pkt->data_end) {
		bpf_debug("[Scaled_EP:%d:0x%x] ABORTED: Bad offset\n", __LINE__,
			  bpf_ntohl(pkt->itf_ipv4));
		return XDP_ABORTED;
	}

	pkt->scaled_ep_opt = (void *)pkt->rts_opt + sizeof(*pkt->rts_opt);

	if (pkt->scaled_ep_opt + 1 > pkt->data_end) {
		bpf_debug("[Scaled_EP:%d:0x%x] ABORTED: Bad offset\n", __LINE__,
			  bpf_ntohl(pkt->itf_ipv4));
		return XDP_ABORTED;
	}

	return trn_sep_process_inner_eth(pkt);
}

static __inline int trn_sep_process_udp(struct transit_packet *pkt)
{
	/* Get the UDP header */
	pkt->udp = (void *)pkt->ip + sizeof(*pkt->ip);

	if (pkt->udp + 1 > pkt->data_end) {
		bpf_debug("[Scaled_EP:%d:0x%x] ABORTED: Bad offset\n", __LINE__,
			  bpf_ntohl(pkt->itf_ipv4));
		return XDP_ABORTED;
	}

	return trn_sep_process_geneve(pkt);
}

static __inline int trn_sep_process_ip(struct transit_packet *pkt)
{
	/* Get the IP header */
	pkt->ip = (void *)pkt->eth + pkt->eth_off;

	if (pkt->ip + 1 > pkt->data_end) {
		bpf_debug("[Scaled_EP:%d:0x%x] ABORTED: Bad offset\n", __LINE__,
			  bpf_ntohl(pkt->itf_ipv4));
		return XDP_ABORTED;
	}

	if (!pkt->ip->ttl)
		return XDP_DROP;

	return trn_sep_process_udp(pkt);
}

static __inline int trn_sep_process_eth(struct transit_packet *pkt)
{
	pkt->eth = pkt->data;
	pkt->eth_off = sizeof(*pkt->eth);

	if (pkt->data + pkt->eth_off > pkt->data_end) {
		bpf_debug("[Scaled_EP:%d:0x%x] ABORTED: Bad offset\n", __LINE__,
			  bpf_ntohl(pkt->itf_ipv4));
		return XDP_ABORTED;
	}

	return trn_sep_process_ip(pkt);
}

SEC("transit_scaled_endpoint")
int _transit_scaled_endpoint(struct xdp_md *ctx)
{
	/* A Simple scaled endpoint implementation */

	void *interface_config_map = NULL;
	struct transit_packet pkt = {};
	int map_idx = 0;
	pkt.data = (void *)(long)ctx->data;
	pkt.data_end = (void *)(long)ctx->data_end;
	pkt.xdp = ctx;
	struct tunnel_iface_t *itf = NULL;

	interface_config_map =
		bpf_map_lookup_elem(&interface_config_map_ref, &map_idx);
	if (!interface_config_map) {
		bpf_debug(
			"[Scaled_EP:%d:] failed to find interface_config_map\n",
			__LINE__);
		return XDP_ABORTED;
	}

	int k = 0;
	itf = bpf_map_lookup_elem(interface_config_map, &k);

	if (!itf) {
		bpf_debug("[Scaled_EP:%d:] ABORTED: Bad configuration\n",
			  __LINE__);
		return XDP_ABORTED;
	}

	pkt.itf_ipv4 = itf->ip;
	pkt.itf_idx = itf->iface_index;

	return trn_sep_process_eth(&pkt);
}

char _license[] SEC("license") = "GPL";
