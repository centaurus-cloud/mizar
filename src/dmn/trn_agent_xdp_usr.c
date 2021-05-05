// SPDX-License-Identifier: GPL-2.0-or-later
/**
 * @file trn_agent_xdp_usr.c
 * @author Sherif Abdelwahab (@zasherif)
 *         Phu Tran          (@phudtran)
 *
 * @brief User space APIs to program transit agent
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

#include "extern/linux/err.h"
#include "trn_agent_xdp_usr.h"
#include "trn_log.h"
#include "shared_map_names.h"

#define _REUSE_MAP_IF_PINNED(map)                                        	\
	do {									\
		int err_code;							\
		if (0 != (err_code = _reuse_pinned_map_if_exists(md->obj, 	\
			#map, 							\
			map##_path)))						\
		{ 								\
			TRN_LOG_INFO("failed to reuse shared map at %s, error code %d\n", map##_path, err_code); \
			return 1;						\
		}								\
	} while (0)

static int def_fwd_flow_mod_cache_map_fd = -1;
static int def_rev_flow_mod_cache_map_fd = -1;
static int def_ep_flow_host_cache_map_fd = -1;
static int def_ep_host_cache_map_fd = -1;

int trn_agent_user_metadata_free(struct agent_user_metadata_t *md)
{
	__u32 curr_prog_id = 0;

	if (bpf_map__unpin(md->xdpcap_hook_map, md->pcapfile)) {
		TRN_LOG_ERROR("Failed to unpin the pcap map file.");
		return 1;
	}

	if (bpf_get_link_xdp_id(md->ifindex, &curr_prog_id, md->xdp_flags)) {
		TRN_LOG_ERROR("bpf_get_link_xdp_id failed.");
		return 1;
	}

	if (md->prog_id == curr_prog_id)
		bpf_set_link_xdp_fd(md->ifindex, -1, md->xdp_flags);
	else if (!curr_prog_id)
		TRN_LOG_WARN("Couldn't find a prog id on a given interface.");
	else
		TRN_LOG_WARN("program on interface changed, not removing.");

	bpf_object__close(md->obj);

	return 0;
}

static int _trn_agent_update_inner_map_fd(const char *outer_map_name,
					  int outer_map_fd, int inner_map_fd)
{
	int pos = 0;
	int err = bpf_map_update_elem(outer_map_fd, &pos, &inner_map_fd, 0);

	if (err) {
		TRN_LOG_ERROR(
			"Failed to update array map of maps [%s] outer_fd [%d], inner_fd [%d]\n",
			outer_map_name, outer_map_fd, inner_map_fd);
		return 1;
	}

	return 0;
}

int trn_agent_update_agent_metadata(struct agent_user_metadata_t *umd,
				    struct agent_metadata_t *md,
				    struct user_metadata_t *eth_md)
{
	int key = 0;
	int eth_idx = md->eth.iface_index;

	umd->fwd_flow_mod_cache_map_fd = eth_md->fwd_flow_mod_cache_fd;
	umd->rev_flow_mod_cache_map_fd = eth_md->rev_flow_mod_cache_fd;
	umd->ep_flow_host_cache_map_fd = eth_md->ep_flow_host_cache_fd;
	umd->ep_host_cache_map_fd = eth_md->ep_host_cache_fd;

	int err = bpf_map_update_elem(umd->agentmetadata_map_fd, &key, md, 0);
	if (err) {
		TRN_LOG_ERROR("Configuring agent metadata (err:%d).", err);
		return 1;
	}

	err = bpf_map_update_elem(umd->interfaces_map_fd, &key, &eth_idx, 0);

	if (err) {
		TRN_LOG_ERROR(
			"Failed to update agent's interfaces map (err:%d).",
			err);
		return 1;
	}

	if (_trn_agent_update_inner_map_fd("fwd_flow_mod_cache_ref",
					   umd->fwd_flow_mod_cache_ref_fd,
					   umd->fwd_flow_mod_cache_map_fd))
		return 1;

	if (_trn_agent_update_inner_map_fd("rev_flow_mod_cache_ref",
					   umd->rev_flow_mod_cache_ref_fd,
					   umd->rev_flow_mod_cache_map_fd))
		return 1;

	if (_trn_agent_update_inner_map_fd("ep_flow_host_cache_ref",
					   umd->ep_flow_host_cache_ref_fd,
					   umd->ep_flow_host_cache_map_fd))
		return 1;

	if (_trn_agent_update_inner_map_fd("ep_host_cache_ref",
					   umd->ep_host_cache_ref_fd,
					   umd->ep_host_cache_map_fd))
		return 1;

	return 0;
}

int trn_agent_delete_agent_metadata(struct agent_user_metadata_t *umd)
{
	// We call update with a zeroed out struct
	// This is becauses the agent metadata is stored as an array
	struct agent_metadata_t md;
	memset(&md, 0, sizeof(md));

	int key = 0;
	int eth_idx = md.eth.iface_index;

	umd->fwd_flow_mod_cache_map_fd = def_fwd_flow_mod_cache_map_fd;
	umd->rev_flow_mod_cache_map_fd = def_rev_flow_mod_cache_map_fd;
	umd->ep_flow_host_cache_map_fd = def_ep_flow_host_cache_map_fd;
	umd->ep_host_cache_map_fd = def_ep_host_cache_map_fd;

	int err = bpf_map_update_elem(umd->agentmetadata_map_fd, &key, &md, 0);
	if (err) {
		TRN_LOG_ERROR("Configuring agent metadata (err:%d).", err);
		return 1;
	}

	err = bpf_map_update_elem(umd->interfaces_map_fd, &key, &eth_idx, 0);

	if (err) {
		TRN_LOG_ERROR(
			"Failed to update agent's interfaces map (err:%d).",
			err);
		return 1;
	}

	if (_trn_agent_update_inner_map_fd("fwd_flow_mod_cache_ref",
					   umd->fwd_flow_mod_cache_ref_fd,
					   umd->fwd_flow_mod_cache_map_fd))
		return 1;

	if (_trn_agent_update_inner_map_fd("rev_flow_mod_cache_ref",
					   umd->rev_flow_mod_cache_ref_fd,
					   umd->rev_flow_mod_cache_map_fd))
		return 1;

	if (_trn_agent_update_inner_map_fd("ep_flow_host_cache_ref",
					   umd->ep_flow_host_cache_ref_fd,
					   umd->ep_flow_host_cache_map_fd))
		return 1;

	if (_trn_agent_update_inner_map_fd("ep_host_cache_ref",
					   umd->ep_host_cache_ref_fd,
					   umd->ep_host_cache_map_fd))
		return 1;

	return 0;
}

int trn_agent_get_agent_metadata(struct agent_user_metadata_t *umd,
				 struct agent_metadata_t *md)
{
	int key = 0;
	int err = bpf_map_lookup_elem(umd->agentmetadata_map_fd, &key, md);
	if (err) {
		TRN_LOG_ERROR("Querying agent metadata (err:%d).", err);
		return 1;
	}
	return 0;
}

int trn_agent_add_prog(struct agent_user_metadata_t *umd, int prog, int prog_fd)
{
	int err = bpf_map_update_elem(umd->jmp_table_fd, &prog, &prog_fd, 0);
	if (err) {
		TRN_LOG_ERROR("Error add prog to agent jmp table (err:%d).",
			      err);
		return 1;
	}
	return 0;
}

int trn_agent_update_endpoint(struct agent_user_metadata_t *umd,
			      struct endpoint_key_t *epkey,
			      struct endpoint_t *ep)
{
	int err = bpf_map_update_elem(umd->endpoints_map_fd, epkey, ep, 0);
	if (err) {
		TRN_LOG_ERROR("Store endpoint mapping failed (err:%d).", err);
		return 1;
	}
	return 0;
}

int trn_agent_get_endpoint(struct agent_user_metadata_t *umd,
			   struct endpoint_key_t *epkey, struct endpoint_t *ep)
{
	int err = bpf_map_lookup_elem(umd->endpoints_map_fd, epkey, ep);
	if (err) {
		TRN_LOG_ERROR(
			"Querying agent endpoint mapping failed (err:%d).",
			err);
		return 1;
	}
	return 0;
}

int trn_agent_delete_endpoint(struct agent_user_metadata_t *umd,
			      struct endpoint_key_t *epkey)
{
	int err = bpf_map_delete_elem(umd->endpoints_map_fd, epkey);
	if (err) {
		TRN_LOG_ERROR(
			"Deleting agent endpoint mapping failed (err:%d).",
			err);
		return 1;
	}
	return 0;
}

int trn_agent_update_packet_metadata(struct agent_user_metadata_t *umd,
			      struct packet_metadata_key_t *key,
			      struct packet_metadata_t *packet_metadata)
{
	int err = bpf_map_update_elem(umd->packet_metadata_map_fd, key, packet_metadata, 0);
	if (err) {
		TRN_LOG_ERROR("Store packet metadata mapping failed (err:%d).", err);
		return 1;
	}
	return 0;
}

int trn_agent_delete_packet_metadata(struct agent_user_metadata_t *umd,
			      struct packet_metadata_key_t *key)
{
	int err = bpf_map_delete_elem(umd->packet_metadata_map_fd, key);
	if (err) {
		TRN_LOG_ERROR(
			"Deleting packet metadata mapping failed (err:%d).", err);
		return 1;
	}
	return 0;
}

int trn_agent_bpf_maps_init(struct agent_user_metadata_t *md)
{
	md->jmp_table_map = bpf_map__next(NULL, md->obj);
	md->agentmetadata_map = bpf_map__next(md->jmp_table_map, md->obj);
	md->endpoints_map = bpf_map__next(md->agentmetadata_map, md->obj);
	md->packet_metadata_map = bpf_map__next(md->endpoints_map, md->obj);
	md->interfaces_map = bpf_map__next(md->packet_metadata_map, md->obj);
	md->xdpcap_hook_map = bpf_map__next(md->interfaces_map, md->obj);
	md->fwd_flow_mod_cache_ref =
		bpf_map__next(md->xdpcap_hook_map, md->obj);
	md->rev_flow_mod_cache_ref =
		bpf_map__next(md->fwd_flow_mod_cache_ref, md->obj);
	md->ep_flow_host_cache_ref =
		bpf_map__next(md->rev_flow_mod_cache_ref, md->obj);
	md->ep_host_cache_ref =
		bpf_map__next(md->ep_flow_host_cache_ref, md->obj);
	md->eg_vsip_enforce_map = bpf_map__next(md->ep_host_cache_ref, md->obj);
	md->eg_vsip_prim_map = bpf_map__next(md->eg_vsip_enforce_map, md->obj);
	md->eg_vsip_ppo_map = bpf_map__next(md->eg_vsip_prim_map, md->obj);
	md->eg_vsip_supp_map = bpf_map__next(md->eg_vsip_ppo_map, md->obj);
	md->eg_vsip_except_map = bpf_map__next(md->eg_vsip_supp_map, md->obj);
	md->ing_vsip_enforce_map = bpf_map__next(md->eg_vsip_except_map, md->obj);
	md->ing_vsip_prim_map = bpf_map__next(md->ing_vsip_enforce_map, md->obj);
	md->ing_vsip_ppo_map = bpf_map__next(md->ing_vsip_prim_map, md->obj);
	md->ing_vsip_supp_map = bpf_map__next(md->ing_vsip_ppo_map, md->obj);
	md->ing_vsip_except_map = bpf_map__next(md->ing_vsip_supp_map, md->obj);
	md->conn_track_cache = bpf_map__next(md->ing_vsip_except_map, md->obj);

	if (!md->jmp_table_map || !md->agentmetadata_map ||
	    !md->endpoints_map || !md->xdpcap_hook_map ||
	    !md->fwd_flow_mod_cache_ref || !md->rev_flow_mod_cache_ref ||
	    !md->ep_flow_host_cache_ref || !md->ep_host_cache_ref ||
	    !md->eg_vsip_enforce_map || !md->eg_vsip_prim_map ||
	    !md->eg_vsip_ppo_map || !md->eg_vsip_supp_map ||
	    !md->eg_vsip_except_map ||  !md->ing_vsip_enforce_map ||
	    !md->ing_vsip_prim_map || !md->ing_vsip_ppo_map ||
	    !md->ing_vsip_supp_map || !md->ing_vsip_except_map ||
	    !md->conn_track_cache || !md->packet_metadata_map) {
		TRN_LOG_ERROR("Failure finding maps objects.");
		return 1;
	}

	md->jmp_table_fd = bpf_map__fd(md->jmp_table_map);
	md->agentmetadata_map_fd = bpf_map__fd(md->agentmetadata_map);
	md->endpoints_map_fd = bpf_map__fd(md->endpoints_map);
	md->packet_metadata_map_fd = bpf_map__fd(md->packet_metadata_map);
	md->interfaces_map_fd = bpf_map__fd(md->interfaces_map);
	md->fwd_flow_mod_cache_ref_fd = bpf_map__fd(md->fwd_flow_mod_cache_ref);
	md->rev_flow_mod_cache_ref_fd = bpf_map__fd(md->rev_flow_mod_cache_ref);
	md->ep_flow_host_cache_ref_fd = bpf_map__fd(md->ep_flow_host_cache_ref);
	md->ep_host_cache_ref_fd = bpf_map__fd(md->ep_host_cache_ref);
	md->eg_vsip_enforce_map_fd	= bpf_map__fd(md->eg_vsip_enforce_map);
	md->eg_vsip_prim_map_fd		= bpf_map__fd(md->eg_vsip_prim_map);
	md->eg_vsip_ppo_map_fd		= bpf_map__fd(md->eg_vsip_ppo_map);
	md->eg_vsip_supp_map_fd		= bpf_map__fd(md->eg_vsip_supp_map);
	md->eg_vsip_except_map_fd	= bpf_map__fd(md->eg_vsip_except_map);
	md->ing_vsip_enforce_map_fd	= bpf_map__fd(md->ing_vsip_enforce_map);
	md->ing_vsip_prim_map_fd	= bpf_map__fd(md->ing_vsip_prim_map);
	md->ing_vsip_ppo_map_fd		= bpf_map__fd(md->ing_vsip_ppo_map);
	md->ing_vsip_supp_map_fd	= bpf_map__fd(md->ing_vsip_supp_map);
	md->ing_vsip_except_map_fd	= bpf_map__fd(md->ing_vsip_except_map);
	md->conn_track_cache_fd		= bpf_map__fd(md->conn_track_cache);

	if (bpf_map__unpin(md->xdpcap_hook_map, md->pcapfile) == 0) {
		TRN_LOG_INFO("unpin exiting pcap map file: %s", md->pcapfile);
	}

	int rc = bpf_map__pin(md->xdpcap_hook_map, md->pcapfile);

	if (rc != 0) {
		TRN_LOG_ERROR("Failed to pin xdpcap map [%s].", strerror(rc));
		return 1;
	}

	// pins the policy maps & conn_track map if not yet
	bpf_map__pin(md->eg_vsip_enforce_map, eg_vsip_enforce_map_path);
	bpf_map__pin(md->eg_vsip_prim_map, eg_vsip_prim_map_path);
	bpf_map__pin(md->eg_vsip_ppo_map, eg_vsip_ppo_map_path);
	bpf_map__pin(md->eg_vsip_supp_map, eg_vsip_supp_map_path);
	bpf_map__pin(md->eg_vsip_except_map, eg_vsip_except_map_path);
	bpf_map__pin(md->ing_vsip_enforce_map, ing_vsip_enforce_map_path);
	bpf_map__pin(md->ing_vsip_prim_map, ing_vsip_prim_map_path);
	bpf_map__pin(md->ing_vsip_ppo_map, ing_vsip_ppo_map_path);
	bpf_map__pin(md->ing_vsip_supp_map, ing_vsip_supp_map_path);
	bpf_map__pin(md->ing_vsip_except_map, ing_vsip_except_map_path);
	bpf_map__pin(md->conn_track_cache, conn_track_cache_path);

	return 0;
}

static int _trn_agent_set_inner_map(struct bpf_object *obj,
				    struct bpf_map **map,
				    const char *outer_map_name,
				    int inner_map_fd)
{
	int err;
	*map = bpf_object__find_map_by_name(obj, outer_map_name);

	if (!*map) {
		TRN_LOG_ERROR("Failed to find map %s\n", outer_map_name);
		return 1;
	}

	err = bpf_map__set_inner_map_fd(*map, inner_map_fd);
	if (err) {
		TRN_LOG_ERROR(
			"Failed to set inner_map_fd for array of maps %s\n",
			outer_map_name);
		return 1;
	}

	TRN_LOG_INFO("_trn_agent_set_inner_map %s, fd:%d\n", outer_map_name,
		     inner_map_fd);
	return 0;
}

static void _trn_refresh_default_maps(void)
{
	if (def_fwd_flow_mod_cache_map_fd == -1)
		def_fwd_flow_mod_cache_map_fd =
			bpf_create_map(BPF_MAP_TYPE_LRU_HASH,
				       sizeof(struct ipv4_tuple_t),
				       sizeof(struct scaled_endpoint_remote_t),
				       TRAN_MAX_CACHE_SIZE, 0);

	if (def_rev_flow_mod_cache_map_fd == -1)
		def_rev_flow_mod_cache_map_fd =
			bpf_create_map(BPF_MAP_TYPE_LRU_HASH,
				       sizeof(struct ipv4_tuple_t),
				       sizeof(struct scaled_endpoint_remote_t),
				       TRAN_MAX_CACHE_SIZE, 0);

	if (def_ep_flow_host_cache_map_fd == -1)
		def_ep_flow_host_cache_map_fd =
			bpf_create_map(BPF_MAP_TYPE_LRU_HASH,
				       sizeof(struct ipv4_tuple_t),
				       sizeof(struct remote_endpoint_t),
				       TRAN_MAX_CACHE_SIZE, 0);

	if (def_ep_host_cache_map_fd == -1)
		def_ep_host_cache_map_fd =
			bpf_create_map(BPF_MAP_TYPE_LRU_HASH,
				       sizeof(struct endpoint_key_t),
				       sizeof(struct remote_endpoint_t),
				       TRAN_MAX_CACHE_SIZE, 0);
}

static int _reuse_pinned_map_if_exists(struct bpf_object *pobj, const char *map_name, const char *pinned_file)
{
	int fd_pinned_map = bpf_obj_get(pinned_file);
	if (fd_pinned_map >= 0) {
		struct bpf_map *map = bpf_object__find_map_by_name(pobj, map_name);
		if (!map) {
			return -ENOENT;
		}

		if (0 != bpf_map__reuse_fd(map, fd_pinned_map)) {
			return -EBADF;
		}
	}

	return 0;
}

static int _trn_bpf_agent_prog_load_xattr(struct agent_user_metadata_t *md,
					  const struct bpf_prog_load_attr *attr,
					  struct bpf_object **pobj,
					  int *prog_fd)
{
	struct bpf_program *prog, *first_prog = NULL;
	_trn_refresh_default_maps();

	md->fwd_flow_mod_cache_map_fd = def_fwd_flow_mod_cache_map_fd;
	md->rev_flow_mod_cache_map_fd = def_rev_flow_mod_cache_map_fd;
	md->ep_flow_host_cache_map_fd = def_ep_flow_host_cache_map_fd;
	md->ep_host_cache_map_fd = def_ep_host_cache_map_fd;

	*pobj = bpf_object__open(attr->file);

	if (IS_ERR_OR_NULL(*pobj)) {
		TRN_LOG_ERROR("Error openning bpf file: %s\n", attr->file);
		return 1;
	}

	if (_trn_agent_set_inner_map(*pobj, &md->fwd_flow_mod_cache_ref,
				     "fwd_flow_mod_cache_ref",
				     md->fwd_flow_mod_cache_map_fd)) {
		md->fwd_flow_mod_cache_ref = NULL;
		TRN_LOG_INFO("fwd_flow_mod_cache_ref inner fd is not set!\n");
		goto error;
	}

	if (_trn_agent_set_inner_map(*pobj, &md->rev_flow_mod_cache_ref,
				     "rev_flow_mod_cache_ref",
				     md->rev_flow_mod_cache_map_fd)) {
		md->rev_flow_mod_cache_ref = NULL;
		TRN_LOG_INFO("rev_flow_mod_cache_ref inner fd is not set!\n");
		goto error;
	}

	if (_trn_agent_set_inner_map(*pobj, &md->ep_flow_host_cache_ref,
				     "ep_flow_host_cache_ref",
				     md->ep_flow_host_cache_map_fd)) {
		md->ep_flow_host_cache_ref = NULL;
		TRN_LOG_INFO("ep_flow_host_cache_ref inner fd is not set!\n");
		goto error;
	}

	if (_trn_agent_set_inner_map(*pobj, &md->ep_host_cache_ref,
				     "ep_host_cache_ref",
				     md->ep_host_cache_map_fd)) {
		md->ep_flow_host_cache_ref = NULL;
		TRN_LOG_INFO("ep_flow_host_cache_ref inner fd is not set!\n");
		goto error;
	}

	// to share the pinned egress policy maps, if applicable
	_REUSE_MAP_IF_PINNED(eg_vsip_enforce_map);
	_REUSE_MAP_IF_PINNED(eg_vsip_prim_map);
	_REUSE_MAP_IF_PINNED(eg_vsip_ppo_map);
	_REUSE_MAP_IF_PINNED(eg_vsip_supp_map);
	_REUSE_MAP_IF_PINNED(eg_vsip_except_map);
	_REUSE_MAP_IF_PINNED(ing_vsip_enforce_map);
	_REUSE_MAP_IF_PINNED(ing_vsip_prim_map);
	_REUSE_MAP_IF_PINNED(ing_vsip_ppo_map);
	_REUSE_MAP_IF_PINNED(ing_vsip_supp_map);
	_REUSE_MAP_IF_PINNED(ing_vsip_except_map);
	_REUSE_MAP_IF_PINNED(conn_track_cache);

	/* Only one prog is supported */
	bpf_object__for_each_program(prog, *pobj)
	{
		bpf_program__set_xdp(prog);
		if (!first_prog)
			first_prog = prog;
	}

	bpf_object__load(*pobj);

	if (!first_prog) {
		TRN_LOG_ERROR("Failed to find XDP program in object file: %s\n",
			      attr->file);
		goto error;
	}

	*prog_fd = bpf_program__fd(first_prog);

	md->fwd_flow_mod_cache_ref_fd = bpf_map__fd(md->fwd_flow_mod_cache_ref);
	md->rev_flow_mod_cache_ref_fd = bpf_map__fd(md->rev_flow_mod_cache_ref);
	md->ep_flow_host_cache_ref_fd = bpf_map__fd(md->ep_flow_host_cache_ref);
	md->ep_host_cache_ref_fd = bpf_map__fd(md->ep_host_cache_ref);

	return 0;
