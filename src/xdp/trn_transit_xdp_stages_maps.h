// SPDX-License-Identifier: GPL-2.0-or-later
/**
 * @file transit_maps.h
 * @author Sherif Abdelwahab (@zasherif)
 *
 * @brief Defines ebpf maps of transit XDP
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

/* Defines array of maps that carry the main transit XDP program maps accross tail calls */

struct bpf_map_def SEC("maps") networks_map_ref = {
	.type = BPF_MAP_TYPE_ARRAY_OF_MAPS,
	.key_size = sizeof(int),
	.value_size = sizeof(__u32),
	.max_entries = 1,
	.map_flags = 0,
};
BPF_ANNOTATE_KV_PAIR(networks_map_ref, int, __u32);

struct bpf_map_def SEC("maps") vpc_map_ref = {
	.type = BPF_MAP_TYPE_ARRAY_OF_MAPS,
	.key_size = sizeof(int),
	.value_size = sizeof(__u32),
	.max_entries = 1,
	.map_flags = 0,
};
BPF_ANNOTATE_KV_PAIR(vpc_map_ref, int, __u32);

struct bpf_map_def SEC("maps") endpoints_map_ref = {
	.type = BPF_MAP_TYPE_ARRAY_OF_MAPS,
	.key_size = sizeof(int),
	.value_size = sizeof(__u32),
	.max_entries = 1,
	.map_flags = 0,
};
BPF_ANNOTATE_KV_PAIR(endpoints_map_ref, int, __u32);

struct bpf_map_def SEC("maps") port_map_ref = {
	.type = BPF_MAP_TYPE_ARRAY_OF_MAPS,
	.key_size = sizeof(int),
	.value_size = sizeof(__u32),
	.max_entries = 1,
	.map_flags = 0,
};
BPF_ANNOTATE_KV_PAIR(port_map_ref, int, __u32);

struct bpf_map_def SEC("maps") hosted_endpoints_iface_map_ref = {
	.type = BPF_MAP_TYPE_ARRAY_OF_MAPS,
	.key_size = sizeof(int),
	.value_size = sizeof(__u32),
	.max_entries = 1,
	.map_flags = 0,
};
BPF_ANNOTATE_KV_PAIR(hosted_endpoints_iface_map_ref, int, __u32);

struct bpf_map_def SEC("maps") interface_config_map_ref = {
	.type = BPF_MAP_TYPE_ARRAY_OF_MAPS,
	.key_size = sizeof(int),
	.value_size = sizeof(__u32),
	.max_entries = 1,
	.map_flags = 0,
};
BPF_ANNOTATE_KV_PAIR(interface_config_map_ref, int, __u32);

struct bpf_map_def SEC("maps") interfaces_map_ref = {
	.type = BPF_MAP_TYPE_ARRAY_OF_MAPS,
	.key_size = sizeof(int),
	.value_size = sizeof(__u32),
	.max_entries = 1,
	.map_flags = 0,
};
BPF_ANNOTATE_KV_PAIR(interface_map_ref, int, __u32);

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