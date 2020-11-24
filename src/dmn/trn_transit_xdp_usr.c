// SPDX-License-Identifier: GPL-2.0-or-later
/**
 * @file trn_agent_xdp_usr.c
 * @author Sherif Abdelwahab (@zasherif)
 *         Phu Tran          (@phudtran)
 *
 * @brief User space APIs to program transit xdp program (switches and
 * routers)
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

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <linux/if.h>
#include "extern/linux/err.h"
#include "trn_transit_xdp_usr.h"
#include "trn_log.h"

#define _SET_INNER_MAP(map)                                                    \
	do {                                                                   \
		if (_trn_set_inner_map(stage, &stage->map##_ref, #map "_ref",  \
				       md->map##_fd)) {                        \
			stage->map##_ref = NULL;                               \
			TRN_LOG_INFO(#map " is not used by %s\n", prog_path);  \
		}                                                              \
	} while (0)

#define _UPDATE_INNER_MAP(map)                                                 \
	do {                                                                   \
		if (stage->map##_ref &&                                        \
		    _trn_update_inner_map_fd(#map "_ref", stage->map##_ref,    \
					     &stage->map##_ref_fd,             \
					     md->map##_fd))                    \
			goto error;                                            \
	} while (0)

int trn_user_metadata_free(struct user_metadata_t *md)
{
	__u32 curr_prog_id = 0;
	int i;

	if (bpf_map__unpin(md->xdpcap_hook_map, md->pcapfile)) {
		TRN_LOG_ERROR("Failed to unpin the pcap map file %s.",
			      md->pcapfile);
		return 1;
	}

	if (bpf_get_link_xdp_id(md->ifindex, &curr_prog_id, md->xdp_flags)) {
		TRN_LOG_ERROR("bpf_get_link_xdp_id failed");
		return 1;
	}

	if (md->prog_id == curr_prog_id)
		bpf_set_link_xdp_fd(md->ifindex, -1, md->xdp_flags);
	else if (!curr_prog_id)
		TRN_LOG_WARN("couldn't find a prog id on a given interface\n");
	else
		TRN_LOG_WARN("program on interface changed, not removing\n");

	for (i = 0; i < TRAN_MAX_PROG; i++) {
		if (!IS_ERR_OR_NULL(md->ebpf_progs[i].obj)) {
			bpf_object__close(md->ebpf_progs[i].obj);
		}
	}

	bpf_object__close(md->obj);

	return 0;
}

int trn_bpf_maps_init(struct user_metadata_t *md)
{
	md->jmp_table_map = bpf_map__next(NULL, md->obj);
	md->networks_map = bpf_map__next(md->jmp_table_map, md->obj);
	md->vpc_map = bpf_map__next(md->networks_map, md->obj);
	md->endpoints_map = bpf_map__next(md->vpc_map, md->obj);
	md->port_map = bpf_map__next(md->endpoints_map, md->obj);
	md->hosted_endpoints_iface_map = bpf_map__next(md->port_map, md->obj);
	md->interface_config_map =
		bpf_map__next(md->hosted_endpoints_iface_map, md->obj);
	md->interfaces_map = bpf_map__next(md->interface_config_map, md->obj);
	md->fwd_flow_mod_cache = bpf_map__next(md->interfaces_map, md->obj);
	md->rev_flow_mod_cache = bpf_map__next(md->fwd_flow_mod_cache, md->obj);
	md->ep_flow_host_cache = bpf_map__next(md->rev_flow_mod_cache, md->obj);
	md->ep_host_cache = bpf_map__next(md->ep_flow_host_cache, md->obj);
	md->xdpcap_hook_map = bpf_map__next(md->ep_host_cache, md->obj);
	md->metrics_table_map = bpf_map__next(md->xdpcap_hook_map, md->obj);

	if (!md->networks_map || !md->vpc_map || !md->endpoints_map ||
	    !md->port_map || !md->hosted_endpoints_iface_map || !md->interface_config_map ||
	    !md->interfaces_map || !md->fwd_flow_mod_cache ||
	    !md->rev_flow_mod_cache || !md->ep_flow_host_cache ||
	    !md->ep_host_cache || !md->xdpcap_hook_map || !md->jmp_table_map ||
	    !md->metrics_table_map) {
		TRN_LOG_ERROR("Failure finding maps objects.");
		return 1;
	}

	md->jmp_table_fd = bpf_map__fd(md->jmp_table_map);
	md->networks_map_fd = bpf_map__fd(md->networks_map);
	md->vpc_map_fd = bpf_map__fd(md->vpc_map);
	md->endpoints_map_fd = bpf_map__fd(md->endpoints_map);
	md->port_map_fd = bpf_map__fd(md->port_map);
	md->interface_config_map_fd = bpf_map__fd(md->interface_config_map);
	md->hosted_endpoints_iface_map_fd =
		bpf_map__fd(md->hosted_endpoints_iface_map);
	md->interfaces_map_fd = bpf_map__fd(md->interfaces_map);
	md->fwd_flow_mod_cache_fd = bpf_map__fd(md->fwd_flow_mod_cache);
	md->rev_flow_mod_cache_fd = bpf_map__fd(md->rev_flow_mod_cache);
	md->ep_flow_host_cache_fd = bpf_map__fd(md->ep_flow_host_cache);
	md->ep_host_cache_fd = bpf_map__fd(md->ep_host_cache);
	md->metrics_table_map_fd = bpf_map__fd(md->metrics_table_map);

	if (bpf_map__unpin(md->xdpcap_hook_map, md->pcapfile) == 0) {
		TRN_LOG_INFO("unpin exiting pcap map file: %s", md->pcapfile);
	}

	int rc = bpf_map__pin(md->xdpcap_hook_map, md->pcapfile);

	if (rc != 0) {
		TRN_LOG_ERROR("Failed to pin xdpcap map to %s", md->pcapfile);
		return 1;
	}

	return 0;
}

int trn_update_network(struct user_metadata_t *md, struct network_key_t *netkey,
		       struct network_t *net)
{
	netkey->prefixlen += 64; /* tunid size */
	int err = bpf_map_update_elem(md->networks_map_fd, netkey, net, 0);
	if (err) {
		TRN_LOG_ERROR("Store network mapping failed (err:%d)", err);
		return 1;
	}
	return 0;
}