error:
	TRN_LOG_ERROR("Error adding loading tranist agent from file %s.\n",
		      attr->file);
	bpf_object__close(*pobj);
	return 1;
}

int trn_agent_metadata_init(struct agent_user_metadata_t *md, char *itf,
			    char *agent_kern_path, int xdp_flags)
{
	int rc;
	struct rlimit r = { RLIM_INFINITY, RLIM_INFINITY };
	struct bpf_prog_load_attr prog_load_attr = { .prog_type =
							     BPF_PROG_TYPE_XDP,
						     .file = agent_kern_path };
	__u32 info_len = sizeof(md->info);

	md->xdp_flags = xdp_flags;

	if (setrlimit(RLIMIT_MEMLOCK, &r)) {
		TRN_LOG_ERROR("setrlimit(RLIMIT_MEMLOCK)");
		return 1;
	}

	snprintf(md->pcapfile, sizeof(md->pcapfile),
		 "/sys/fs/bpf/%s_transit_agent_pcap", itf);

	md->ifindex = if_nametoindex(itf);
	if (!md->ifindex) {
		TRN_LOG_ERROR("Error retrieving index of interface");
		return 1;
	}

	if (_trn_bpf_agent_prog_load_xattr(md, &prog_load_attr, &md->obj,
					   &md->prog_fd)) {
		TRN_LOG_ERROR("Error loading bpf: %s", agent_kern_path);
		return 1;
	}

	rc = trn_agent_bpf_maps_init(md);

	if (rc != 0) {
		return 1;
	}

	if (!md->prog_fd) {
		TRN_LOG_ERROR("load_bpf_file: %s", strerror(errno));
		return 1;
	}

	if (bpf_set_link_xdp_fd(md->ifindex, md->prog_fd, xdp_flags) < 0) {
		TRN_LOG_ERROR("link set xdp fd failed");
		return 1;
	}

	rc = bpf_obj_get_info_by_fd(md->prog_fd, &md->info, &info_len);
	if (rc != 0) {
		TRN_LOG_ERROR("can't get prog info - %s.", strerror(errno));
		return 1;
	}
	md->prog_id = md->info.id;

	return 0;
}

