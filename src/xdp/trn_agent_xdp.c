// SPDX-License-Identifier: GPL-2.0-or-later
/**
 * @file trn_agent_xdp.c
 * @author Sherif Abdelwahab (@zasherif)
 *
 * @brief Transit agent XDP program
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
#include "trn_agent_xdp_maps.h"
#include "trn_kern.h"
#include "conntrack_common.h"

int _version SEC("version") = 1;
#define SPORT_MAX 6553
#define SPORT_MIN 1024
#define SPORT_BASE (SPORT_MAX - SPORT_MIN)

static __inline int trn_select_transit_switch(struct transit_packet *pkt,
					      struct agent_metadata_t *metadata,
					      __be64 tun_id, __u32 in_src_ip,
					      __u32 in_dst_ip, __u16 *s_port,
					      __u32 *d_addr)
{
	/* Compute the source port and the transit switch address based on a hash
   * of the inner ip */
	__u32 inhash = jhash_2words(in_src_ip, in_dst_ip, INIT_JHASH_SEED);
	*s_port = SPORT_MIN + (inhash % SPORT_BASE);

	if (*s_port < SPORT_MIN || *s_port > SPORT_MAX) {
		return 1;
	}

	if (metadata->net.nswitches == 0) {
		bpf_debug(
			"[Agent:%ld.0x%x] DROP (BUG): No transit switch to send packet to!\n",
			pkt->agent_ep_tunid, bpf_ntohl(pkt->agent_ep_ipv4));
		return 1;
	}

	__u32 sw_idx = inhash % metadata->net.nswitches;

	if (sw_idx > TRAN_MAX_NSWITCH - 1) {
		bpf_debug("[Agent:] DROP (BUG): Selected switch index [%d] "
			  "is greater than maximum number of switches [%d]!\n",
			  sw_idx, TRAN_MAX_NSWITCH);
		return 1;
	}

	*d_addr = metadata->net.switches_ips[sw_idx];
	return 0;
}