static int get_unused_itf_index(struct user_metadata_t *md)
{
	// Simple search for an unused index for now
	int i;
	for (i = 0; i < TRAN_MAX_ITF; i++) {
		if (md->itf_idx[i] == TRAN_UNUSED_ITF_IDX)
			return i;
	}
	return -1;
}

int trn_update_port(struct user_metadata_t *md, struct port_key_t *portkey,
		    struct port_t *port)
{
	int err = bpf_map_update_elem(md->port_map_fd, portkey, port, 0);
	if (err) {
		TRN_LOG_ERROR("Store Port mapping failed (err:%d).", err);
		return 1;
	}
	return 0;
}

int trn_update_endpoint(struct user_metadata_t *md,
			struct endpoint_key_t *epkey, struct endpoint_t *ep)
{
	int err, idx;

	if (ep->hosted_iface != -1) {
		idx = get_unused_itf_index(md);

		if (idx == -1) {
			TRN_LOG_ERROR(
				"Failed to allocate an entry for interface map.");
			return 1;
		}

		err = bpf_map_update_elem(md->interfaces_map_fd, &idx,
					  &ep->hosted_iface, 0);

		if (err) {
			TRN_LOG_ERROR(
				"Failed to update interfaces map (err:%d).",
				err);
			return 1;
		}

		md->itf_idx[idx] = ep->hosted_iface;
		ep->hosted_iface = idx;
	}

	err = bpf_map_update_elem(md->endpoints_map_fd, epkey, ep, 0);

	if (err) {
		TRN_LOG_ERROR("Store endpoint mapping failed (err:%d).", err);
		return 1;
	}

	return 0;
}

int trn_update_vpc(struct user_metadata_t *md, struct vpc_key_t *vpckey,
		   struct vpc_t *vpc)
{
	int err = bpf_map_update_elem(md->vpc_map_fd, vpckey, vpc, 0);
	if (err) {
		TRN_LOG_ERROR("Store VPCs mapping failed (err:%d).", err);
		return 1;
	}
	return 0;
}

int trn_get_network(struct user_metadata_t *md, struct network_key_t *netkey,
		    struct network_t *net)
{
	netkey->prefixlen += 64; /* tunid size */
	int err = bpf_map_lookup_elem(md->networks_map_fd, netkey, net);
	if (err) {
		TRN_LOG_ERROR("Querying network mapping failed (err:%d).", err);
		return 1;
	}
	return 0;
}

int trn_get_endpoint(struct user_metadata_t *md, struct endpoint_key_t *epkey,
		     struct endpoint_t *ep)
{
	int err = bpf_map_lookup_elem(md->endpoints_map_fd, epkey, ep);
	if (err) {
		TRN_LOG_ERROR("Querying endpoint mapping failed (err:%d).",
			      err);
		return 1;
	}
	return 0;
}

static int _trn_set_inner_map(struct ebpf_prog_stage_t *stage,
			      struct bpf_map **map, const char *outer_map_name,
			      int inner_map_fd)
{
	int err;
	*map = bpf_object__find_map_by_name(stage->obj, outer_map_name);

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

	TRN_LOG_INFO("_trn_set_inner_map %s, fd:%d\n", outer_map_name,
		     inner_map_fd);
	return 0;
}

