// SPDX-License-Identifier: GPL-2.0-or-later
/**
 * @file test_dmn.c
 * @author Sherif Abdelwahab (@zasherif)
 *         Phu Tran          (@phudtran)
 *
 * @brief dmn unit tests
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
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <string.h>
#include <sys/resource.h>

#include "trn_log.h"
#include "dmn/trn_transitd.h"
#include "rpcgen/trn_rpc_protocol.h"

int __wrap_setrlimit(int resource, const struct rlimit *rlim)
{
	UNUSED(resource);
	UNUSED(rlim);
	return 0;
}

int __wrap_bpf_map_update_elem(void *map, void *key, void *value,
			       unsigned long long flags)
{
	UNUSED(map);
	UNUSED(key);
	UNUSED(value);
	UNUSED(flags);
	function_called();
	return 0;
}

int __wrap_bpf_map_lookup_elem(void *map, void *key, void *value)
{
	UNUSED(map);
	UNUSED(key);
	struct endpoint_t *endpoint = mock_ptr_type(struct endpoint_t *);
	struct network_t *network = mock_ptr_type(struct network_t *);
	struct vpc_t *vpc = mock_ptr_type(struct vpc_t *);
	struct agent_metadata_t *md = mock_ptr_type(struct agent_metadata_t *);
	function_called();
	if (endpoint != NULL)
		memcpy(value, endpoint, sizeof(*endpoint));
	else if (network != NULL)
		memcpy(value, network, sizeof(*network));
	else if (vpc != NULL)
		memcpy(value, vpc, sizeof(*vpc));
	else if (md != NULL)
		memcpy(value, md, sizeof(*md));
	else
		return 1;
	return 0;
}

int __wrap_bpf_map_delete_elem(void *map, void *key)
{
	UNUSED(map);
	UNUSED(key);

	bool valid = mock_type(bool);
	function_called();

	if (valid)
		return 0;
	return 1;
}

int __wrap_bpf_prog_load_xattr(const struct bpf_prog_load_attr *attr,
			       struct bpf_object **pobj, int *prog_fd)
{
	UNUSED(attr);
	UNUSED(pobj);

	*prog_fd = 1;
	return 0;
}

int __wrap_bpf_set_link_xdp_fd(int ifindex, int fd, __u32 flags)
{
	UNUSED(ifindex);
	UNUSED(fd);
	UNUSED(flags);
	return 0;
}

int __wrap_bpf_obj_get_info_by_fd(int prog_fd, void *info, __u32 *info_len)
{
	UNUSED(prog_fd);
	UNUSED(info);
	UNUSED(info_len);
	return 0;
}

struct bpf_map *__wrap_bpf_map__next(struct bpf_map *map,
				     struct bpf_object *obj)
{
	UNUSED(map);
	UNUSED(obj);
	return (struct bpf_map *)1;
}

int __wrap_bpf_map__fd(struct bpf_map *map)
{
	UNUSED(map);
	return 1;
}

int __wrap_bpf_map__pin(struct bpf_map *map, const char *path)
{
	UNUSED(map);
	UNUSED(path);
	return 0;
}

int __wrap_bpf_map__unpin(struct bpf_map *map, const char *path)
{
	UNUSED(map);
	UNUSED(path);
	return 0;
}

int __wrap_bpf_get_link_xdp_id(int ifindex, __u32 *prog_id, __u32 flags)
{
	UNUSED(ifindex);
	UNUSED(prog_id);
	UNUSED(flags);
	return 0;
}

unsigned int __wrap_if_nametoindex(const char *ifname)
{
	UNUSED(ifname);
	return 1;
}

const char *__wrap_if_indextoname(unsigned int ifindex, char *buf)
{
	UNUSED(buf);
	UNUSED(ifindex);
	if (ifindex != 1)
		return NULL;
	else
		return "lo";
}

struct bpf_object *__wrap_bpf_object__open(const char *path)
{
	UNUSED(path);
	return (struct bpf_object *)1;
}

int __wrap_bpf_create_map(enum bpf_map_type map_type, int key_size,
			  int value_size, int max_entries)
{
	UNUSED(map_type);
	UNUSED(key_size);
	UNUSED(value_size);
	UNUSED(max_entries);
	return 0;
}

int __wrap_bpf_program__fd(const struct bpf_program *prog)
{
	UNUSED(prog);
	return 1;
}

int __wrap_bpf_object__load(struct bpf_object *obj)
{
	UNUSED(obj);
	return 0;
}

struct bpf_map *
__wrap_bpf_object__find_map_by_name(const struct bpf_object *obj,
				    const char *name)
{
	UNUSED(obj);
	UNUSED(name);
	return (struct bpf_map *)1;
}
int __wrap_bpf_map__set_inner_map_fd(struct bpf_map *map, int fd)
{
	UNUSED(map);
	UNUSED(fd);
	return 0;
}

int __wrap_bpf_program__set_xdp(struct bpf_program *prog)
{
	UNUSED(prog);
	return 0;
}

struct bpf_program *__wrap_bpf_program__next(struct bpf_program *prev,
					     const struct bpf_object *obj)
{
	UNUSED(obj);
	if (prev == NULL) {
		return (struct bpf_program *)1;
	} else
		return NULL;
}

void __wrap_bpf_object__close(struct bpf_object *object)
{
	UNUSED(object);
	return;
}
static inline int cmpfunc(const void *a, const void *b)
{
	return (*(int *)a - *(int *)b);
}

static int check_vpc_equal(struct rpc_trn_vpc_t *vpc,
			   struct rpc_trn_vpc_t *c_vpc)
{
	u_int i;

	if (strcmp(vpc->interface, c_vpc->interface) != 0) {
		return false;
	}

	if (vpc->tunid != c_vpc->tunid) {
		return false;
	}

	if (vpc->routers_ips.routers_ips_len !=
	    c_vpc->routers_ips.routers_ips_len) {
		return false;
	}

	qsort(vpc->routers_ips.routers_ips_val,
	      vpc->routers_ips.routers_ips_len, sizeof(uint32_t), cmpfunc);

	qsort(c_vpc->routers_ips.routers_ips_val,
	      c_vpc->routers_ips.routers_ips_len, sizeof(uint32_t), cmpfunc);

	for (i = 0; i < vpc->routers_ips.routers_ips_len; i++) {
		if (c_vpc->routers_ips.routers_ips_val[i] !=
		    vpc->routers_ips.routers_ips_val[i])
			return false;
	}
	return true;
}

static int check_net_equal(rpc_trn_network_t *net, rpc_trn_network_t *c_net)
{
	u_int i;

	if (strcmp(net->interface, c_net->interface) != 0) {
		return false;
	}

	if (net->tunid != c_net->tunid) {
		return false;
	}

	if (net->prefixlen != c_net->prefixlen) {
		return false;
	}

	if (net->netip != c_net->netip) {
		return false;
	}

	if (net->switches_ips.switches_ips_len !=
	    c_net->switches_ips.switches_ips_len) {
		return false;
	}

	qsort(net->switches_ips.switches_ips_val,
	      net->switches_ips.switches_ips_len, sizeof(uint32_t), cmpfunc);

	qsort(c_net->switches_ips.switches_ips_val,
	      c_net->switches_ips.switches_ips_len, sizeof(uint32_t), cmpfunc);

	for (i = 0; i < net->switches_ips.switches_ips_len; i++) {
		if (c_net->switches_ips.switches_ips_val[i] !=
		    net->switches_ips.switches_ips_val[i])
			return false;
	}
	return true;
}

static int check_ep_equal(rpc_trn_endpoint_t *ep, rpc_trn_endpoint_t *c_ep)
{
	u_int i;

	if (strcmp(ep->interface, c_ep->interface) != 0) {
		return false;
	}

	if (strcmp(ep->hosted_interface, c_ep->hosted_interface) != 0) {
		return false;
	}

	if (strcmp(ep->veth, c_ep->veth) != 0) {
		return false;
	}

	if (memcmp(ep->mac, c_ep->mac, sizeof(char) * 6) != 0) {
		return false;
	}

	if (ep->ip != c_ep->ip) {
		return false;
	}

	if (ep->eptype != c_ep->eptype) {
		return false;
	}

	if (ep->tunid != c_ep->tunid) {
		return false;
	}

	if (ep->remote_ips.remote_ips_len != c_ep->remote_ips.remote_ips_len) {
		return false;
	}

	if (ep->remote_ips.remote_ips_len == 0) {
		return true;
	}

	qsort(ep->remote_ips.remote_ips_val, ep->remote_ips.remote_ips_len,
	      sizeof(uint32_t), cmpfunc);

	qsort(c_ep->remote_ips.remote_ips_val, c_ep->remote_ips.remote_ips_len,
	      sizeof(uint32_t), cmpfunc);

	for (i = 0; i < ep->remote_ips.remote_ips_len; i++) {
		if (c_ep->remote_ips.remote_ips_val[i] !=
		    ep->remote_ips.remote_ips_val[i]) {
			return false;
		}
	}

	return true;
}

static int check_md_equal(struct rpc_trn_agent_metadata_t *md,
			  struct rpc_trn_agent_metadata_t *c_md)

{
	if (strcmp(md->interface, c_md->interface) != 0) {
		return false;
	}

	if (!check_ep_equal(&md->ep, &c_md->ep)) {
		return false;
	}

	if (!check_net_equal(&md->net, &c_md->net)) {
		return false;
	}

	if (strcmp(md->eth.interface, c_md->eth.interface) != 0) {
		return false;
	}

	if (md->eth.ip != c_md->eth.ip) {
		return false;
	}

	if (memcmp(md->eth.mac, c_md->eth.mac, sizeof(char) * 6) != 0) {
		return false;
	}

	return true;
}

static void do_lo_xdp_load(void)
{
	rpc_trn_xdp_intf_t xdp_intf;
	char itf[] = "lo";
	char xdp_path[] = "/path/to/xdp/object/file";
	char pcapfile[] = "/path/to/bpf/pinned/map";
	xdp_intf.interface = itf;
	xdp_intf.xdp_path = xdp_path;
	xdp_intf.pcapfile = pcapfile;

	int *rc;
	expect_function_call(__wrap_bpf_map_update_elem);
	expect_function_call(__wrap_bpf_map_update_elem);
	rc = load_transit_xdp_1_svc(&xdp_intf, NULL);
	assert_int_equal(*rc, 0);
}

static void do_lo_xdp_unload(void)
{
	int *rc;
	rpc_intf_t test_itf;
	char itf_str[] = "lo";
	test_itf.interface = itf_str;
	rc = unload_transit_xdp_1_svc(&test_itf, NULL);
	assert_int_equal(*rc, 0);
}

static void do_veth_agent_load(void)
{
	rpc_trn_xdp_intf_t xdp_intf;
	char itf[] = "veth";
	char xdp_path[] = "/path/to/xdp/object/file";
	char pcapfile[] = "/path/to/bpf/pinned/map";
	xdp_intf.interface = itf;
	xdp_intf.xdp_path = xdp_path;
	xdp_intf.pcapfile = pcapfile;

	int *rc;
	rc = load_transit_agent_xdp_1_svc(&xdp_intf, NULL);
	assert_int_equal(*rc, 0);
}

static void do_veth_agent_unload(void)
{
	int *rc;
	rpc_intf_t test_itf;
	char itf_str[] = "veth";
	test_itf.interface = itf_str;
	rc = unload_transit_agent_xdp_1_svc(&test_itf, NULL);
	assert_int_equal(*rc, 0);
}

static void test_update_vpc_1_svc(void **state)
{
	UNUSED(state);

	char itf[] = "lo";
	uint32_t routers[] = { 0x100000a, 0x200000a };

	struct rpc_trn_vpc_t vpc1 = {
		.interface = itf,
		.tunid = 3,
		.routers_ips = { .routers_ips_len = 2,
				 .routers_ips_val = routers }

	};

	int *rc;
	expect_function_call(__wrap_bpf_map_update_elem);
	rc = update_vpc_1_svc(&vpc1, NULL);
	assert_int_equal(*rc, 0);
}

static void test_update_net_1_svc(void **state)
{
	UNUSED(state);

	char itf[] = "lo";
	uint32_t switches[] = { 0x100000a, 0x200000a };

	struct rpc_trn_network_t net1 = {
		.interface = itf,
		.prefixlen = 16,
		.tunid = 3,
		.netip = 0xa,
		.switches_ips = { .switches_ips_len = 2,
				  .switches_ips_val = switches }
	};

	int *rc;
	expect_function_call(__wrap_bpf_map_update_elem);
	rc = update_net_1_svc(&net1, NULL);
	assert_int_equal(*rc, 0);
}

static void test_update_ep_1_svc(void **state)
{
	UNUSED(state);

	char itf[] = "lo";
	char vitf[] = "veth0";
	char hosted_itf[] = "veth";
	uint32_t remote[] = { 0x200000a };
	char mac[6] = { 1, 2, 3, 4, 5, 6 };

	struct rpc_trn_endpoint_t ep1 = {
		.interface = itf,
		.ip = 0x100000a,
		.eptype = 1,
		.remote_ips = { .remote_ips_len = 1, .remote_ips_val = remote },
		.hosted_interface = hosted_itf,
		.veth = vitf,
		.tunid = 3,
	};

	memcpy(ep1.mac, mac, sizeof(char) * 6);

	int *rc;
	expect_function_calls(__wrap_bpf_map_update_elem, 2);
	rc = update_ep_1_svc(&ep1, NULL);
	assert_int_equal(*rc, 0);
}

static void test_update_agent_ep_1_svc(void **state)
{
	UNUSED(state);

	char itf[] = "veth";
	char vitf[] = "veth0";
	char hosted_itf[] = "";
	uint32_t remote[] = { 0x200000a };
	char mac[6] = { 1, 2, 3, 4, 5, 6 };

	struct rpc_trn_endpoint_t ep1 = {
		.interface = itf,
		.ip = 0x100000a,
		.eptype = 1,
		.remote_ips = { .remote_ips_len = 1, .remote_ips_val = remote },
		.hosted_interface = hosted_itf,
		.veth = vitf,
		.tunid = 3,
	};

	memcpy(ep1.mac, mac, sizeof(char) * 6);

	int *rc;
	expect_function_call(__wrap_bpf_map_update_elem);
	rc = update_agent_ep_1_svc(&ep1, NULL);
	assert_int_equal(*rc, 0);
}

static void test_update_packet_metadata_1_svc(void **state)
{
	UNUSED(state);

	char itf[] = "veth";
	char vitf[] = "veth0";
	char hosted_itf[] = "";
	uint32_t remote[] = { 0x200000a };
	char mac[6] = { 1, 2, 3, 4, 5, 6 };

	struct rpc_trn_packet_metadata_t packet_metadata1 = {
		.interface = itf,
		.ip = 0x100000a,
		.tunid = 3,
		.pod_label_value = 11,
		.namespace_label_value = 1,
	};

	int *rc;
	expect_function_call(__wrap_bpf_map_update_elem);
	rc = update_packet_metadata_1_svc(&packet_metadata1, NULL);
	assert_int_equal(*rc, 0);
}

static void test_update_agent_md_1_svc(void **state)
{
	UNUSED(state);

	char itf[] = "veth";
	char vitf[] = "veth0";
	char hosted_itf[] = "lo";
	uint32_t remote[] = { 0x200000a };
	uint32_t switches[] = { 0x100000a, 0x200000a };
	char mac[6] = { 1, 2, 3, 4, 5, 6 };
	char mac_eth[6] = { 6, 5, 4, 3, 2, 1 };

	struct rpc_trn_agent_metadata_t md1 = {
		.interface = itf,
		.ep = { .interface = itf,
			.ip = 0x100000a,
			.eptype = 1,
			.hosted_interface = hosted_itf,
			.veth = vitf,
			.tunid = 3,
			.remote_ips = { .remote_ips_len = 1,
					.remote_ips_val = remote } },
		.net = { .interface = itf,
			 .prefixlen = 16,
			 .tunid = 3,
			 .netip = 0xa,
			 .switches_ips = { .switches_ips_len = 2,
					   .switches_ips_val = switches } },
		.eth = { .interface = itf, .ip = 0x100000a }
	};

	memcpy(md1.ep.mac, mac, sizeof(char) * 6);
	memcpy(md1.eth.mac, mac_eth, sizeof(char) * 6);

	int *rc;
	expect_function_calls(__wrap_bpf_map_update_elem, 7);
	rc = update_agent_md_1_svc(&md1, NULL);
	assert_int_equal(*rc, 0);

	UNUSED(md1);
}

static void test_update_transit_network_policy_1_svc(void **state)
{
	UNUSED(state);
	char itf[] = "lo";

	struct rpc_trn_vsip_cidr_t policies[2] = {{
		.interface = itf,
		.tunid = 3,
		.local_ip = 0x100000a,
		.cidr_prefixlen = 16,
		.cidr_ip = 0xac00012,
		.cidr_type = 1,
		.bit_val = 4
	},
	{
		.interface = itf,
		.tunid = 3,
		.local_ip = 0x100000a,
		.cidr_prefixlen = 16,
		.cidr_ip = 0xac00012,
		.cidr_type = 2,
		.bit_val = 4
	}};

	int *rc;
	expect_function_call(__wrap_bpf_map_update_elem);
	rc = update_transit_network_policy_1_svc(policies, NULL);
	assert_int_equal(*rc, 0);
}

static void test_update_agent_network_policy_1_svc(void **state)
{
	UNUSED(state);
	char itf[] = "lo";

	struct rpc_trn_vsip_cidr_t policies[2] = {{
		.interface = itf,
		.tunid = 3,
		.local_ip = 0x100000a,
		.cidr_prefixlen = 16,
		.cidr_ip = 0xac00012,
		.cidr_type = 1,
		.bit_val = 4
	},
	{
		.interface = itf,
		.tunid = 3,
		.local_ip = 0x100000a,
		.cidr_prefixlen = 16,
		.cidr_ip = 0xac00012,
		.cidr_type = 2,
		.bit_val = 4
	}};

	int *rc;
	expect_function_call(__wrap_bpf_map_update_elem);
	rc = update_agent_network_policy_1_svc(policies, NULL);
	assert_int_equal(*rc, 0);
}

static void test_delete_transit_network_policy_1_svc(void **state)
{
	UNUSED(state);
	char itf[] = "lo";
	struct rpc_trn_vsip_cidr_key_t policy_keys[2] = {{
		.interface = itf,
		.tunid = 3,
		.local_ip = 0x100000a,
		.cidr_prefixlen = 16,
		.cidr_ip = 0xac00012,
		.cidr_type = 1
	},
	{
		.interface = itf,
		.tunid = 1,
		.local_ip = 0x900000a,
		.cidr_prefixlen = 16,
		.cidr_ip = 0xac00012,
		.cidr_type = 1
	}};
	int *rc;

	/* Test delete_transit_network_policy_1 with valid vp_ckey */
	will_return(__wrap_bpf_map_delete_elem, TRUE);
	expect_function_call(__wrap_bpf_map_delete_elem);
	rc = delete_transit_network_policy_1_svc(policy_keys, NULL);
	assert_int_equal(*rc, 0);

	/* Test delete_transit_network_policy_1 with invalid vpc_key */
	will_return(__wrap_bpf_map_delete_elem, FALSE);
	expect_function_call(__wrap_bpf_map_delete_elem);
	rc = delete_transit_network_policy_1_svc(policy_keys, NULL);
	assert_int_equal(*rc, RPC_TRN_FATAL);

	/* Test delete_transit_network_policy_1 with invalid interface*/
	policy_keys[0].interface = "";
	rc = delete_transit_network_policy_1_svc(policy_keys, NULL);
	assert_int_equal(*rc, RPC_TRN_ERROR);
}

