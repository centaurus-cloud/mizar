// SPDX-License-Identifier: GPL-2.0-or-later
/**
 * @file conntrack_common.h
 * @author Hongwei Chen (@hong.chen@futurewei.com)
 *
 * @brief Defines common code used in conntrack feature
 *
 * @copyright Copyright (c) 2020 The Authors.
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

#include "trn_kern.h"

__ALWAYS_INLINE__
static inline int conntrack_set_conn_state(void *conntracks, __u64 tunnel_id, const struct ipv4_tuple_t *tuple, __u8 state)
{
	struct ipv4_ct_tuple_t conn = {
		.vpc.tunnel_id = tunnel_id,
		.tuple = *tuple,
	};
	return bpf_map_update_elem(conntracks, &conn, &state, 0);
}

__ALWAYS_INLINE__
static inline __u8* get_originated_conn_state(void *conntracks, __u64 tunnel_id, const struct ipv4_tuple_t *reply_tuple)
{
	struct ipv4_ct_tuple_t rev_conn = {
		.vpc.tunnel_id = tunnel_id,
		.tuple = {
			.protocol = reply_tuple->protocol,
			.saddr = reply_tuple->daddr,
			.daddr = reply_tuple->saddr,
			.sport = reply_tuple->dport,
			.dport = reply_tuple->sport,
		},
	};
	return bpf_map_lookup_elem(conntracks, &rev_conn);
}

/*
   check if ingress network policy should be enforced to specific destination
   return value:
     0 (false) :    no ingress policy
     non-0 (true) : need to enforce ingress policy check
*/
__ALWAYS_INLINE__
static inline int is_ingress_enforced(__u64 tunnel_id, __be32 ip_addr)
{
	struct vsip_enforce_t vsip = { .tunnel_id = tunnel_id,
				       .local_ip = ip_addr };
	__u8 *v = bpf_map_lookup_elem(&ing_vsip_enforce_map, &vsip);
	return v && *v;
}

/*
   enforce_ingress_policy enforces egress network policy with the (incoming) packet
   return value:
     0: ingress policy allows this packet; no error
    -1: ingress policy denies this packet; ingress policy denial error
*/
__ALWAYS_INLINE__
static inline int enforce_ingress_policy(__u64 tunnel_id, const struct ipv4_tuple_t *ipv4_tuple)
{
	const __u32 full_vsip_cidr_prefix = (__u32)(sizeof(struct vsip_cidr_t) - sizeof(__u32)) * 8;

	struct vsip_ppo_t vsip_ppo = {
		.tunnel_id = tunnel_id,
		.local_ip = ipv4_tuple->daddr,
		.proto = 0, 	// L3
		.port = 0, 	// L3
	};
	__u64 *policies_l3 = bpf_map_lookup_elem(&ing_vsip_ppo_map, &vsip_ppo);
	__u64 policies_ppo = (policies_l3) ? *policies_l3 : 0;

	vsip_ppo.proto = ipv4_tuple->protocol;	// L4
	vsip_ppo.port = ipv4_tuple->dport;	// L4
	__u64 *policies_l4 = bpf_map_lookup_elem(&ing_vsip_ppo_map, &vsip_ppo);
	if (policies_l4) policies_ppo |= *policies_l4;

	if (0 == policies_ppo) return -1;

	struct vsip_cidr_t vsip_cidr = {
		.prefixlen = full_vsip_cidr_prefix,
		.tunnel_id = tunnel_id,
		.local_ip = ipv4_tuple->daddr,
		.remote_ip = ipv4_tuple->saddr,
	};
	__u64 *policies_sip_prim = bpf_map_lookup_elem(&ing_vsip_prim_map, &vsip_cidr);
	if (policies_sip_prim && (policies_ppo & *policies_sip_prim))
		return 0;

	__u64 *policies_sip_supp = bpf_map_lookup_elem(&ing_vsip_supp_map, &vsip_cidr);
	__u64 *policies_sip_except = bpf_map_lookup_elem(&ing_vsip_except_map, &vsip_cidr);
	if (policies_sip_supp) {
		__u64 excepts = (policies_sip_except) ? *policies_sip_except : 0;
		if ((*policies_sip_supp & ~excepts) & policies_ppo)
			return 0;
	}

	return -1;
}

/*
   ingress_policy_check checks whether an incoming packet of the specific tunnel id and connection tuple
     should be allowed or denied
   return value:
     0: allows this packet; no error (by either open policy or an allowing ingress policy)
    -1: denies this packet; ingress policy denial error
*/
__ALWAYS_INLINE__
static inline int ingress_policy_check(__u64 tunnel_id, const struct ipv4_tuple_t *ipv4_tuple)
{
	if (!is_ingress_enforced(tunnel_id, ipv4_tuple->daddr))
		return 0;

	return enforce_ingress_policy(tunnel_id, ipv4_tuple);
}

/*
   check if egress network policy should be enforced
   return value:
     0 (false) :    no egress policy
     non-0 (true) : need to enforce egress policy check
*/
__ALWAYS_INLINE__
static inline int is_egress_enforced(__u64 tunnel_id, __be32 ip_addr)
{
	// todo: use agent metadata applicable field in lieu of packet metadata
	struct vsip_enforce_t vsip = {.tunnel_id = tunnel_id, .local_ip = ip_addr};
	__u8 *v = bpf_map_lookup_elem(&eg_vsip_enforce_map, &vsip);
	return v && *v;
}