static int _trn_update_inner_map_fd(const char *outer_map_name,
				    struct bpf_map *outer_map,
				    int *outer_map_fd, int inner_map_fd)
{
	int pos = 0;
	*outer_map_fd = bpf_map__fd(outer_map);
	int err = bpf_map_update_elem(*outer_map_fd, &pos, &inner_map_fd, 0);
	TRN_LOG_INFO("_trn_update_inner_map_fd %s, outer_fd: %d, inner_fd:%d\n",
		     outer_map_name, *outer_map_fd, inner_map_fd);
	if (err) {
		TRN_LOG_ERROR(
			"Failed to update array map of maps outer_fd %d, inner_fd %d\n",
			*outer_map_fd, inner_map_fd);
		return 1;
	}

	return 0;
}

int trn_add_prog(struct user_metadata_t *md, unsigned int prog_idx,
		 const char *prog_path)
{
	struct ebpf_prog_stage_t *stage = &md->ebpf_progs[prog_idx];
	struct bpf_program *prog, *first_prog = NULL;
	int err;

	stage->obj = bpf_object__open(prog_path);

	if (IS_ERR_OR_NULL(stage->obj)) {
		TRN_LOG_ERROR("Error openning bpf file: %s\n", prog_path);
		return 1;
	}

	_SET_INNER_MAP(networks_map);
	_SET_INNER_MAP(vpc_map);
	_SET_INNER_MAP(endpoints_map);
	_SET_INNER_MAP(port_map);
	_SET_INNER_MAP(hosted_endpoints_iface_map);
	_SET_INNER_MAP(interface_config_map);
	_SET_INNER_MAP(interfaces_map);
	_SET_INNER_MAP(fwd_flow_mod_cache);
	_SET_INNER_MAP(rev_flow_mod_cache);
	_SET_INNER_MAP(ep_flow_host_cache);
	_SET_INNER_MAP(ep_host_cache);
	_SET_INNER_MAP(metrics_table_map);

	/* Only one prog is supported */
	bpf_object__for_each_program(prog, stage->obj)
	{
		bpf_program__set_xdp(prog);
		if (!first_prog)
			first_prog = prog;
	}

	bpf_object__load(stage->obj);

	if (!first_prog) {
		TRN_LOG_ERROR("Failed to find XDP program in object file\n");
		goto error;
	}
	stage->prog_fd = bpf_program__fd(first_prog);

	/* Now add the program to jump table */
	err = bpf_map_update_elem(md->jmp_table_fd, &prog_idx, &stage->prog_fd,
				  0);
	if (err) {
		TRN_LOG_ERROR("Error add prog to trn jmp table (err:%d).", err);
		goto error;
	}

	_UPDATE_INNER_MAP(networks_map);
	_UPDATE_INNER_MAP(vpc_map);
	_UPDATE_INNER_MAP(endpoints_map);
	_UPDATE_INNER_MAP(port_map);
	_UPDATE_INNER_MAP(hosted_endpoints_iface_map);
	_UPDATE_INNER_MAP(interface_config_map);
	_UPDATE_INNER_MAP(interfaces_map);
	_UPDATE_INNER_MAP(fwd_flow_mod_cache);
	_UPDATE_INNER_MAP(rev_flow_mod_cache);
	_UPDATE_INNER_MAP(ep_flow_host_cache);
	_UPDATE_INNER_MAP(ep_host_cache);
	_UPDATE_INNER_MAP(metrics_table_map);

	return 0;
error:
	TRN_LOG_ERROR("Error adding prog %s to stage.\n", prog_path);
	bpf_object__close(stage->obj);
	return 1;
}

int trn_remove_prog(struct user_metadata_t *md, unsigned int prog_idx)
{
	int err;
	err = bpf_map_delete_elem(md->jmp_table_fd, &prog_idx);
	if (err) {
		TRN_LOG_ERROR("Error add prog to trn jmp table (err:%d).", err);
	}
	bpf_object__close(md->ebpf_progs[prog_idx].obj);
	return 0;
}

int trn_get_vpc(struct user_metadata_t *md, struct vpc_key_t *vpckey,
		struct vpc_t *vpc)
{
	int err = bpf_map_lookup_elem(md->vpc_map_fd, vpckey, vpc);
	if (err) {
		TRN_LOG_ERROR("Querying vpc mapping failed (err:%d).", err);
		return 1;
	}
	return 0;
}