static void test_delete_agent_network_policy_1_svc(void **state)
{
	UNUSED(state);
	char itf[] = "lo";
	struct rpc_trn_vsip_cidr_key_t policy_keys[2] = {{
		.interface = itf,
		.tunid = 3,
		.local_ip = 0x100000a,
		.cidr_prefixlen = 16,
		.cidr_ip = 0xac00012,
		.cidr_type = 1
	},
	{
		.interface = itf,
		.tunid = 1,
		.local_ip = 0x900000a,
		.cidr_prefixlen = 16,
		.cidr_ip = 0xac00012,
		.cidr_type = 1
	}};
	int *rc;

	/* Test delete_transit_network_policy_1 with valid vp_ckey */
	will_return(__wrap_bpf_map_delete_elem, TRUE);
	expect_function_call(__wrap_bpf_map_delete_elem);
	rc = delete_agent_network_policy_1_svc(policy_keys, NULL);
	assert_int_equal(*rc, 0);

	/* Test delete_transit_network_policy_1 with invalid vpc_key */
	will_return(__wrap_bpf_map_delete_elem, FALSE);
	expect_function_call(__wrap_bpf_map_delete_elem);
	rc = delete_agent_network_policy_1_svc(policy_keys, NULL);
	assert_int_equal(*rc, RPC_TRN_FATAL);

	/* Test delete_transit_network_policy_1 with invalid interface*/
	policy_keys[0].interface = "";
	rc = delete_agent_network_policy_1_svc(policy_keys, NULL);
	assert_int_equal(*rc, RPC_TRN_ERROR);
}