static __inline int trn_encapsulate(struct transit_packet *pkt,
				    struct agent_metadata_t *metadata,
				    __be64 tun_id, __u32 in_src_ip,
				    __u32 in_dst_ip)
{
	struct remote_endpoint_t *dst_r_ep;
	struct endpoint_t *r_ep;
	struct endpoint_key_t r_epkey;
	unsigned char *d_mac;
	__u16 s_port;
	__u32 d_addr;
	__u64 c_sum = 0;
	int old_size = pkt->data_end - pkt->data;

	/* First attempt to see if destination ep's host is cached */
	int map_idx = 0;
	void *ep_host_cache;
	struct endpoint_key_t dst_epkey;

	ep_host_cache = bpf_map_lookup_elem(&ep_host_cache_ref, &map_idx);
	if (!ep_host_cache) {
		bpf_debug("[Agent:%ld.0x%x] Failed to find ep_host_cache\n",
			  pkt->agent_ep_tunid, bpf_ntohl(pkt->agent_ep_ipv4));
		return XDP_DROP;
	}

	__builtin_memcpy(&dst_epkey.tunip[0], &tun_id, sizeof(tun_id));
	dst_epkey.tunip[2] = in_dst_ip;

	dst_r_ep = bpf_map_lookup_elem(ep_host_cache, &dst_epkey);

	if (dst_r_ep) {
		d_addr = dst_r_ep->ip;
		d_mac = dst_r_ep->mac;

		bpf_debug(
			"[Agent:%ld.0x%x] Host of 0x%x, found sending directly!\n",
			pkt->agent_ep_tunid, bpf_ntohl(pkt->agent_ep_ipv4),
			bpf_ntohl(in_dst_ip));

	} else if (!trn_select_transit_switch(pkt, metadata, tun_id, in_src_ip,
					      in_dst_ip, &s_port, &d_addr)) {
		/* Get the endpoint of the transit switch for outer dst mac */
		bpf_debug(
			"[Agent:%ld.0x%x] Sending dst 0x%x, to transit switch!\n",
			pkt->agent_ep_tunid, bpf_ntohl(pkt->agent_ep_ipv4),
			bpf_ntohl(in_dst_ip));
		r_epkey.tunip[0] = 0;
		r_epkey.tunip[1] = 0;
		r_epkey.tunip[2] = d_addr;
		r_ep = bpf_map_lookup_elem(&endpoints_map, &r_epkey);

		if (!r_ep) {
			bpf_debug("[Agent:%ld.0x%x] DROP (BUG): Missing mac "
				  " information for transit switches!\n",
				  pkt->agent_ep_tunid,
				  bpf_ntohl(pkt->agent_ep_ipv4));
			return XDP_DROP;
		}

		d_mac = r_ep->mac;

	} else {
		return XDP_DROP;
	}

	struct packet_metadata_key_t packet_metadata_key;
	struct packet_metadata_t *packet_metadata;
	packet_metadata_key.tunip[0] = metadata->epkey.tunip[0];
	packet_metadata_key.tunip[1] = metadata->epkey.tunip[1];
	packet_metadata_key.tunip[2] = metadata->epkey.tunip[2];

	int pod_label_value = 0;
	int namespace_label_value = 0;
	packet_metadata = bpf_map_lookup_elem(&packet_metadata_map, &packet_metadata_key);
	if(packet_metadata){
		pod_label_value = packet_metadata->pod_label_value;
		namespace_label_value = packet_metadata->namespace_label_value;
	}

	/* Readjust the packet size to fit the outer headers */
	int gnv_rts_opt_size = sizeof(*pkt->rts_opt);
	int gnv_scaled_ep_opt_size = sizeof(*pkt->scaled_ep_opt);
	int gnv_pod_label_value_opt_size = sizeof(*pkt->pod_label_value_opt);
	int gnv_namespace_label_value_opt_size = sizeof(*pkt->namespace_label_value_opt);

	int gnv_opt_size = gnv_rts_opt_size + gnv_scaled_ep_opt_size + gnv_pod_label_value_opt_size + gnv_namespace_label_value_opt_size;
	int gnv_hdr_size = sizeof(*pkt->geneve) + gnv_opt_size;
	int udp_hdr_size = sizeof(*pkt->udp);
	int ip_hdr_size = sizeof(*pkt->ip);
	int eth_hdr_size = sizeof(*pkt->eth);

	int outer_hdr_size =
		gnv_hdr_size + udp_hdr_size + ip_hdr_size + eth_hdr_size;
	int outer_ip_payload =
		gnv_hdr_size + udp_hdr_size + ip_hdr_size + old_size;
	int outer_udp_payload = gnv_hdr_size + udp_hdr_size + old_size;

	if (bpf_xdp_adjust_head(pkt->xdp, 0 - outer_hdr_size)) {
		bpf_debug(
			"[Agent:%ld.0x%x] DROP (BUG): Failure adjusting packet header!\n",
			pkt->agent_ep_tunid, bpf_ntohl(pkt->agent_ep_ipv4));
		return XDP_DROP;
	}

	pkt->data = (void *)(long)pkt->xdp->data;
	pkt->data_end = (void *)(long)pkt->xdp->data_end;

	pkt->eth = pkt->data;
	pkt->eth_off = eth_hdr_size;

	pkt->ip = (void *)pkt->eth + eth_hdr_size;
	pkt->udp = (void *)pkt->ip + ip_hdr_size;
	pkt->geneve = (void *)pkt->udp + udp_hdr_size;
	pkt->rts_opt = (void *)&pkt->geneve->options[0];
	pkt->scaled_ep_opt = (void *)pkt->rts_opt + sizeof(*pkt->rts_opt);
	pkt->pod_label_value_opt = (void *)pkt->scaled_ep_opt + sizeof(*pkt->scaled_ep_opt);
	pkt->namespace_label_value_opt = (void *)pkt->pod_label_value_opt + sizeof(*pkt->pod_label_value_opt);

	if (pkt->eth + 1 > pkt->data_end || pkt->ip + 1 > pkt->data_end ||
	    pkt->udp + 1 > pkt->data_end || pkt->geneve + 1 > pkt->data_end ||
	    pkt->rts_opt + 1 > pkt->data_end ||
	    pkt->scaled_ep_opt + 1 > pkt->data_end ||
	    pkt->pod_label_value_opt + 1 > pkt->data_end ||
	    pkt->namespace_label_value_opt + 1 > pkt->data_end) {
		bpf_debug("[Agent:%ld.0x%x] ABORTED: Bad offset [%d]\n",
			  pkt->agent_ep_tunid, bpf_ntohl(pkt->agent_ep_ipv4),
			  __LINE__);
		return XDP_ABORTED;
	}

	/* Populate the outer header fields */
	pkt->eth->h_proto = bpf_htons(ETH_P_IP);

	trn_set_dst_mac(pkt->data, d_mac);
	trn_set_src_mac(pkt->data, metadata->eth.mac);

	pkt->ip->version = 4;
	pkt->ip->ihl = ip_hdr_size >> 2;
	pkt->ip->frag_off = 0;
	pkt->ip->protocol = IPPROTO_UDP;
	pkt->ip->check = 0;
	pkt->ip->tos = 0;
	pkt->ip->tot_len = bpf_htons(outer_ip_payload);
	pkt->ip->daddr = d_addr;
	pkt->ip->saddr = metadata->eth.ip;
	pkt->ip->ttl = pkt->inner_ttl;

	c_sum = 0;
	trn_ipv4_csum_inline(pkt->ip, &c_sum);
	pkt->ip->check = c_sum;

	pkt->udp->source = bpf_htons(
		s_port); // TODO: a hash value based on inner IP packet
	pkt->udp->dest = GEN_DSTPORT;
	pkt->udp->len = bpf_htons(outer_udp_payload);

	__builtin_memset(pkt->geneve, 0, gnv_hdr_size);
	pkt->geneve->opt_len = gnv_opt_size / 4;
	pkt->geneve->ver = 0;
	pkt->geneve->rsvd1 = 0;
	pkt->geneve->rsvd2 = 0;
	pkt->geneve->oam = 0;
	pkt->geneve->critical = 0;
	pkt->geneve->proto_type = bpf_htons(ETH_P_TEB);
	trn_tunnel_id_to_vni(tun_id, pkt->geneve->vni);

	pkt->rts_opt->opt_class = TRN_GNV_OPT_CLASS;
	pkt->rts_opt->type = TRN_GNV_RTS_OPT_TYPE;
	pkt->rts_opt->length = sizeof(pkt->rts_opt->rts_data) / 4;
	pkt->rts_opt->rts_data.match_flow = 0;
	pkt->rts_opt->rts_data.host.ip = metadata->eth.ip;
	__builtin_memcpy(pkt->rts_opt->rts_data.host.mac, metadata->eth.mac,
			 6 * sizeof(unsigned char));

	pkt->scaled_ep_opt->opt_class = TRN_GNV_OPT_CLASS;
	pkt->scaled_ep_opt->type = 0;
	pkt->scaled_ep_opt->length = 0;
	__builtin_memset(&pkt->scaled_ep_opt->scaled_ep_data, 0,
			 sizeof(struct trn_gnv_scaled_ep_data));

	pkt->pod_label_value_opt->opt_class = TRN_GNV_OPT_CLASS;
	pkt->pod_label_value_opt->type = TRN_GNV_LABEL_VALUE_OPT_TYPE;
	pkt->pod_label_value_opt->length = sizeof(pkt->pod_label_value_opt->label_value_data) / 4;
	pkt->pod_label_value_opt->label_value_data.value = pod_label_value;

	pkt->namespace_label_value_opt->opt_class = TRN_GNV_OPT_CLASS;
	pkt->namespace_label_value_opt->type = TRN_GNV_LABEL_VALUE_OPT_TYPE;
	pkt->namespace_label_value_opt->length = sizeof(pkt->namespace_label_value_opt->label_value_data) / 4;;
	pkt->namespace_label_value_opt->label_value_data.value = namespace_label_value;

	/* If the source and dest address of the tunneled packet is the
	 * same, then this host is also a transit switch. Just invoke the
	 * transit XDP program by a tail call;
	 */

	if (pkt->ip->saddr == pkt->ip->daddr) {
		bpf_debug(
			"[Agent:%ld.0x%x] TAILCALL: transit switch on same host. Tunnel to dst=[0x%x].\n",
			pkt->agent_ep_tunid, bpf_ntohl(pkt->agent_ep_ipv4),
			bpf_htonl(pkt->ip->daddr));

		__u32 key = XDP_TRANSIT;
		bpf_tail_call(pkt->xdp, &jmp_table, key);
	}

	/* Send the packet on the egress of the tunneling interface */
	bpf_debug("[Agent:%ld.0x%x] REDIRECT: Tunnel to dst=[x%0x].\n",
		  pkt->agent_ep_tunid, bpf_ntohl(pkt->agent_ep_ipv4),
		  bpf_htonl(pkt->ip->daddr));

	return bpf_redirect_map(&interfaces_map, 0, 0);
}