int trn_delete_network(struct user_metadata_t *md, struct network_key_t *netkey)
{
	netkey->prefixlen += 64; /* tunid size */
	int err = bpf_map_delete_elem(md->networks_map_fd, netkey);
	if (err) {
		TRN_LOG_ERROR("Deleting network mapping failed (err:%d).", err);
		return 1;
	}
	return 0;
}

int trn_delete_endpoint(struct user_metadata_t *md,
			struct endpoint_key_t *epkey)
{
	struct endpoint_t ep;

	int err = bpf_map_lookup_elem(md->endpoints_map_fd, epkey, &ep);

	if (err) {
		TRN_LOG_ERROR("Querying endpoint for delete failed (err:%d).",
			      err);
		return 1;
	}

	if (ep.hosted_iface != -1) {
		md->itf_idx[ep.hosted_iface] = TRAN_UNUSED_ITF_IDX;
	}

	err = bpf_map_delete_elem(md->endpoints_map_fd, epkey);
	if (err) {
		TRN_LOG_ERROR("Deleting endpoint mapping failed (err:%d).",
			      err);
		return 1;
	}

	return 0;
}

int trn_delete_vpc(struct user_metadata_t *md, struct vpc_key_t *vpckey)
{
	int err = bpf_map_delete_elem(md->vpc_map_fd, vpckey);
	if (err) {
		TRN_LOG_ERROR("Deleting vpc mapping failed (err:%d).", err);
		return 1;
	}
	return 0;
}

int trn_user_metadata_init(struct user_metadata_t *md, char *itf,
			   char *kern_path, int xdp_flags)
{
	int rc;
	struct rlimit r = { RLIM_INFINITY, RLIM_INFINITY };
	struct bpf_prog_load_attr prog_load_attr = { .prog_type =
							     BPF_PROG_TYPE_XDP,
						     .file = kern_path };
	__u32 info_len = sizeof(md->info);
	md->xdp_flags = xdp_flags;

	if (setrlimit(RLIMIT_MEMLOCK, &r)) {
		TRN_LOG_ERROR("setrlimit(RLIMIT_MEMLOCK)");
		return 1;
	}

	snprintf(md->pcapfile, sizeof(md->pcapfile),
		 "/sys/fs/bpf/%s_transit_pcap", itf);

	md->ifindex = if_nametoindex(itf);
	if (!md->ifindex) {
		TRN_LOG_ERROR("if_nametoindex");
		return 1;
	}

	md->eth.ip = trn_get_interface_ipv4(md->ifindex);
	md->eth.iface_index = md->ifindex;

	if (bpf_prog_load_xattr(&prog_load_attr, &md->obj, &md->prog_fd)) {
		TRN_LOG_ERROR("Error loading bpf: %s", kern_path);
		return 1;
	}

	rc = trn_bpf_maps_init(md);

	if (rc != 0) {
		return 1;
	}

	if (!md->prog_fd) {
		TRN_LOG_ERROR("load_bpf_file: %s.", strerror(errno));
		return 1;
	}

	if (bpf_set_link_xdp_fd(md->ifindex, md->prog_fd, md->xdp_flags) < 0) {
		TRN_LOG_ERROR("link set xdp fd failed - %s.", strerror(errno));
		return 1;
	}

	rc = bpf_obj_get_info_by_fd(md->prog_fd, &md->info, &info_len);
	if (rc != 0) {
		TRN_LOG_ERROR("can't get prog info - %s.", strerror(errno));
		return rc;
	}
	md->prog_id = md->info.id;

	int idx = get_unused_itf_index(md);

	if (idx == -1) {
		TRN_LOG_ERROR("Failed to allocate an entry for interface map.");
		return 1;
	}

	rc = bpf_map_update_elem(md->interfaces_map_fd, &idx,
				 &md->eth.iface_index, 0);

	if (rc != 0) {
		TRN_LOG_ERROR("Failed to update interfaces map with index: %d.",
			      md->eth.iface_index);
		return 1;
	}

	md->itf_idx[idx] = md->eth.iface_index;
	md->eth.iface_index = idx;

	int k = 0;

	rc = bpf_map_update_elem(md->interface_config_map_fd, &k, &md->eth, 0);
	if (rc != 0) {
		TRN_LOG_ERROR("Failed to store interface data.");
		return 1;
	}

	return 0;
}

uint32_t trn_get_interface_ipv4(int itf_idx)
{
	int fd;
	struct ifreq ifr;

	fd = socket(AF_INET, SOCK_DGRAM, 0);

	/* IPv4 IP address */
	ifr.ifr_addr.sa_family = AF_INET;

	if_indextoname(itf_idx, ifr.ifr_name);
	ioctl(fd, SIOCGIFADDR, &ifr);

	close(fd);

	return ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr.s_addr;
}