int trn_update_agent_network_policy_map(int fd,
					 struct vsip_cidr_t *ipcidr,
					 __u64 bitmap)
{
	int err = bpf_map_update_elem(fd, ipcidr, &bitmap, 0);
	if (err) {
		TRN_LOG_ERROR("Store network policy egress CIDR map failed (err:%d) for ip address 0x%x wit remote cidr 0x%x / %d ",
			err, ipcidr->local_ip, ipcidr->remote_ip, ipcidr->prefixlen);
		return 1;
	}
	return 0;
}

int trn_delete_agent_network_policy_map(int fd,
					 struct vsip_cidr_t *ipcidr)
{
	int err = bpf_map_delete_elem(fd, ipcidr);
	if (err) {
		TRN_LOG_ERROR("Delete network policy egress CIDR map failed (err:%d) for ip address 0x%x wit remote cidr 0x%x / %d ",
			err, ipcidr->local_ip, ipcidr->remote_ip, ipcidr->prefixlen);
		return 1;
	}
	return 0;
}


int trn_update_agent_network_policy_enforcement_map(struct agent_user_metadata_t *md,
						      struct vsip_enforce_t *local,
						      __u8 isenforce)
{
	int err = bpf_map_update_elem(md->eg_vsip_enforce_map_fd, local, &isenforce, 0);

	if (err) {
		TRN_LOG_ERROR("Update Enforcement egress map failed (err:%d) for ip address 0x%x. \n",
				err, local->local_ip);
		return 1;
	}

	return 0;
}