static __inline int trn_redirect(struct transit_packet *pkt, __u32 inner_src_ip,
				 __u32 inner_dst_ip)
{
	struct agent_metadata_t *md = pkt->agent_md;
	__be64 tunnel_id = pkt->agent_ep_tunid;

	return trn_encapsulate(pkt, md, tunnel_id, inner_src_ip, inner_dst_ip);
}

static __inline int trn_process_inner_ip(struct transit_packet *pkt)
{
	pkt->inner_ip = (void *)pkt->inner_eth + pkt->inner_eth_off;

	if (pkt->inner_ip + 1 > pkt->data_end) {
		bpf_debug("[Agent:%ld.0x%x] ABORTED: Bad offset [%d]\n",
			  pkt->agent_ep_tunid, bpf_ntohl(pkt->agent_ep_ipv4),
			  __LINE__);
		return XDP_ABORTED;
	}

	int map_idx = 0;
	void *fwd_flow_mod_cache =
		bpf_map_lookup_elem(&fwd_flow_mod_cache_ref, &map_idx);
	if (!fwd_flow_mod_cache) {
		bpf_debug(
			"[Agent:%ld.0x%x] Failed to find fwd_flow_mod_cache\n",
			pkt->agent_ep_tunid, bpf_ntohl(pkt->agent_ep_ipv4));
		return XDP_DROP;
	}

	pkt->inner_ipv4_tuple.saddr = pkt->inner_ip->saddr;
	pkt->inner_ipv4_tuple.daddr = pkt->inner_ip->daddr;
	pkt->inner_ipv4_tuple.protocol = pkt->inner_ip->protocol;
	pkt->inner_ipv4_tuple.sport = 0;
	pkt->inner_ipv4_tuple.dport = 0;

	if (pkt->inner_ipv4_tuple.protocol == IPPROTO_TCP) {
		pkt->inner_tcp = (void *)pkt->inner_ip + sizeof(*pkt->inner_ip);

		if (pkt->inner_tcp + 1 > pkt->data_end) {
			bpf_debug("[Agent:%ld.0x%x] ABORTED: Bad offset [%d]\n",
				  pkt->agent_ep_tunid,
				  bpf_ntohl(pkt->agent_ep_ipv4), __LINE__);
			return XDP_ABORTED;
		}

		pkt->inner_ipv4_tuple.sport = pkt->inner_tcp->source;
		pkt->inner_ipv4_tuple.dport = pkt->inner_tcp->dest;
	}

	if (pkt->inner_ipv4_tuple.protocol == IPPROTO_UDP) {
		pkt->inner_udp = (void *)pkt->inner_ip + sizeof(*pkt->inner_ip);

		if (pkt->inner_udp + 1 > pkt->data_end) {
			bpf_debug("[Agent:%ld.0x%x] ABORTED: Bad offset [%d]\n",
				  pkt->agent_ep_tunid,
				  bpf_ntohl(pkt->agent_ep_ipv4), __LINE__);
			return XDP_ABORTED;
		}

		pkt->inner_ipv4_tuple.sport = pkt->inner_udp->source;
		pkt->inner_ipv4_tuple.dport = pkt->inner_udp->dest;
	}

	if (pkt->inner_ipv4_tuple.protocol == IPPROTO_TCP || pkt->inner_ipv4_tuple.protocol == IPPROTO_UDP) {
		__u8 *tracked_state = get_originated_conn_state(&conn_track_cache, pkt->agent_ep_tunid, &pkt->inner_ipv4_tuple);
		// todo: only check for bi-directional connections
		if (NULL != tracked_state){
			// reply packet is usually allowed, unless re-eval blocks it
			if (0 != egress_reply_packet_check(pkt->agent_ep_tunid, &pkt->inner_ipv4_tuple, *tracked_state))
			{
				bpf_debug("[Agent:%ld.0x%x] ABORTED: packet to 0x%x egress policy denied, reply of a denied conn\n",
					pkt->agent_ep_tunid,
					bpf_ntohl(pkt->agent_ep_ipv4),
					bpf_ntohl(pkt->inner_ip->daddr));
				return XDP_ABORTED;
			}
		} else {
			// originated-directional packet subjects to policy check, if required so
			if (0 != egress_policy_check(pkt->agent_ep_tunid, &pkt->inner_ipv4_tuple)) {
				bpf_debug("[Agent:%ld.0x%x] ABORTED: packet to 0x%x egress policy denied\n",
					pkt->agent_ep_tunid,
					bpf_ntohl(pkt->agent_ep_ipv4),
					bpf_ntohl(pkt->inner_ip->daddr));
				__u8 conn_denied = (pkt->inner_ipv4_tuple.protocol == IPPROTO_UDP) ? FLAG_REEVAL | TRFFIC_DENIED : TRFFIC_DENIED;
				conntrack_set_conn_state(&conn_track_cache, pkt->agent_ep_tunid, &pkt->inner_ipv4_tuple, conn_denied);
				return XDP_ABORTED;
			}

			// todo: consider to handle error in case it happens
			__u8 conn_allowed = (pkt->inner_ipv4_tuple.protocol == IPPROTO_UDP) ? FLAG_REEVAL : 0;
			conntrack_set_conn_state(&conn_track_cache, pkt->agent_ep_tunid, &pkt->inner_ipv4_tuple, conn_allowed);
		}
	}

	/* Check if we need to apply a forward flow update */

	struct ipv4_tuple_t in_tuple;
	struct scaled_endpoint_remote_t *out_tuple;
	__builtin_memcpy(&in_tuple, &pkt->inner_ipv4_tuple,
			 sizeof(struct ipv4_tuple_t));

	out_tuple = bpf_map_lookup_elem(fwd_flow_mod_cache, &in_tuple);

	if (out_tuple) {
		/* Modify the inner packet accordingly */
		trn_set_src_dst_port(pkt, out_tuple->sport, out_tuple->dport);
		trn_set_src_dst_inner_ip_csum(pkt, out_tuple->saddr,
					      out_tuple->daddr);
		trn_set_dst_mac(pkt->inner_eth, out_tuple->h_dest);
	} else {
		bpf_debug("[Agent:%ld.0x%x] No dest IP address found! [%d]\n",
			  pkt->agent_ep_tunid, bpf_ntohl(in_tuple.daddr),
			  __LINE__);
	}

	return trn_redirect(pkt, pkt->inner_ip->saddr, pkt->inner_ip->daddr);
}