static void test_update_transit_network_policy_enforcement_1_svc(void **state)
{
	UNUSED(state);
	char itf[] = "lo";

	struct rpc_trn_vsip_enforce_t enforce1 = {
		.interface = itf,
		.tunid = 3,
		.local_ip = 0x100000a
	};

	int *rc;
	expect_function_call(__wrap_bpf_map_update_elem);
	rc = update_transit_network_policy_enforcement_1_svc(&enforce1, NULL);
	assert_int_equal(*rc, 0);
}

static void test_update_agent_network_policy_enforcement_1_svc(void **state)
{
	UNUSED(state);
	char itf[] = "lo";

	struct rpc_trn_vsip_enforce_t enforce1 = {
		.interface = itf,
		.tunid = 3,
		.local_ip = 0x100000a
	};

	int *rc;
	expect_function_call(__wrap_bpf_map_update_elem);
	rc = update_agent_network_policy_enforcement_1_svc(&enforce1, NULL);
	assert_int_equal(*rc, 0);
}

static void test_delete_transit_network_policy_enforcement_1_svc(void **state)
{
	UNUSED(state);
	char itf[] = "lo";

	struct rpc_trn_vsip_enforce_t enforce_key = {
		.interface = itf,
		.tunid = 3,
		.local_ip = 0x100000a
	};

	int *rc;

	/* Test delete_transit_network_policy_enforcement_1 with valid enforce_key */
	will_return(__wrap_bpf_map_delete_elem, TRUE);
	expect_function_call(__wrap_bpf_map_delete_elem);
	rc = delete_transit_network_policy_enforcement_1_svc(&enforce_key, NULL);
	assert_int_equal(*rc, 0);

	/* Test delete_transit_network_policy_enforcement_1 with invalid enforce_key */
	will_return(__wrap_bpf_map_delete_elem, FALSE);
	expect_function_call(__wrap_bpf_map_delete_elem);
	rc = delete_transit_network_policy_enforcement_1_svc(&enforce_key, NULL);
	assert_int_equal(*rc, RPC_TRN_FATAL);

	/* Test delete_transit_network_policy_enforcement_1 with invalid interface*/
	enforce_key.interface = "";
	rc = delete_transit_network_policy_enforcement_1_svc(&enforce_key, NULL);
	assert_int_equal(*rc, RPC_TRN_ERROR);
}