int trn_delete_agent_network_policy_enforcement_map(struct agent_user_metadata_t *md,
						      struct vsip_enforce_t *local)
{
	int err = bpf_map_delete_elem(md->eg_vsip_enforce_map_fd, local);
	if (err) {
		TRN_LOG_ERROR("Delete Enforcement egress map failed (err:%d) for ip address 0x%x. ",
				err, local->local_ip);
		return 1;
	}

	return 0;
}

int trn_update_agent_network_policy_protocol_port_map(struct agent_user_metadata_t *md,
						        struct vsip_ppo_t *policy,
						        __u64 bitmap)
{
	int err = bpf_map_update_elem(md->eg_vsip_ppo_map_fd, policy, &bitmap, 0);

	if (err) {
		TRN_LOG_ERROR("Update Protocol-Port egress map failed (err:%d) for ip address 0x%x with protocol %d and port %d. \n",
				err, policy->local_ip, policy->proto, policy->port);
		return 1;
	}

	return 0;
}

int trn_delete_agent_network_policy_protocol_port_map(struct agent_user_metadata_t *md,
						        struct vsip_ppo_t *policy)
{
	int err = bpf_map_delete_elem(md->eg_vsip_ppo_map_fd, policy);
	if (err) {
		TRN_LOG_ERROR("Delete Protocol-Port egress map failed (err:%d).for ip address 0x%x with protocol %d and port %d. \n",
				err, policy->local_ip, policy->proto, policy->port);
		return 1;
	}
	return 0;
}