static __inline int trn_process_arp(struct transit_packet *pkt)
{
	unsigned char *sha;
	unsigned char *tha = NULL;
	__u32 *sip;
	__u32 *tip;

	pkt->inner_arp = (void *)pkt->inner_eth + sizeof(*pkt->inner_eth);

	if (pkt->inner_arp + 1 > pkt->data_end) {
		bpf_debug("[Agent:%ld.0x%x] ABORTED: Bad offset [%d]\n",
			  pkt->agent_ep_tunid, bpf_ntohl(pkt->agent_ep_ipv4),
			  __LINE__);
		return XDP_ABORTED;
	}

	if (pkt->inner_arp->ar_pro != bpf_htons(ETH_P_IP) ||
	    pkt->inner_arp->ar_hrd != bpf_htons(ARPHRD_ETHER)) {
		bpf_debug(
			"[Agent:%ld.0x%x] DROP: unsupported ARP protocol or hardware!\n",
			pkt->agent_ep_tunid, bpf_ntohl(pkt->agent_ep_ipv4));
		return XDP_DROP;
	}

	if (pkt->inner_arp->ar_op != bpf_htons(ARPOP_REPLY) &&
	    pkt->inner_arp->ar_op != bpf_htons(ARPOP_REQUEST)) {
		bpf_debug(
			"[Agent:%ld.0x%x] DROP: unsupported ARP OP code [0x%x]\n",
			pkt->agent_ep_tunid, bpf_ntohl(pkt->agent_ep_ipv4),
			pkt->inner_arp->ar_op);
		return XDP_DROP;
	}

	if ((unsigned char *)(pkt->inner_arp + 1) > pkt->data_end) {
		bpf_debug("[Agent:%ld.0x%x] ABORTED: Bad offset [%d]\n",
			  pkt->agent_ep_tunid, bpf_ntohl(pkt->agent_ep_ipv4),
			  __LINE__);
		return XDP_ABORTED;
	}

	sha = (unsigned char *)(pkt->inner_arp + 1);

	if (sha + ETH_ALEN > pkt->data_end) {
		bpf_debug("[Agent:%ld.0x%x] ABORTED: Bad offset [%d]\n",
			  pkt->agent_ep_tunid, bpf_ntohl(pkt->agent_ep_ipv4),
			  __LINE__);
		return XDP_ABORTED;
	}

	sip = (__u32 *)(sha + ETH_ALEN);

	if (sip + 1 > pkt->data_end) {
		bpf_debug("[Agent:%ld.0x%x] ABORTED: Bad offset [%d]\n",
			  pkt->agent_ep_tunid, bpf_ntohl(pkt->agent_ep_ipv4),
			  __LINE__);
		return XDP_ABORTED;
	}

	tha = (unsigned char *)sip + sizeof(__u32);

	if (tha + ETH_ALEN > pkt->data_end) {
		bpf_debug("[Agent:%ld.0x%x] ABORTED: Bad offset [%d]\n",
			  pkt->agent_ep_tunid, bpf_ntohl(pkt->agent_ep_ipv4),
			  __LINE__);
		return XDP_ABORTED;
	}

	tip = (__u32 *)(tha + ETH_ALEN);

	if ((void *)tip + sizeof(__u32) > pkt->data_end) {
		bpf_debug("[Agent:%ld.0x%x] ABORTED: Bad offset [%d]\n",
			  pkt->agent_ep_tunid, bpf_ntohl(pkt->agent_ep_ipv4),
			  __LINE__);
		return XDP_ABORTED;
	}

	return trn_redirect(pkt, *sip, *tip);
}