static void test_delete_agent_network_policy_enforcement_1_svc(void **state)
{
	UNUSED(state);
	char itf[] = "lo";

	struct rpc_trn_vsip_enforce_t enforce_key = {
		.interface = itf,
		.tunid = 3,
		.local_ip = 0x100000a
	};

	int *rc;

	/* Test delete_agent_network_policy_enforcement_1 with valid enforce_key */
	will_return(__wrap_bpf_map_delete_elem, TRUE);
	expect_function_call(__wrap_bpf_map_delete_elem);
	rc = delete_agent_network_policy_enforcement_1_svc(&enforce_key, NULL);
	assert_int_equal(*rc, 0);

	/* Test delete_agent_network_policy_enforcement_1 with invalid enforce_key */
	will_return(__wrap_bpf_map_delete_elem, FALSE);
	expect_function_call(__wrap_bpf_map_delete_elem);
	rc = delete_agent_network_policy_enforcement_1_svc(&enforce_key, NULL);
	assert_int_equal(*rc, RPC_TRN_FATAL);

	/* Test delete_agent_network_policy_enforcement_1 with invalid interface*/
	enforce_key.interface = "";
	rc = delete_agent_network_policy_enforcement_1_svc(&enforce_key, NULL);
	assert_int_equal(*rc, RPC_TRN_ERROR);
}

static void test_update_transit_network_policy_protocol_port_1_svc(void **state)
{
	UNUSED(state);
	char itf[] = "lo";

	struct rpc_trn_vsip_ppo_t ppo1[2] = {{
		.interface = itf,
		.tunid = 3,
		.local_ip = 0x300000a,
		.proto = 6,
		.port = 60184,
		.bit_val = 10
	},
	{
		.interface = itf,
		.tunid = 3,
		.local_ip = 0x300000a,
		.proto = 6,
		.port = 60184,
		.bit_val = 10
	}};

	int *rc;
	expect_function_call(__wrap_bpf_map_update_elem);
	rc = update_transit_network_policy_protocol_port_1_svc(ppo1, NULL);
	assert_int_equal(*rc, 0);
}

static void test_update_agent_network_policy_protocol_port_1_svc(void **state)
{
	UNUSED(state);
	char itf[] = "lo";

	struct rpc_trn_vsip_ppo_t ppo1[2] = {{
		.interface = itf,
		.tunid = 3,
		.local_ip = 0x300000a,
		.proto = 6,
		.port = 60184,
		.bit_val = 10
	},
	{
		.interface = itf,
		.tunid = 3,
		.local_ip = 0x300000a,
		.proto = 6,
		.port = 60184,
		.bit_val = 10
	}};

	int *rc;
	expect_function_call(__wrap_bpf_map_update_elem);
	rc = update_agent_network_policy_protocol_port_1_svc(ppo1, NULL);
	assert_int_equal(*rc, 0);
}

