// SPDX-License-Identifier: GPL-2.0-or-later
/**
 * @file transit_kern.h
 * @author Sherif Abdelwahab (@zasherif)
 *
 * @brief Helper functions, macros and data structures.
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
#pragma once

#include <linux/bpf.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <linux/tcp.h>
#include <stddef.h>

#include "extern/bpf_helpers.h"
#include "extern/jhash.h"

#include "trn_datamodel.h"

#define PRIu8 "hu"
#define PRId8 "hd"
#define PRIx8 "hx"
#define PRIu16 "hu"
#define PRId16 "hd"
#define PRIx16 "hx"
#define PRIu32 "u"
#define PRId32 "d"
#define PRIx32 "x"
#define PRIu64 "llu" // or possibly "lu"
#define PRId64 "lld" // or possibly "ld"
#define PRIx64 "llx" // or possibly "lx"

#define TRN_DEFAULT_TTL 64
#define GEN_DSTPORT 0xc117
#define INIT_JHASH_SEED 0xdeadbeef

#define TRN_GNV_OPT_CLASS 0x0111
#define TRN_GNV_RTS_OPT_TYPE 0x48
#define TRN_GNV_SCALED_EP_OPT_TYPE 0x49
#define TRN_GNV_LABEL_VALUE_OPT_TYPE 0x50

/* Scaled endpoint messages type */
#define TRN_SCALED_EP_MODIFY 0x4d // (M: Modify)

#ifndef __inline
#define __inline inline __attribute__((always_inline))
#endif

struct trn_gnv_scaled_ep_data {
	__u8 msg_type;
	struct scaled_endpoint_remote_t target;
} __attribute__((packed, aligned(4)));

struct trn_gnv_scaled_ep_opt {
	__be16 opt_class;
	__u8 type;
	__u8 length : 5;
	__u8 r3 : 1;
	__u8 r2 : 1;
	__u8 r1 : 1;
	/* opt data */
	struct trn_gnv_scaled_ep_data scaled_ep_data;
} __attribute__((packed, aligned(4)));

struct trn_gnv_label_value_data {
	__u32 value;
} __attribute__((packed, aligned(4)));

struct trn_gnv_label_value_opt {
	__be16 opt_class;
	__u8 type;
	__u8 length : 5;
	__u8 r3 : 1;
	__u8 r2 : 1;
	__u8 r1 : 1;
	/* opt data */
	struct trn_gnv_label_value_data label_value_data;
} __attribute__((packed, aligned(4)));

struct trn_gnv_rts_data {
	__u8 match_flow : 1;
	struct remote_endpoint_t host;
} __attribute__((packed, aligned(4)));

struct trn_gnv_rts_opt {
	__be16 opt_class;
	__u8 type;
	__u8 length : 5;
	__u8 r3 : 1;
	__u8 r2 : 1;
	__u8 r1 : 1;
	/* opt data */
	struct trn_gnv_rts_data rts_data;
} __attribute__((packed, aligned(4)));

struct geneve_opt {
	__be16 opt_class;
	__u8 type;
	__u8 length : 5;
	__u8 r3 : 1;
	__u8 r2 : 1;
	__u8 r1 : 1;
	__u8 opt_data[];
};

struct genevehdr {
	/* Big endian! */
	__u8 opt_len : 6;
	__u8 ver : 2;
	__u8 rsvd1 : 6;
	__u8 critical : 1;
	__u8 oam : 1;
	__be16 proto_type;
	__u8 vni[3];
	__u8 rsvd2;
	struct geneve_opt options[];
};

struct vlan_hdr {
	__be16 h_vlan_TCI;
	__be16 h_vlan_encapsulated_proto;
};

struct transit_packet {
	void *data;
	void *data_end;

	/* interface index */
	int itf_idx;
	__u32 itf_ipv4;

	/* xdp*/
	struct xdp_md *xdp;

	/* Ether */
	struct ethhdr *eth;
	__u64 eth_off;

	/* IP */
	struct iphdr *ip;

	/* UDP */
	struct udphdr *udp;

	/* Geneve */
	struct genevehdr *geneve;
	struct trn_gnv_rts_opt *rts_opt;
	struct trn_gnv_scaled_ep_opt *scaled_ep_opt;
	struct trn_gnv_label_value_opt *pod_label_value_opt;
	struct trn_gnv_label_value_opt *namespace_label_value_opt;
	int gnv_hdr_len;
	int gnv_opt_len;