static __inline int trn_process_inner_eth(struct transit_packet *pkt)
{
	pkt->inner_eth = (void *)pkt->data;
	pkt->inner_eth_off = sizeof(*pkt->inner_eth);

	if (pkt->inner_eth + 1 > pkt->data_end) {
		bpf_debug("[Agent:%ld.0x%x] ABORTED: Bad offset [%d]\n",
			  pkt->agent_ep_tunid, bpf_ntohl(pkt->agent_ep_ipv4),
			  __LINE__);
		return XDP_ABORTED;
	}

	/* ARP */
	if (pkt->inner_eth->h_proto == bpf_htons(ETH_P_ARP)) {
		bpf_debug("[Agent:%ld.0x%x] Processing ARP\n",
			  pkt->agent_ep_tunid, bpf_ntohl(pkt->agent_ep_ipv4));
		return ({ trn_process_arp(pkt); });
	}

	if (pkt->inner_eth->h_proto != bpf_htons(ETH_P_IP)) {
		bpf_debug("[Agent:%ld.0x%x] DROP: unsupported"
			  " inner packet type [0x%x]\n",
			  pkt->agent_ep_tunid, bpf_ntohl(pkt->agent_ep_ipv4),
			  bpf_ntohs(pkt->inner_eth->h_proto));
		return XDP_DROP;
	}

	bpf_debug("[Agent:%ld.0x%x] Processing IP\n", pkt->agent_ep_tunid,
		  bpf_ntohl(pkt->agent_ep_ipv4));

	return trn_process_inner_ip(pkt);
}