/*
   enforce_egress_policy enforces egress network policy with the (outgoing) packet
   return value:
     0: egress policy allows this packet; no error
    -1: egress policy denies this packet; egress policy denial error
*/
__ALWAYS_INLINE__
static inline int enforce_egress_policy(__u64 tunnel_id, const struct ipv4_tuple_t *ipv4_tuple)
{
	const __u32 full_vsip_cidr_prefix = (__u32)(sizeof(struct vsip_cidr_t) - sizeof(__u32)) * 8;

	struct vsip_ppo_t vsip_ppo = {
		.tunnel_id = tunnel_id,
		.local_ip = ipv4_tuple->saddr,
		.proto = 0,	// L3
		.port = 0,	// L3
	};
	__u64 *policies_l3 = bpf_map_lookup_elem(&eg_vsip_ppo_map, &vsip_ppo);
	__u64 policies_ppo = (policies_l3) ? *policies_l3 : 0;

	vsip_ppo.proto = ipv4_tuple->protocol;	// L4
	vsip_ppo.port = ipv4_tuple->dport;	// L4
	__u64 *policies_l4 = bpf_map_lookup_elem(&eg_vsip_ppo_map, &vsip_ppo);
	if (policies_l4) policies_ppo |= *policies_l4;

	if (0 == policies_ppo) {
		return -1;
	}

	struct vsip_cidr_t vsip_cidr = {
		.prefixlen = full_vsip_cidr_prefix,
		.tunnel_id = tunnel_id,
		.local_ip = ipv4_tuple->saddr,
		.remote_ip = ipv4_tuple->daddr,
	};
	__u64 *policies_dip_prim = bpf_map_lookup_elem(&eg_vsip_prim_map, &vsip_cidr);
	if (policies_dip_prim && (policies_ppo & *policies_dip_prim))
		return 0;

	__u64 *policies_dip_supp = bpf_map_lookup_elem(&eg_vsip_supp_map, &vsip_cidr);
	__u64 *policies_dip_except = bpf_map_lookup_elem(&eg_vsip_except_map, &vsip_cidr);
	if (policies_dip_supp) {
		__u64 excepts = (policies_dip_except) ? *policies_dip_except : 0;
		if ((*policies_dip_supp & ~excepts) & policies_ppo)
			return 0;
	}

	return -1;
}

/*
   egress_policy_check checks whether an incoming packet of the specific tunnel id and connection tuple
     should be allowed or denied
   return value:
     0: allows this packet; no error (by either open policy or an allowing egress policy)
    -1: denies this packet; ingress policy denial error
*/
__ALWAYS_INLINE__
static inline int egress_policy_check(__u64 tunnel_id, const struct ipv4_tuple_t *ipv4_tuple)
{
	if (!is_egress_enforced(tunnel_id, ipv4_tuple->saddr))
		return 0;

	return enforce_egress_policy(tunnel_id, ipv4_tuple);
}

/*
   egress_reply_packet_check checks whether the outgoing (egress) reply packet of the specific tunnel id and connection tuple
     should be allowed or denied, based on the tracked originated conn state, and the derived policy if applicable
   return value:
     0: allows this packet; no error (by either open policy or an allowing egress policy)
     non-0: denies this packet; ingress policy denial error
*/
__ALWAYS_INLINE__
static inline int egress_reply_packet_check(__u64 tunnel_id, const struct ipv4_tuple_t *ipv4_tuple, __u8 originated_conn_state)
{
	if (!(originated_conn_state & FLAG_REEVAL))
		return originated_conn_state & TRFFIC_DENIED;

	struct ipv4_tuple_t originated_tuple = {
		.protocol = ipv4_tuple->protocol,
		.saddr = ipv4_tuple->daddr,
		.daddr = ipv4_tuple->saddr,
		.sport = ipv4_tuple->dport,
		.dport = ipv4_tuple->sport,
	};
	return ingress_policy_check(tunnel_id, &originated_tuple);
}

/*
   ingress_reply_packet_check checks whether the incoming (ingress) reply packet of the specific tunnel id and connection tuple
     should be allowed or denied, based on the tracked originated conn state, and the derived policy if applicable
   return value:
     0: allows this packet; no error (by either open policy or an allowing egress policy)
     non-0: denies this packet; ingress policy denial error
*/
__ALWAYS_INLINE__
static inline int ingress_reply_packet_check(__u64 tunnel_id, const struct ipv4_tuple_t *ipv4_tuple, __u8 originated_conn_state)
{
	if (!(originated_conn_state & FLAG_REEVAL))
		return originated_conn_state & TRFFIC_DENIED;

	struct ipv4_tuple_t originated_tuple = {
		.protocol = ipv4_tuple->protocol,
		.saddr = ipv4_tuple->daddr,
		.daddr = ipv4_tuple->saddr,
		.sport = ipv4_tuple->dport,
		.dport = ipv4_tuple->sport,
	};
	return egress_policy_check(tunnel_id, &originated_tuple);
}
