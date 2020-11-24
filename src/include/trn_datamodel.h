// SPDX-License-Identifier: GPL-2.0-or-later
/**
 * @file datamodel.h
 * @author Sherif Abdelwahab (@zasherif)
 *
 * @brief Data models between user and kernel space. data propagated
 * from control-plane through transitd.
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

#include <linux/types.h>
#define __ALIGNED_64__ __attribute__((aligned(64)))
#define __ALWAYS_INLINE__ __attribute__((__always_inline__))

#define TRAN_MAX_NEP 65537
#define TRAN_MAX_NSWITCH 32768
#define TRAN_MAX_NROUTER 16384
#define TRAN_MAX_REMOTES 256
#define TRAN_MAX_ITF 256
#define TRAN_UNUSED_ITF_IDX -1

#define TRAN_SUBSTRT_VNI 0

#define TRAN_SUBSTRT_EP 0
#define TRAN_SIMPLE_EP 1
#define TRAN_SCALED_EP 2
#define TRAN_GATEWAY_EP 3

#define TRAN_MAX_PROG 100
/* XDP programs keys in transit agent */
#define XDP_TRANSIT 0

/* Cache related const */
#define TRAN_MAX_CACHE_SIZE 1000000

/* XDP programs keys in transit XDP */
enum trn_xdp_stage_t {
	XDP_TX_PROC = 0,
	XDP_PASS_PROC,
	XDP_REDIRECT_PROC,
	XDP_DROP_PROC,
	XDP_SCALED_EP_PROC
};

struct port_key_t {
	__u32 tunip[3];
	__u16 port;
	__u8 protocol;
} __attribute__((packed));

struct port_t {
	__u16 target_port;
} __attribute__((packed, aligned(4)));

struct endpoint_key_t {
	__u32 tunip[3];
} __attribute__((packed));

struct endpoint_t {
	__u32 eptype;
	__u32 nremote_ips;
	__u32 remote_ips[TRAN_MAX_REMOTES];
	int hosted_iface;
	unsigned char mac[6];
} __attribute__((packed, aligned(4)));

struct network_key_t {
	__u32 prefixlen; /* up to 32 for AF_INET, 128 for AF_INET6*/
	__u32 nip[3];
} __attribute__((packed));

struct network_t {
	__u32 prefixlen; /* up to 32 for AF_INET, 128 for AF_INET6 */
	__u32 nip[3];
	__u32 nswitches;
	__u32 switches_ips[TRAN_MAX_NSWITCH];
} __attribute__((packed, aligned(4)));

struct vpc_key_t {
	union {
		__be64 tunnel_id;
	};
} __attribute__((packed));

struct vpc_t {
	__u32 nrouters;
	__u32 routers_ips[TRAN_MAX_NROUTER];
} __attribute__((packed, aligned(4)));

struct tunnel_iface_t {
	int iface_index;
	__u32 ip;
	unsigned char mac[6];
} __attribute__((packed, aligned(4)));

struct agent_metadata_t {
	struct tunnel_iface_t eth;
	struct network_key_t nkey;
	struct network_t net;
	struct endpoint_key_t epkey;
	struct endpoint_t ep;
} __attribute__((packed, aligned(4)));

struct ipv4_tuple_t {
	__u32 saddr;
	__u32 daddr;

	/* ports */
	__u16 sport;
	__u16 dport;

	/* Addresses */
	__u8 protocol;

	/*TODO: include TCP flags, no use case for the moment! */
} __attribute__((packed));

struct remote_endpoint_t {
	__u32 ip;
	unsigned char mac[6];
} __attribute__((packed));

struct scaled_endpoint_remote_t {
	/* Addresses */
	__u32 saddr;
	__u32 daddr;

	/* ports */
	__u16 sport;
	__u16 dport;

	unsigned char h_source[6];
	unsigned char h_dest[6];
} __attribute__((packed));

/* 
 * Common stats data record shared between user program and kernel program
 */
struct metrics_record {
	__u64 n_pkts; /* # of pkts received */
	__u64 total_bytes_rx; /* to calcualte rx bandwidth */
	__u64 total_bytes_tx; /* to calcualte tx bandwidth */
	__u64 n_tx; /* TX PPS for the Bouncer (which is symmetric) == RX PPS for the bouncer */
	__u64 n_pass; /* forwarded to kernel */
	__u64 n_drop; /* packets to be dropped*/
	__u64 n_redirect; /* packets to be redirected*/
	__u64 n_aborted;
} __attribute__((packed));