static void test_delete_transit_network_policy_protocol_port_1_svc(void **state)
{
	UNUSED(state);
	char itf[] = "lo";

	struct rpc_trn_vsip_ppo_key_t ppo_keys[2] = {{
		.interface = itf,
		.tunid = 3,
		.local_ip = 0x300000a,
		.proto = 6,
		.port = 60184
	},
	{
		.interface = itf,
		.tunid = 2,
		.local_ip = 0x200000a,
		.proto = 6,
		.port = 60184
	}};

	int *rc;
	/* Test delete_transit_network_policy_protocol_port_1 with valid ppo_keys */
	will_return(__wrap_bpf_map_delete_elem, TRUE);
	expect_function_call(__wrap_bpf_map_delete_elem);
	rc = delete_transit_network_policy_protocol_port_1_svc(ppo_keys, NULL);
	assert_int_equal(*rc, 0);

	/* Test delete_transit_network_policy_protocol_port_1 with invalid ppo_keys */
	will_return(__wrap_bpf_map_delete_elem, FALSE);
	expect_function_call(__wrap_bpf_map_delete_elem);
	rc = delete_transit_network_policy_protocol_port_1_svc(ppo_keys, NULL);
	assert_int_equal(*rc, RPC_TRN_FATAL);

	/* Test delete_transit_network_policy_protocol_port_1 with invalid interface*/
	ppo_keys[0].interface = "";
	ppo_keys[1].interface = "";
	rc = delete_transit_network_policy_protocol_port_1_svc(ppo_keys, NULL);
	assert_int_equal(*rc, RPC_TRN_ERROR);
}

static void test_delete_agent_network_policy_protocol_port_1_svc(void **state)
{
	UNUSED(state);
	char itf[] = "lo";

	struct rpc_trn_vsip_ppo_key_t ppo_keys[2] = {{
		.interface = itf,
		.tunid = 3,
		.local_ip = 0x300000a,
		.proto = 6,
		.port = 60184
	},
	{
		.interface = itf,
		.tunid = 2,
		.local_ip = 0x200000a,
		.proto = 6,
		.port = 60184
	}};

	int *rc;
	/* Test delete_agent_network_policy_protocol_port_1 with valid ppo_keys */
	will_return(__wrap_bpf_map_delete_elem, TRUE);
	expect_function_call(__wrap_bpf_map_delete_elem);
	rc = delete_agent_network_policy_protocol_port_1_svc(ppo_keys, NULL);
	assert_int_equal(*rc, 0);

	/* Test delete_agent_network_policy_protocol_port_1 with invalid ppo_keys */
	will_return(__wrap_bpf_map_delete_elem, FALSE);
	expect_function_call(__wrap_bpf_map_delete_elem);
	rc = delete_agent_network_policy_protocol_port_1_svc(ppo_keys, NULL);
	assert_int_equal(*rc, RPC_TRN_FATAL);

	/* Test delete_agent_network_policy_protocol_port_1 with invalid interface*/
	ppo_keys[0].interface = "";
	ppo_keys[1].interface = "";
	rc = delete_agent_network_policy_protocol_port_1_svc(ppo_keys, NULL);
	assert_int_equal(*rc, RPC_TRN_ERROR);
}

static void test_get_vpc_1_svc(void **state)
{
	UNUSED(state);

	char itf[] = "lo";
	uint32_t routers[] = { 0x100000a, 0x200000a };
	struct rpc_trn_vpc_key_t vpc_key1 = { .interface = itf, .tunid = 3 };
	struct rpc_trn_vpc_t vpc1 = {
		.interface = itf,
		.tunid = 3,
		.routers_ips = { .routers_ips_len = 2,
				 .routers_ips_val = routers }

	};

	struct vpc_t vpc_val;
	vpc_val.nrouters = 2;
	vpc_val.routers_ips[0] = routers[0];
	vpc_val.routers_ips[1] = routers[1];
	struct rpc_trn_vpc_t *retval;

	/* Test get_vpc with valid vpc_key */
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	will_return(__wrap_bpf_map_lookup_elem, &vpc_val);
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	expect_function_call(__wrap_bpf_map_lookup_elem);
	retval = get_vpc_1_svc(&vpc_key1, NULL);
	assert_true(check_vpc_equal(retval, &vpc1));

	/* Test get_vpc with bad return code from bpf_lookup */
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	expect_function_call(__wrap_bpf_map_lookup_elem);
	retval = get_vpc_1_svc(&vpc_key1, NULL);
	assert_false(check_vpc_equal(retval, &vpc1));

	/* Test get_vpc with invalid interface*/
	vpc_key1.interface = "";
	retval = get_vpc_1_svc(&vpc_key1, NULL);
	assert_true(strlen(retval->interface) == 0);
}

static void test_get_net_1_svc(void **state)
{
	UNUSED(state);

	char itf[] = "lo";
	uint32_t switches[] = { 0x100000a, 0x200000a };
	struct rpc_trn_network_t net1 = {
		.interface = itf,
		.prefixlen = 16,
		.tunid = 3,
		.netip = 0xa,
		.switches_ips = { .switches_ips_len = 2,
				  .switches_ips_val = switches }
	};
	struct rpc_trn_network_key_t net_key1 = {
		.interface = itf,
		.prefixlen = 16,
		.tunid = 3,
		.netip = 0xa,
	};

	struct network_t net_val;
	net_val.prefixlen = 16;
	net_val.nswitches = 2;
	memcpy(&net_val.nip, &net1.tunid, sizeof(net1.tunid));
	net_val.nip[2] = net1.netip;
	net_val.switches_ips[0] = switches[0];
	net_val.switches_ips[1] = switches[1];

	/* Test get_net with valid net_key */
	struct rpc_trn_network_t *retval;
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	will_return(__wrap_bpf_map_lookup_elem, &net_val);
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	expect_function_call(__wrap_bpf_map_lookup_elem);
	retval = get_net_1_svc(&net_key1, NULL);
	assert_true(check_net_equal(retval, &net1));

	/* Test get_net with bad return code from bpf_lookup */
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	expect_function_call(__wrap_bpf_map_lookup_elem);
	retval = get_net_1_svc(&net_key1, NULL);
	assert_false(check_net_equal(retval, &net1));

	/* Test get_net with invalid interface*/
	net_key1.interface = "";
	retval = get_net_1_svc(&net_key1, NULL);
	assert_true(strlen(retval->interface) == 0);
}