	/* Inner ethernet */
	struct ethhdr *inner_eth;
	__u64 inner_eth_off;

	/* Inner arp */
	// struct _arp_hdr *inner_arp;
	struct arphdr *inner_arp;

	/* Inner IP */
	struct iphdr *inner_ip;
	__u8 inner_ttl;

	/* Inner udp */
	struct udphdr *inner_udp;

	/* Inner tcp */
	struct tcphdr *inner_tcp;

	/* inner ipv4 tuple */
	struct ipv4_tuple_t inner_ipv4_tuple;

	/* Agent metadata */
	struct agent_metadata_t *agent_md;
	__be64 agent_ep_tunid;
	__u32 agent_ep_ipv4;

	// TODO: Inner UDP or TCP
} __attribute__((packed));

__ALWAYS_INLINE__
static inline __u32 trn_get_inner_packet_hash(struct transit_packet *pkt)
{
	// TODO: Just the source IP for now, change to the 4-tuples
	return jhash_2words(pkt->inner_ip->saddr, 0, INIT_JHASH_SEED);
}

__ALWAYS_INLINE__
static __be64 trn_vni_to_tunnel_id(const __u8 *vni)
{
	/* Big endian! */
	return (vni[0] << 16) | (vni[1] << 8) | vni[2];
}

__ALWAYS_INLINE__
static void trn_tunnel_id_to_vni(__be64 tun_id, __u8 *vni)
{
	/* Big endian! */
	vni[0] = (__u8)(tun_id >> 16);
	vni[1] = (__u8)(tun_id >> 8);
	vni[2] = (__u8)tun_id;
}

__ALWAYS_INLINE__
static inline __u16 trn_csum_fold_helper(__u64 csum)
{
	int i;
#pragma unroll
	for (i = 0; i < 4; i++) {
		if (csum >> 16)
			csum = (csum & 0xffff) + (csum >> 16);
	}
	return ~csum;
}

__ALWAYS_INLINE__
static inline void trn_ipv4_csum_inline(void *iph, __u64 *csum)
{
	__u16 *next_iph_u16 = (__u16 *)iph;
#pragma clang loop unroll(full)
	for (int i = 0; i<sizeof(struct iphdr)>> 1; i++) {
		*csum += *next_iph_u16++;
	}
	*csum = trn_csum_fold_helper(*csum);
}

__ALWAYS_INLINE__
static inline void trn_update_l4_csum(__u64 *csum, __be32 old_addr,
				      __be32 new_addr)
{
	*csum = (~*csum & 0xffff) + ~old_addr + new_addr;
	*csum = trn_csum_fold_helper(*csum);
}

__ALWAYS_INLINE__
static inline void trn_update_l4_csum_port(__u64 *csum, __be16 old_port,
					   __be16 new_port)
{
	*csum = (~*csum & 0xffff) + ~old_port + new_port;
	*csum = trn_csum_fold_helper(*csum);
}

__ALWAYS_INLINE__
static inline void trn_set_arp_ha(void *ha, unsigned char *mac)
{
	unsigned short *p = ha;
	unsigned short *dst = (unsigned short *)mac;

	p[0] = dst[0];
	p[1] = dst[1];
	p[2] = dst[2];
}

__ALWAYS_INLINE__
static inline void trn_set_dst_mac(void *data, unsigned char *dst_mac)
{
	unsigned short *p = data;
	unsigned short *dst = (unsigned short *)dst_mac;

	p[0] = dst[0];
	p[1] = dst[1];
	p[2] = dst[2];
}

__ALWAYS_INLINE__
static inline void trn_set_src_mac(void *data, unsigned char *src_mac)
{
	unsigned short *p = data;
	unsigned short *src = (unsigned short *)src_mac;

	p[3] = src[0];
	p[4] = src[1];
	p[5] = src[2];
}

__ALWAYS_INLINE__
static inline void trn_swap_src_dst_mac(void *data)
{
	unsigned short *p = data;
	unsigned short dst[3];

	dst[0] = p[0];
	dst[1] = p[1];
	dst[2] = p[2];
	p[0] = p[3];
	p[1] = p[4];
	p[2] = p[5];
	p[3] = dst[0];
	p[4] = dst[1];
	p[5] = dst[2];
}

