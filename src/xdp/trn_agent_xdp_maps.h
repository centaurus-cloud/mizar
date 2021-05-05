// SPDX-License-Identifier: GPL-2.0-or-later
/**
 * @file agent_maps.h
 * @author Sherif Abdelwahab (@zasherif)
 *
 * @brief Defines ebpf maps of transit agent
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

#include "extern/bpf_helpers.h"
#include "extern/xdpcap_hook.h"

#include "trn_datamodel.h"
struct bpf_map_def SEC("maps") jmp_table = {
	.type = BPF_MAP_TYPE_PROG_ARRAY,
	.key_size = sizeof(__u32),
	.value_size = sizeof(__u32),
	.max_entries = 100,
};
BPF_ANNOTATE_KV_PAIR(jmp_table, __u32, __u32);

struct bpf_map_def SEC("maps") agentmetadata_map = {
	.type = BPF_MAP_TYPE_ARRAY,
	.key_size = sizeof(int),
	.value_size = sizeof(struct agent_metadata_t),
	.max_entries = 1,
	.map_flags = 0,
};
BPF_ANNOTATE_KV_PAIR(agentmetadata_map, int, struct agent_metadata_t);

struct bpf_map_def SEC("maps") endpoints_map = {
	.type = BPF_MAP_TYPE_HASH,
	.key_size = sizeof(struct endpoint_key_t),
	.value_size = sizeof(struct endpoint_t),
	.max_entries = TRAN_MAX_NEP,
	.map_flags = 0,
};
BPF_ANNOTATE_KV_PAIR(endpoints_map, struct endpoint_key_t, struct endpoint_t);

struct bpf_map_def SEC("maps") packet_metadata_map = {
	.type = BPF_MAP_TYPE_HASH,
	.key_size = sizeof(struct packet_metadata_key_t),
	.value_size = sizeof(struct packet_metadata_t),
	.max_entries = TRAN_MAX_NEP,
	.map_flags = 0,
};
BPF_ANNOTATE_KV_PAIR(packet_metadata_map, struct packet_metadata_key_t, struct packet_metadata_t);

struct bpf_map_def SEC("maps") interfaces_map = {
	.type = BPF_MAP_TYPE_DEVMAP,
	.key_size = sizeof(int),
	.value_size = sizeof(int),
	.max_entries = 1,
};
BPF_ANNOTATE_KV_PAIR(interface_map, int, int);

struct bpf_map_def SEC("maps") xdpcap_hook = XDPCAP_HOOK();

struct bpf_map_def SEC("maps") fwd_flow_mod_cache_ref = {
	.type = BPF_MAP_TYPE_ARRAY_OF_MAPS,
	.key_size = sizeof(int),
	.value_size = sizeof(__u32),
	.max_entries = 1,
	.map_flags = 0,
};
BPF_ANNOTATE_KV_PAIR(fwd_flow_mod_cache_ref, int, __u32);

struct bpf_map_def SEC("maps") rev_flow_mod_cache_ref = {
	.type = BPF_MAP_TYPE_ARRAY_OF_MAPS,
	.key_size = sizeof(int),
	.value_size = sizeof(__u32),
	.max_entries = 1,
	.map_flags = 0,
};
BPF_ANNOTATE_KV_PAIR(rev_flow_mod_cache_ref, int, __u32);

struct bpf_map_def SEC("maps") ep_flow_host_cache_ref = {
	.type = BPF_MAP_TYPE_ARRAY_OF_MAPS,
	.key_size = sizeof(int),
	.value_size = sizeof(__u32),
	.max_entries = 1,
	.map_flags = 0,
};
BPF_ANNOTATE_KV_PAIR(ep_flow_host_cache_ref, int, __u32);

struct bpf_map_def SEC("maps") ep_host_cache_ref = {
	.type = BPF_MAP_TYPE_ARRAY_OF_MAPS,
	.key_size = sizeof(int),
	.value_size = sizeof(__u32),
	.max_entries = 1,
	.map_flags = 0,
};
BPF_ANNOTATE_KV_PAIR(ep_host_cache_ref, int, __u32);

// DONOT change the location of this inlude for now.
// pinned maps for egress policy checks (shared by transit agent xdp progs)
// and the global conn_track map
#include "shared_map_defs.h"