static void test_get_ep_1_svc(void **state)
{
	UNUSED(state);

	char itf[] = "lo";
	char vitf[] = "";
	char hosted_itf[] = "lo";
	uint32_t remote[] = { 0x200000a };
	char mac[6] = { 1, 2, 3, 4, 5, 6 };

	struct rpc_trn_endpoint_t ep1 = {
		.interface = itf,
		.ip = 0x100000a,
		.eptype = 1,
		.remote_ips = { .remote_ips_len = 1, .remote_ips_val = remote },
		.hosted_interface = hosted_itf,
		.veth = vitf,
		.tunid = 3,
	};

	struct rpc_trn_endpoint_key_t ep_key1 = {
		.interface = itf,
		.ip = 0x100000a,
		.tunid = 3,
	};

	struct endpoint_t ep_val;
	ep_val.eptype = 1;
	ep_val.nremote_ips = 1;
	ep_val.remote_ips[0] = remote[0];
	ep_val.hosted_iface = 1;
	memcpy(ep_val.mac, ep1.mac, sizeof(ep1.mac));

	struct rpc_trn_endpoint_t ep2 = {
		.interface = itf,
		.ip = 0x100000a,
		.eptype = 0,
		.remote_ips = { .remote_ips_len = 1, .remote_ips_val = remote },
		.hosted_interface = "",
		.veth = vitf,
		.tunid = 0,
	};

	struct rpc_trn_endpoint_key_t ep_key2 = {
		.interface = itf,
		.ip = 0x100000a,
		.tunid = 0,
	};

	struct endpoint_t ep_val2;
	ep_val2.eptype = 0;
	ep_val2.nremote_ips = 1;
	ep_val2.remote_ips[0] = remote[0];
	ep_val2.hosted_iface = -1;
	memcpy(ep_val2.mac, ep2.mac, sizeof(ep2.mac));

	/* Test get_ep with valid ep_key */
	struct rpc_trn_endpoint_t *retval;
	will_return(__wrap_bpf_map_lookup_elem, &ep_val);
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	expect_function_call(__wrap_bpf_map_lookup_elem);
	retval = get_ep_1_svc(&ep_key1, NULL);
	assert_true(check_ep_equal(retval, &ep1));

	/* Test get_ep substrate with valid ep_key */
	will_return(__wrap_bpf_map_lookup_elem, &ep_val2);
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	expect_function_call(__wrap_bpf_map_lookup_elem);
	retval = get_ep_1_svc(&ep_key2, NULL);
	assert_true(check_ep_equal(retval, &ep2));

	/* Test get_ep with bad return code from bpf_lookup */
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	expect_function_call(__wrap_bpf_map_lookup_elem);
	retval = get_ep_1_svc(&ep_key1, NULL);
	assert_false(check_ep_equal(retval, &ep1));

	/* Test get_ep with invalid interface index*/
	ep_val.hosted_iface = 2;
	will_return(__wrap_bpf_map_lookup_elem, &ep_val);
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	expect_function_call(__wrap_bpf_map_lookup_elem);
	retval = get_ep_1_svc(&ep_key1, NULL);
	assert_false(check_ep_equal(retval, &ep1));

	/* Test get_ep with invalid interface*/
	ep_key1.interface = "";
	retval = get_ep_1_svc(&ep_key1, NULL);
	assert_true(strlen(retval->interface) == 0);
}

static void test_get_agent_ep_1_svc(void **state)
{
	UNUSED(state);

	char itf[] = "lo";
	char vitf[] = "";
	char hosted_itf[] = "lo";
	uint32_t remote[] = { 0x200000a };
	char mac[6] = { 1, 2, 3, 4, 5, 6 };

	struct rpc_trn_endpoint_t ep1 = {
		.interface = itf,
		.ip = 0x100000a,
		.eptype = 1,
		.remote_ips = { .remote_ips_len = 1, .remote_ips_val = remote },
		.hosted_interface = hosted_itf,
		.veth = vitf,
		.tunid = 3,
	};

	struct rpc_trn_endpoint_key_t ep_key1 = {
		.interface = itf,
		.ip = 0x100000a,
		.tunid = 3,
	};

	struct endpoint_t ep_val;
	ep_val.eptype = 1;
	ep_val.nremote_ips = 1;
	ep_val.remote_ips[0] = remote[0];
	ep_val.hosted_iface = 1;
	memcpy(ep_val.mac, ep1.mac, sizeof(ep1.mac));

	struct rpc_trn_endpoint_t ep2 = {
		.interface = itf,
		.ip = 0x100000a,
		.eptype = 0,
		.remote_ips = { .remote_ips_len = 1, .remote_ips_val = remote },
		.hosted_interface = "",
		.veth = vitf,
		.tunid = 0,
	};

	struct rpc_trn_endpoint_key_t ep_key2 = {
		.interface = itf,
		.ip = 0x100000a,
		.tunid = 0,
	};

	struct endpoint_t ep_val2;
	ep_val2.eptype = 0;
	ep_val2.nremote_ips = 1;
	ep_val2.remote_ips[0] = remote[0];
	ep_val2.hosted_iface = -1;
	memcpy(ep_val2.mac, ep2.mac, sizeof(ep2.mac));

	/* Test get_agent_ep with valid ep_key */
	struct rpc_trn_endpoint_t *retval;
	will_return(__wrap_bpf_map_lookup_elem, &ep_val);
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	expect_function_call(__wrap_bpf_map_lookup_elem);
	retval = get_agent_ep_1_svc(&ep_key1, NULL);
	assert_true(check_ep_equal(retval, &ep1));

	/* Test get_agent_ep substrate with valid ep_key */
	will_return(__wrap_bpf_map_lookup_elem, &ep_val2);
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	expect_function_call(__wrap_bpf_map_lookup_elem);
	retval = get_agent_ep_1_svc(&ep_key2, NULL);
	assert_true(check_ep_equal(retval, &ep2));

	/* Test get_agent_ep with bad return code from bpf_lookup */
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	expect_function_call(__wrap_bpf_map_lookup_elem);
	retval = get_agent_ep_1_svc(&ep_key1, NULL);
	assert_false(check_ep_equal(retval, &ep1));

	/* Test get_ep with invalid interface index*/
	ep_val.hosted_iface = 2;
	will_return(__wrap_bpf_map_lookup_elem, &ep_val);
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	expect_function_call(__wrap_bpf_map_lookup_elem);
	retval = get_agent_ep_1_svc(&ep_key1, NULL);
	assert_false(check_ep_equal(retval, &ep1));

	/* Test get_agent_ep with invalid interface*/
	ep_key1.interface = "";
	retval = get_agent_ep_1_svc(&ep_key1, NULL);
	assert_true(strlen(retval->interface) == 0);
}