__ALWAYS_INLINE__
static inline void trn_set_src_ip(void *data, void *data_end, __u32 saddr)
{
	int off = offsetof(struct iphdr, saddr);
	__u32 *addr = data + off;
	if ((void *)addr > data_end)
		return;

	*addr = saddr;
}

__ALWAYS_INLINE__
static inline void trn_set_dst_ip(void *data, void *data_end, __u32 daddr)
{
	int off = offsetof(struct iphdr, daddr);
	__u32 *addr = data + off;
	if ((void *)addr > data_end)
		return;

	*addr = daddr;
}

__ALWAYS_INLINE__
static inline void trn_swap_src_dst_ip(struct iphdr *ip, void *data_end)
{
	__u32 tmp = ip->saddr;
	trn_set_src_ip(ip, data_end, ip->daddr);
	trn_set_dst_ip(ip, data_end, tmp);
}

__ALWAYS_INLINE__
static inline void trn_set_sport_udp(void *data, void *data_end, __u16 sport)
{
	int off = offsetof(struct udphdr, source);
	__u16 *addr = data + off;
	if ((void *)addr + sizeof(__be16) > data_end)
		return;

	*addr = sport;
}

__ALWAYS_INLINE__
static inline void trn_set_dport_udp(void *data, void *data_end, __u16 dport)
{
	int off = offsetof(struct udphdr, dest);
	__u16 *addr = data + off;
	if ((void *)addr + sizeof(__be16) > data_end)
		return;

	*addr = dport;
}

__ALWAYS_INLINE__
static inline void trn_swap_sport_dport_udp(struct udphdr *udp, void *data_end)
{
	__u16 tmp = udp->source;
	trn_set_sport_udp(udp, data_end, udp->dest);
	trn_set_dport_udp(udp, data_end, tmp);
}

__ALWAYS_INLINE__
static inline void trn_set_src_dst_ip_csum(struct transit_packet *pkt,
					   __u32 saddr, __u32 daddr)
{
	/* Since the packet destination is being rewritten we also
	decrement the TTL */
	pkt->ip->ttl--;

	__u64 csum = 0;
	trn_set_src_ip(pkt->ip, pkt->data_end, saddr);
	trn_set_dst_ip(pkt->ip, pkt->data_end, daddr);
	csum = 0;
	pkt->ip->check = 0;
	trn_ipv4_csum_inline(pkt->ip, &csum);
	pkt->ip->check = csum;

	bpf_debug("Modified IP Address, src: 0x%x, dst: 0x%x, csum: 0x%x\n",
		  pkt->ip->saddr, pkt->ip->daddr, pkt->ip->check);
}

__ALWAYS_INLINE__
static inline void trn_inner_l4_csum_update(struct transit_packet *pkt,
					    __u32 old_addr, __u32 new_addr)
{
	if (new_addr == old_addr)
		return;

	if (pkt->inner_ip->protocol == IPPROTO_UDP) {
		if (!pkt->inner_udp) {
			return;
		}

		if (pkt->inner_udp + 1 > pkt->data_end) {
			return;
		}

		__u64 cs = pkt->inner_udp->check;
		trn_update_l4_csum(&cs, old_addr, new_addr);
		pkt->inner_udp->check = cs;
	}

	if (pkt->inner_ip->protocol == IPPROTO_TCP) {
		if (!pkt->inner_tcp) {
			return;
		}

		if (pkt->inner_tcp + 1 > pkt->data_end) {
			return;
		}

		__u64 cs = pkt->inner_tcp->check;
		trn_update_l4_csum(&cs, old_addr, new_addr);
		pkt->inner_tcp->check = cs;
	}
}

