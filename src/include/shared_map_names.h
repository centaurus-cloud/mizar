// SPDX-License-Identifier: GPL-2.0-or-later
/**
 * @file conntrack_common.h
 * @author Hongwei Chen (@hong.chen@futurewei.com)
 *
 * @brief Defines common code used in metwork policy and conntrack feature
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

// global pinned map file paths
static const char *eg_vsip_enforce_map_path	= "/sys/fs/bpf/eg_vsip_enforce_map";
static const char *eg_vsip_prim_map_path	= "/sys/fs/bpf/eg_vsip_prim_map";
static const char *eg_vsip_ppo_map_path 	= "/sys/fs/bpf/eg_vsip_ppo_map";
static const char *eg_vsip_supp_map_path	= "/sys/fs/bpf/eg_vsip_supp_map";
static const char *eg_vsip_except_map_path	= "/sys/fs/bpf/eg_vsip_except_map";
static const char *ing_vsip_enforce_map_path	= "/sys/fs/bpf/ing_vsip_enforce_map";
static const char *ing_vsip_prim_map_path	= "/sys/fs/bpf/ing_vsip_prim_map";
static const char *ing_vsip_ppo_map_path	= "/sys/fs/bpf/ing_vsip_ppo_map";
static const char *ing_vsip_supp_map_path	= "/sys/fs/bpf/ing_vsip_supp_map";
static const char *ing_vsip_except_map_path	= "/sys/fs/bpf/ing_vsip_except_map";
static const char *conn_track_cache_path	= "/sys/fs/bpf/conn_track_cache";
