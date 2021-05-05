// SPDX-License-Identifier: GPL-2.0-or-later
/**
 * @file trn_agent_xdp_usr.h
 * @author Sherif Abdelwahab (@zasherif)
 *         Phu Tran          (@phudtran)
 *
 * @brief User space APIs and data structures to program transit
 * agent.
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

#include <argp.h>
#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <libgen.h>
#include <linux/bpf.h>
#include <linux/if_link.h>
#include <net/if.h>
#include <netinet/in.h>
#include <search.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <unistd.h>

#include "bpf/bpf.h"
#include "bpf/libbpf.h"

#include "extern/cJSON.h"

#include "trn_transit_xdp_usr.h"
#include "trn_datamodel.h"

struct agent_user_metadata_t {
	int ifindex;
	int prog_fd;
	int tunifindex;

	__u32 xdp_flags;
	__u32 prog_id;
	char pcapfile[256];

	int jmp_table_fd;
	int agentmetadata_map_fd;
	int endpoints_map_fd;
	int packet_metadata_map_fd;
	int interfaces_map_fd;
	int fwd_flow_mod_cache_ref_fd;
	int rev_flow_mod_cache_ref_fd;
	int ep_flow_host_cache_ref_fd;
	int ep_host_cache_ref_fd;
	int eg_vsip_enforce_map_fd;
	int eg_vsip_prim_map_fd;
	int eg_vsip_ppo_map_fd;
	int eg_vsip_supp_map_fd;
	int eg_vsip_except_map_fd;
	int ing_vsip_enforce_map_fd;
	int ing_vsip_prim_map_fd;
	int ing_vsip_ppo_map_fd;
	int ing_vsip_supp_map_fd;
	int ing_vsip_except_map_fd;
	int conn_track_cache_fd;

	int fwd_flow_mod_cache_map_fd;
	int rev_flow_mod_cache_map_fd;
	int ep_flow_host_cache_map_fd;
	int ep_host_cache_map_fd;

	struct bpf_map *jmp_table_map;
	struct bpf_map *agentmetadata_map;
	struct bpf_map *endpoints_map;
	struct bpf_map *packet_metadata_map;
	struct bpf_map *interfaces_map;
	struct bpf_map *fwd_flow_mod_cache_ref;
	struct bpf_map *rev_flow_mod_cache_ref;
	struct bpf_map *ep_flow_host_cache_ref;
	struct bpf_map *ep_host_cache_ref;
	struct bpf_map *xdpcap_hook_map;
	struct bpf_map *eg_vsip_enforce_map;
	struct bpf_map *eg_vsip_prim_map;
	struct bpf_map *eg_vsip_ppo_map;
	struct bpf_map *eg_vsip_supp_map;
	struct bpf_map *eg_vsip_except_map;
	struct bpf_map *ing_vsip_enforce_map;
	struct bpf_map *ing_vsip_prim_map;
	struct bpf_map *ing_vsip_ppo_map;
	struct bpf_map *ing_vsip_supp_map;
	struct bpf_map *ing_vsip_except_map;
	struct bpf_map *conn_track_cache;

	struct bpf_prog_info info;
	struct bpf_object *obj;
};

int trn_agent_user_metadata_free(struct agent_user_metadata_t *md);

int trn_agent_update_agent_metadata(struct agent_user_metadata_t *umd,
				    struct agent_metadata_t *md,
				    struct user_metadata_t *eth_md);

int trn_agent_delete_agent_metadata(struct agent_user_metadata_t *umd);

int trn_agent_get_agent_metadata(struct agent_user_metadata_t *umd,
				 struct agent_metadata_t *md);

int trn_agent_update_endpoint(struct agent_user_metadata_t *umd,
			      struct endpoint_key_t *epkey,
			      struct endpoint_t *ep);

int trn_agent_get_endpoint(struct agent_user_metadata_t *umd,
			   struct endpoint_key_t *epkey, struct endpoint_t *ep);

int trn_agent_delete_endpoint(struct agent_user_metadata_t *umd,
			      struct endpoint_key_t *epkey);

int trn_agent_update_packet_metadata(struct agent_user_metadata_t *umd,
			      struct packet_metadata_key_t *key,
			      struct packet_metadata_t *packet_metadata);

int trn_agent_delete_packet_metadata(struct agent_user_metadata_t *umd,
			      struct packet_metadata_key_t *key);

int trn_agent_bpf_maps_init(struct agent_user_metadata_t *md);

int trn_agent_metadata_init(struct agent_user_metadata_t *md, char *itf,
			    char *agent_kern_path, int xdp_flags);

int trn_agent_add_prog(struct agent_user_metadata_t *umd, int prog,
		       int prog_fd);

int trn_update_agent_network_policy_map(int fd,
					 struct vsip_cidr_t *cidr,
					 __u64 bitmap);

int trn_delete_agent_network_policy_map(int fd,
					 struct vsip_cidr_t *ipcidr);

int trn_update_agent_network_policy_enforcement_map(struct agent_user_metadata_t *md,
						      struct vsip_enforce_t *local,
						      __u8 isenforce);

int trn_delete_agent_network_policy_enforcement_map(struct agent_user_metadata_t *md,
						      struct vsip_enforce_t *local);

int trn_update_agent_network_policy_protocol_port_map(struct agent_user_metadata_t *md,
						        struct vsip_ppo_t *policy,
						        __u64 bitmap);

int trn_delete_agent_network_policy_protocol_port_map(struct agent_user_metadata_t *md,
						        struct vsip_ppo_t *policy);