static void test_get_agent_md_1_svc(void **state)
{
	UNUSED(state);

	char itf[] = "lo";
	char vitf[] = "";
	char hosted_itf[] = "lo";
	uint32_t remote[] = { 0x200000a };
	uint32_t switches[] = { 0x100000a, 0x200000a };
	char mac[6] = { 1, 2, 3, 4, 5, 6 };
	char mac_eth[6] = { 6, 5, 4, 3, 2, 1 };

	struct rpc_trn_agent_metadata_t md1 = {
		.interface = itf,
		.ep = { .interface = "",
			.ip = 0x100000a,
			.eptype = 1,
			.hosted_interface = hosted_itf,
			.veth = "",
			.tunid = 3,
			.remote_ips = { .remote_ips_len = 1,
					.remote_ips_val = remote } },
		.net = { .interface = "",
			 .prefixlen = 16,
			 .tunid = 3,
			 .netip = 0xa,
			 .switches_ips = { .switches_ips_len = 2,
					   .switches_ips_val = switches } },
		.eth = { .interface = itf, .ip = 0x100000a }
	};
	memcpy(md1.ep.mac, mac, sizeof(char) * 6);
	memcpy(md1.eth.mac, mac_eth, sizeof(char) * 6);

	struct agent_metadata_t md_val = {
		.eth = { .ip = 0x100000a, .iface_index = 1 },
		.nkey = { .prefixlen = 16 },
		.net = { .prefixlen = 16, .nswitches = 2 },
		.ep = { .eptype = 1, .nremote_ips = 1, .hosted_iface = 1 },
	};

	memcpy(md_val.eth.mac, mac_eth, sizeof(char) * 6);
	memcpy(&md_val.nkey.nip, &md1.net.tunid, sizeof(md1.net.tunid));
	md_val.nkey.nip[2] = md1.net.netip;
	memcpy(&md_val.net.nip, &md_val.nkey.nip, sizeof(md_val.nkey.nip));
	md_val.net.switches_ips[0] = switches[0];
	md_val.net.switches_ips[1] = switches[1];
	memcpy(&md_val.epkey.tunip, &md1.ep.tunid, sizeof(md1.ep.tunid));
	md_val.epkey.tunip[2] = md1.ep.ip;
	md_val.ep.remote_ips[0] = remote[0];
	memcpy(md_val.ep.mac, mac, sizeof(char) * 6);

	struct rpc_intf_t md_key1 = { .interface = itf };

	/* Test get_agent_md with valid md_key */
	struct rpc_trn_agent_metadata_t *retval;
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	will_return(__wrap_bpf_map_lookup_elem, &md_val);
	expect_function_call(__wrap_bpf_map_lookup_elem);
	retval = get_agent_md_1_svc(&md_key1, NULL);
	assert_true(check_md_equal(retval, &md1));

	/* Test get_agent_md with bad return code from bpf_lookup */
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	expect_function_call(__wrap_bpf_map_lookup_elem);
	retval = get_agent_md_1_svc(&md_key1, NULL);
	assert_false(check_md_equal(retval, &md1));

	/* Test get_agent_md with invalid eth hosted interface*/
	md_val.eth.iface_index = 2;
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	will_return(__wrap_bpf_map_lookup_elem, &md_val);
	expect_function_call(__wrap_bpf_map_lookup_elem);
	retval = get_agent_md_1_svc(&md_key1, NULL);
	assert_false(check_md_equal(retval, &md1));

	/* Test get_agent_md with invalid ep hosted interface*/
	md_val.eth.iface_index = 1;
	md_val.ep.hosted_iface = 2;
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	will_return(__wrap_bpf_map_lookup_elem, &md_val);
	expect_function_call(__wrap_bpf_map_lookup_elem);
	retval = get_agent_md_1_svc(&md_key1, NULL);
	assert_false(check_md_equal(retval, &md1));

	/* Test get_agent_md with invalid interface*/
	md_key1.interface = "";
	retval = get_agent_md_1_svc(&md_key1, NULL);
	assert_true(strlen(retval->interface) == 0);
}

static void test_delete_vpc_1_svc(void **state)
{
	UNUSED(state);
	char itf[] = "lo";
	struct rpc_trn_vpc_key_t vpc_key = { .interface = itf, .tunid = 3 };
	int *rc;

	/* Test delete_vpc_1 with valid vp_ckey */
	will_return(__wrap_bpf_map_delete_elem, TRUE);
	expect_function_call(__wrap_bpf_map_delete_elem);
	rc = delete_vpc_1_svc(&vpc_key, NULL);
	assert_int_equal(*rc, 0);

	/* Test delete_vpc_1 with invalid vpc_key */
	will_return(__wrap_bpf_map_delete_elem, FALSE);
	expect_function_call(__wrap_bpf_map_delete_elem);
	rc = delete_vpc_1_svc(&vpc_key, NULL);
	assert_int_equal(*rc, RPC_TRN_FATAL);

	/* Test delete_vpc_1 with invalid interface*/
	vpc_key.interface = "";
	rc = delete_vpc_1_svc(&vpc_key, NULL);
	assert_int_equal(*rc, RPC_TRN_ERROR);
}

static void test_delete_net_1_svc(void **state)
{
	UNUSED(state);
	char itf[] = "lo";
	struct rpc_trn_network_key_t net_key = {
		.interface = itf,
		.prefixlen = 16,
		.tunid = 3,
		.netip = 0xa,
	};
	int *rc;

	/* Test delete_net_1 with valid net_key */
	will_return(__wrap_bpf_map_delete_elem, TRUE);
	expect_function_call(__wrap_bpf_map_delete_elem);
	rc = delete_net_1_svc(&net_key, NULL);
	assert_int_equal(*rc, 0);

	/* Test delete_net_1 with invalid net_key */
	will_return(__wrap_bpf_map_delete_elem, FALSE);
	expect_function_call(__wrap_bpf_map_delete_elem);
	rc = delete_net_1_svc(&net_key, NULL);
	assert_int_equal(*rc, RPC_TRN_ERROR);

	/* Test delete_net_1 with invalid interface*/
	net_key.interface = "";
	rc = delete_net_1_svc(&net_key, NULL);
	assert_int_equal(*rc, RPC_TRN_ERROR);
}