SEC("agent")
int _agent(struct xdp_md *ctx)
{
	struct transit_packet pkt;
	pkt.data = (void *)(long)ctx->data;
	pkt.data_end = (void *)(long)ctx->data_end;
	pkt.xdp = ctx;
	pkt.agent_ep_ipv4 = -1;
	pkt.agent_ep_tunid = -1;
	pkt.inner_ttl = TRN_DEFAULT_TTL; // Default in case of non inner IP
	/* Get the metadata of the agent */
	int key = 0;
	pkt.agent_md = bpf_map_lookup_elem(&agentmetadata_map, &key);
	if (!pkt.agent_md) {
		bpf_debug("DROP (BUG): Agent metadata is missing\n");
		return XDP_DROP;
	}

	__builtin_memcpy(&pkt.agent_ep_tunid, &pkt.agent_md->epkey.tunip[0],
			 sizeof(__be64));
	pkt.agent_ep_ipv4 = pkt.agent_md->epkey.tunip[2];
	bpf_debug("[Agent:%ld.0x%x]\n", pkt.agent_ep_tunid,
		  bpf_ntohl(pkt.agent_ep_ipv4));

	int action = trn_process_inner_eth(&pkt);

	if (action == XDP_PASS)
		return xdpcap_exit(ctx, &xdpcap_hook, XDP_PASS);

	if (action == XDP_DROP)
		return xdpcap_exit(ctx, &xdpcap_hook, XDP_DROP);

	if (action == XDP_TX)
		return xdpcap_exit(ctx, &xdpcap_hook, XDP_TX);

	if (action == XDP_ABORTED)
		return xdpcap_exit(ctx, &xdpcap_hook, XDP_ABORTED);

	if (action == XDP_REDIRECT)
		return xdpcap_exit(ctx, &xdpcap_hook, XDP_REDIRECT);

	return xdpcap_exit(ctx, &xdpcap_hook, XDP_PASS);
}

char _license[] SEC("license") = "GPL";