__ALWAYS_INLINE__
static inline void trn_set_src_dst_inner_ip_csum(struct transit_packet *pkt,
						 __u32 saddr, __u32 daddr)
{
	if (pkt->inner_ip + 1 > pkt->data_end) {
		return;
	}

	__u32 old_saddr = pkt->inner_ip->saddr;
	__u32 old_daddr = pkt->inner_ip->daddr;

	__u64 csum = 0;
	trn_set_src_ip(pkt->inner_ip, pkt->data_end, saddr);
	trn_inner_l4_csum_update(pkt, old_saddr, saddr);

	trn_set_dst_ip(pkt->inner_ip, pkt->data_end, daddr);
	trn_inner_l4_csum_update(pkt, old_daddr, daddr);

	csum = 0;
	pkt->inner_ip->check = 0;
	trn_ipv4_csum_inline(pkt->inner_ip, &csum);
	pkt->inner_ip->check = csum;

	bpf_debug(
		"Modified Inner IP Address, src: 0x%x, dst: 0x%x, csum: 0x%x\n",
		pkt->inner_ip->saddr, pkt->inner_ip->daddr,
		pkt->inner_ip->check);
}

__ALWAYS_INLINE__
static inline void trn_set_src_dst_port(struct transit_packet *pkt, __u16 sport,
					__u16 dport)
{
	if (pkt->inner_ipv4_tuple.protocol == IPPROTO_TCP) {
		if (pkt->inner_tcp + 1 > pkt->data_end)
			return;
		__u16 old_dport = pkt->inner_tcp->dest;
		__u16 old_sport = pkt->inner_tcp->source;
		pkt->inner_tcp->source = sport;
		pkt->inner_tcp->dest = dport;
		// Compute csum
		if (old_dport != dport) {
			__u64 cs = pkt->inner_tcp->check;
			trn_update_l4_csum_port(&cs, old_dport, dport);
			pkt->inner_tcp->check = cs - bpf_htons(256);
		}
		if (old_sport != sport) {
			__u64 cs = pkt->inner_tcp->check;
			trn_update_l4_csum_port(&cs, old_sport, sport);
			pkt->inner_tcp->check = cs - bpf_htons(256);
		}
		bpf_debug("Modified Inner TCP Ports src: %u, dest: %u, csum: 0x%x\n",
			  bpf_ntohs(pkt->inner_tcp->source),
			  bpf_ntohs(pkt->inner_tcp->dest), pkt->inner_tcp->check);
	} else if (pkt->inner_ipv4_tuple.protocol == IPPROTO_UDP) {
		if (pkt->inner_udp + 1 > pkt->data_end)
			return;
		__u16 old_dport = pkt->inner_udp->dest;
		__u16 old_sport = pkt->inner_udp->source;
		pkt->inner_udp->source = sport;
		pkt->inner_udp->dest = dport;

		// Compute csum
		if (old_dport != dport) {
			__u64 cs = pkt->inner_udp->check;
			trn_update_l4_csum_port(&cs, old_dport, dport);
			pkt->inner_udp->check = cs - bpf_htons(256);
		}
		if (old_sport != sport) {
			__u64 cs = pkt->inner_udp->check;
			trn_update_l4_csum_port(&cs, old_sport, sport);
			pkt->inner_udp->check = cs - bpf_htons(256);
		}
		bpf_debug("Modified Inner UDP Ports src: %u, dest: %u, csum: 0x%x\n",
			  bpf_ntohs(pkt->inner_udp->source),
			  bpf_ntohs(pkt->inner_udp->dest), pkt->inner_udp->check);
	} else {
		return;
	}
}

__ALWAYS_INLINE__
static inline void trn_reverse_ipv4_tuple(struct ipv4_tuple_t *tuple)
{
	__u32 tmp_addr = tuple->saddr;
	__u16 tmp_port = tuple->sport;

	tuple->saddr = tuple->daddr;
	tuple->sport = tuple->dport;

	tuple->daddr = tmp_addr;
	tuple->dport = tmp_port;
}

__ALWAYS_INLINE__
static inline void trn_reset_rts_opt(struct transit_packet *pkt)

{
	pkt->rts_opt->type = 0;
	pkt->rts_opt->length = 0;
	__builtin_memset(&pkt->rts_opt->rts_data, 0,
			 sizeof(struct trn_gnv_rts_data));
}

__ALWAYS_INLINE__
static inline void trn_reset_scaled_ep_opt(struct transit_packet *pkt)

{
	pkt->scaled_ep_opt->type = 0;
	pkt->scaled_ep_opt->length = 0;
	__builtin_memset(&pkt->scaled_ep_opt->scaled_ep_data, 0,
			 sizeof(struct trn_gnv_scaled_ep_data));
}