static void test_delete_ep_1_svc(void **state)
{
	UNUSED(state);
	char itf[] = "lo";
	struct rpc_trn_endpoint_key_t ep_key = {
		.interface = itf,
		.ip = 0x100000a,
		.tunid = 3,
	};
	int *rc;

	uint32_t remote[] = { 0x200000a };
	char mac[6] = { 1, 2, 3, 4, 5, 6 };

	struct endpoint_t ep_val;
	ep_val.eptype = 1;
	ep_val.nremote_ips = 1;
	ep_val.remote_ips[0] = remote[0];
	ep_val.hosted_iface = 1;
	memcpy(ep_val.mac, mac, sizeof(mac));

	/* Test delete_ep_1 with valid ep_key */
	will_return(__wrap_bpf_map_lookup_elem, &ep_val);
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	will_return(__wrap_bpf_map_delete_elem, TRUE);
	expect_function_call(__wrap_bpf_map_lookup_elem);
	expect_function_call(__wrap_bpf_map_delete_elem);
	rc = delete_ep_1_svc(&ep_key, NULL);
	assert_int_equal(*rc, 0);

	/* Test delete_ep_1 with invalid ep_key */
	will_return(__wrap_bpf_map_lookup_elem, &ep_val);
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	will_return(__wrap_bpf_map_delete_elem, FALSE);
	expect_function_call(__wrap_bpf_map_lookup_elem);
	expect_function_call(__wrap_bpf_map_delete_elem);
	rc = delete_ep_1_svc(&ep_key, NULL);
	assert_int_equal(*rc, RPC_TRN_ERROR);

	/* Test delete_ep_1 with invalid interface*/
	ep_key.interface = "";
	rc = delete_ep_1_svc(&ep_key, NULL);
	assert_int_equal(*rc, RPC_TRN_ERROR);
}

static void test_delete_agent_ep_1_svc(void **state)
{
	UNUSED(state);
	char itf[] = "lo";
	struct rpc_trn_endpoint_key_t ep_key = {
		.interface = itf,
		.ip = 0x100000a,
		.tunid = 3,
	};
	int *rc;

	/* Test delete_agent_ep_1 with valid ep_key */
	will_return(__wrap_bpf_map_delete_elem, TRUE);
	expect_function_call(__wrap_bpf_map_delete_elem);
	rc = delete_agent_ep_1_svc(&ep_key, NULL);
	assert_int_equal(*rc, 0);

	/* Test delete_agent_ep_1 with invalid ep_key */
	will_return(__wrap_bpf_map_delete_elem, FALSE);
	expect_function_call(__wrap_bpf_map_delete_elem);
	rc = delete_agent_ep_1_svc(&ep_key, NULL);
	assert_int_equal(*rc, RPC_TRN_ERROR);

	/* Test delete_agent_ep_1 with invalid interface*/
	ep_key.interface = "";
	rc = delete_agent_ep_1_svc(&ep_key, NULL);
	assert_int_equal(*rc, RPC_TRN_ERROR);
}

static void test_delete_packet_metadata_1_svc(void **state)
{
	UNUSED(state);
	char itf[] = "lo";
	struct rpc_trn_packet_metadata_key_t key = {
		.interface = itf,
		.ip = 0x100000a,
		.tunid = 3,
	};
	int *rc;

	/* Test delete_packet_metadata_1 with valid ep_key */
	will_return(__wrap_bpf_map_delete_elem, TRUE);
	expect_function_call(__wrap_bpf_map_delete_elem);
	rc = delete_packet_metadata_1_svc(&key, NULL);
	assert_int_equal(*rc, 0);

	/* Test delete_packet_metadata_1 with invalid ep_key */
	will_return(__wrap_bpf_map_delete_elem, FALSE);
	expect_function_call(__wrap_bpf_map_delete_elem);
	rc = delete_packet_metadata_1_svc(&key, NULL);
	assert_int_equal(*rc, RPC_TRN_ERROR);

	/* Test delete_packet_metadata_1 with invalid interface*/
	key.interface = "";
	rc = delete_packet_metadata_1_svc(&key, NULL);
	assert_int_equal(*rc, RPC_TRN_ERROR);
}

static void test_delete_agent_md_1_svc(void **state)
{
	UNUSED(state);
	int *rc;
	char itf[] = "lo";
	struct rpc_intf_t md_key = { .interface = itf };

	/* Test delete_agent_md_1 with valid md_key */
	expect_function_calls(__wrap_bpf_map_update_elem, 6);
	rc = delete_agent_md_1_svc(&md_key, NULL);
	assert_int_equal(*rc, 0);

	/* Test delete_agent_md_1 with invalid interface*/
	md_key.interface = "";
	rc = delete_agent_md_1_svc(&md_key, NULL);
	assert_int_equal(*rc, RPC_TRN_ERROR);
}

/**
 * This is run once before all group tests
 */
static int groupSetup(void **state)
{
	UNUSED(state);
	TRN_LOG_INIT("transitd_unit");
	trn_itf_table_init();
	do_lo_xdp_load();
	do_veth_agent_load();
	return 0;
}

/**
 * This is run once after all group tests
 */
static int groupTeardown(void **state)
{
	UNUSED(state);
	do_lo_xdp_unload();
	do_veth_agent_unload();
	trn_itf_table_free();
	TRN_LOG_CLOSE();
	return 0;
}

int main()
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_update_vpc_1_svc),
		cmocka_unit_test(test_update_net_1_svc),
		cmocka_unit_test(test_update_ep_1_svc),
		cmocka_unit_test(test_update_agent_md_1_svc),
		cmocka_unit_test(test_update_agent_ep_1_svc),
		cmocka_unit_test(test_update_packet_metadata_1_svc),
		cmocka_unit_test(test_get_vpc_1_svc),
		cmocka_unit_test(test_get_net_1_svc),
		cmocka_unit_test(test_get_ep_1_svc),
		cmocka_unit_test(test_get_agent_ep_1_svc),
		cmocka_unit_test(test_get_agent_md_1_svc),
		cmocka_unit_test(test_delete_vpc_1_svc),
		cmocka_unit_test(test_delete_net_1_svc),
		cmocka_unit_test(test_delete_ep_1_svc),
		cmocka_unit_test(test_delete_agent_ep_1_svc),
		cmocka_unit_test(test_delete_packet_metadata_1_svc),
		cmocka_unit_test(test_delete_agent_md_1_svc),
		cmocka_unit_test(test_update_transit_network_policy_1_svc),
		cmocka_unit_test(test_update_agent_network_policy_1_svc),
		cmocka_unit_test(test_delete_transit_network_policy_1_svc),
		cmocka_unit_test(test_delete_agent_network_policy_1_svc),
		cmocka_unit_test(test_update_transit_network_policy_enforcement_1_svc),
		cmocka_unit_test(test_update_agent_network_policy_enforcement_1_svc),
		cmocka_unit_test(test_delete_transit_network_policy_enforcement_1_svc),
		cmocka_unit_test(test_delete_agent_network_policy_enforcement_1_svc),
		cmocka_unit_test(test_update_transit_network_policy_protocol_port_1_svc),
		cmocka_unit_test(test_update_agent_network_policy_protocol_port_1_svc),
		cmocka_unit_test(test_delete_transit_network_policy_protocol_port_1_svc),
		cmocka_unit_test(test_delete_agent_network_policy_protocol_port_1_svc)
	};

	int result = cmocka_run_group_tests(tests, groupSetup, groupTeardown);

	return result;
}
