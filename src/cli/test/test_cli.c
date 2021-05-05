// SPDX-License-Identifier: GPL-2.0-or-later
/**
 * @file test_cli.c
 * @author Sherif Abdelwahab (@zasherif)
 *         Phu Tran          (@phudtran)
 *
 * @brief cli unit tests
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
#include "cli/trn_cli.h"
#include "trn_log.h"
#include <stdio.h>
#include <stdbool.h>
#include <sys/resource.h>

static int clnt_perror_called = 0;

static int update_net_1_rc = 0;
static int *update_net_1_rc_ptr = &update_net_1_rc;

int __wrap_setrlimit(int resource, const struct rlimit *rlim)
{
	UNUSED(resource);
	UNUSED(rlim);
	return 0;
}

int *__wrap_update_vpc_1(rpc_trn_vpc_t *vpc, CLIENT *clnt)
{
	check_expected_ptr(vpc);
	check_expected_ptr(clnt);
	int *retval = mock_ptr_type(int *);
	function_called();
	return retval;
}

int *__wrap_update_net_1(rpc_trn_network_t *net, CLIENT *clnt)
{
	check_expected_ptr(net);
	check_expected_ptr(clnt);
	int *retval = mock_ptr_type(int *);
	function_called();
	return retval;
}

int *__wrap_update_ep_1(rpc_trn_endpoint_t *ep, CLIENT *clnt)
{
	check_expected_ptr(ep);
	check_expected_ptr(clnt);
	int *retval = mock_ptr_type(int *);
	function_called();
	return retval;
}

int *__wrap_load_transit_xdp_1(rpc_trn_xdp_intf_t *itf, CLIENT *clnt)
{
	UNUSED(itf);
	UNUSED(clnt);
	int *retval = mock_ptr_type(int *);
	function_called();
	return retval;
}

int *__wrap_unload_transit_xdp_1(rpc_trn_xdp_intf_t *itf, CLIENT *clnt)
{
	UNUSED(itf);
	UNUSED(clnt);
	int *retval = mock_ptr_type(int *);
	function_called();
	return retval;
}

int *__wrap_load_transit_agent_xdp_1(rpc_trn_xdp_intf_t *itf, CLIENT *clnt)
{
	UNUSED(itf);
	UNUSED(clnt);
	int *retval = mock_ptr_type(int *);
	function_called();
	return retval;
}

int *__wrap_unload_transit_agent_xdp_1(rpc_trn_xdp_intf_t *itf, CLIENT *clnt)
{
	UNUSED(itf);
	UNUSED(clnt);
	int *retval = mock_ptr_type(int *);
	function_called();
	return retval;
}

int *__wrap_update_agent_ep_1(rpc_trn_endpoint_t *ep, CLIENT *clnt)
{
	check_expected_ptr(ep);
	check_expected_ptr(clnt);
	int *retval = mock_ptr_type(int *);
	function_called();
	return retval;
}

int *__wrap_update_packet_metadata_1(rpc_trn_packet_metadata_t *packet_metadata, CLIENT *clnt)
{
	check_expected_ptr(packet_metadata);
	check_expected_ptr(clnt);
	int *retval = mock_ptr_type(int *);
	function_called();
	return retval;
}

int *__wrap_update_agent_md_1(rpc_trn_agent_metadata_t *md, CLIENT *clnt)
{
	check_expected_ptr(md);
	check_expected_ptr(clnt);
	int *retval = mock_ptr_type(int *);
	function_called();
	return retval;
}

int *__wrap_update_transit_network_policy_1(rpc_trn_vsip_cidr_t *policy, CLIENT *clnt)
{
	check_expected_ptr(policy);
	check_expected_ptr(clnt);
	int *retval = mock_ptr_type(int *);
	function_called();
	return retval;
}

int *__wrap_update_agent_network_policy_1(rpc_trn_vsip_cidr_t *policy, CLIENT *clnt)
{
	check_expected_ptr(policy);
	check_expected_ptr(clnt);
	int *retval = mock_ptr_type(int *);
	function_called();
	return retval;
}

int *__wrap_update_transit_network_policy_enforcement_1(rpc_trn_vsip_enforce_t *enforce, CLIENT *clnt)
{
	check_expected_ptr(enforce);
	check_expected_ptr(clnt);
	int *retval = mock_ptr_type(int *);
	function_called();
	return retval;
}

int *__wrap_update_agent_network_policy_enforcement_1(rpc_trn_vsip_enforce_t *enforce, CLIENT *clnt)
{
	check_expected_ptr(enforce);
	check_expected_ptr(clnt);
	int *retval = mock_ptr_type(int *);
	function_called();
	return retval;
}

int *__wrap_delete_transit_network_policy_enforcement_1(rpc_trn_vsip_enforce_t *enforce, CLIENT *clnt)
{
	check_expected_ptr(enforce);
	check_expected_ptr(clnt);
	int *retval = mock_ptr_type(int *);
	function_called();
	return retval;
}

int *__wrap_delete_agent_network_policy_enforcement_1(rpc_trn_vsip_enforce_t *enforce, CLIENT *clnt)
{
	check_expected_ptr(enforce);
	check_expected_ptr(clnt);
	int *retval = mock_ptr_type(int *);
	function_called();
	return retval;
}

int *__wrap_update_transit_network_policy_protocol_port_1(rpc_trn_vsip_ppo_t *ppo, CLIENT *clnt)
{
	check_expected_ptr(ppo);
	check_expected_ptr(clnt);
	int *retval = mock_ptr_type(int *);
	function_called();
	return retval;
}

int *__wrap_update_agent_network_policy_protocol_port_1(rpc_trn_vsip_ppo_t *ppo, CLIENT *clnt)
{
	check_expected_ptr(ppo);
	check_expected_ptr(clnt);
	int *retval = mock_ptr_type(int *);
	function_called();
	return retval;
}

int *__wrap_delete_transit_network_policy_protocol_port_1(rpc_trn_vsip_ppo_key_t *ppo, CLIENT *clnt)
{
	check_expected_ptr(ppo);
	check_expected_ptr(clnt);
	int *retval = mock_ptr_type(int *);
	function_called();
	return retval;
}

int *__wrap_delete_agent_network_policy_protocol_port_1(rpc_trn_vsip_ppo_key_t *ppo, CLIENT *clnt)
{
	check_expected_ptr(ppo);
	check_expected_ptr(clnt);
	int *retval = mock_ptr_type(int *);
	function_called();
	return retval;
}

rpc_trn_vpc_t *__wrap_get_vpc_1(rpc_trn_vpc_key_t *argp, CLIENT *clnt)
{
	check_expected_ptr(argp);
	check_expected_ptr(clnt);
	rpc_trn_vpc_t *retval = mock_ptr_type(rpc_trn_vpc_t *);
	function_called();
	return retval;
}

rpc_trn_network_t *__wrap_get_net_1(rpc_trn_network_key_t *argp, CLIENT *clnt)
{
	check_expected_ptr(argp);
	check_expected_ptr(clnt);
	rpc_trn_network_t *retval = mock_ptr_type(rpc_trn_network_t *);
	function_called();
	return retval;
}

rpc_trn_endpoint_t *__wrap_get_ep_1(rpc_trn_endpoint_key_t *argp, CLIENT *clnt)
{
	check_expected_ptr(argp);
	check_expected_ptr(clnt);
	rpc_trn_endpoint_t *retval = mock_ptr_type(rpc_trn_endpoint_t *);
	function_called();
	return retval;
}

rpc_trn_endpoint_t *__wrap_get_agent_ep_1(rpc_trn_endpoint_key_t *argp,
					  CLIENT *clnt)
{
	check_expected_ptr(argp);
	check_expected_ptr(clnt);
	rpc_trn_endpoint_t *retval = mock_ptr_type(rpc_trn_endpoint_t *);
	function_called();
	return retval;
}

rpc_trn_agent_metadata_t *__wrap_get_agent_md_1(rpc_intf_t *argp, CLIENT *clnt)
{
	check_expected_ptr(argp);
	check_expected_ptr(clnt);
	rpc_trn_agent_metadata_t *retval =
		mock_ptr_type(rpc_trn_agent_metadata_t *);
	function_called();
	return retval;
}

int *__wrap_delete_vpc_1(rpc_trn_vpc_key_t *argp, CLIENT *clnt)
{
	check_expected_ptr(argp);
	check_expected_ptr(clnt);
	int *retval = mock_ptr_type(int *);
	function_called();
	return retval;
}

int *__wrap_delete_net_1(rpc_trn_network_key_t *argp, CLIENT *clnt)
{
	check_expected_ptr(argp);
	check_expected_ptr(clnt);
	int *retval = mock_ptr_type(int *);
	function_called();
	return retval;
}

int *__wrap_delete_ep_1(rpc_trn_endpoint_key_t *argp, CLIENT *clnt)
{
	check_expected_ptr(argp);
	check_expected_ptr(clnt);
	int *retval = mock_ptr_type(int *);
	function_called();
	return retval;
}

int *__wrap_delete_agent_ep_1(rpc_trn_endpoint_key_t *argp, CLIENT *clnt)
{
	check_expected_ptr(argp);
	check_expected_ptr(clnt);
	int *retval = mock_ptr_type(int *);
	function_called();
	return retval;
}

int *__wrap_delete_packet_metadata_1(rpc_trn_packet_metadata_key_t *argp, CLIENT *clnt)
{
	check_expected_ptr(argp);
	check_expected_ptr(clnt);
	int *retval = mock_ptr_type(int *);
	function_called();
	return retval;
}

int *__wrap_delete_agent_md_1(rpc_intf_t *argp, CLIENT *clnt)
{
	check_expected_ptr(argp);
	check_expected_ptr(clnt);
	int *retval = mock_ptr_type(int *);
	function_called();
	return retval;
}

int *__wrap_delete_transit_network_policy_1(rpc_trn_vsip_cidr_key_t *policy, CLIENT *clnt)
{
	check_expected_ptr(policy);
	check_expected_ptr(clnt);
	int *retval = mock_ptr_type(int *);
	function_called();
	return retval;
}

int *__wrap_delete_agent_network_policy_1(rpc_trn_vsip_cidr_key_t *policy, CLIENT *clnt)
{
	check_expected_ptr(policy);
	check_expected_ptr(clnt);
	int *retval = mock_ptr_type(int *);
	function_called();
	return retval;
}

static inline int cmpfunc(const void *a, const void *b)
{
	return (*(int *)a - *(int *)b);
}

static int check_vpc_equal(const LargestIntegralType value,
			   const LargestIntegralType check_value_data)
{
	struct rpc_trn_vpc_t *vpc = (struct rpc_trn_vpc_t *)value;
	struct rpc_trn_vpc_t *c_vpc = (struct rpc_trn_vpc_t *)check_value_data;
	int i;

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

static int check_net_equal(const LargestIntegralType value,
			   const LargestIntegralType check_value_data)
{
	struct rpc_trn_network_t *net = (struct rpc_trn_network_t *)value;
	struct rpc_trn_network_t *c_net =
		(struct rpc_trn_network_t *)check_value_data;
	int i;

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

static int check_ep_equal(const LargestIntegralType value,
			  const LargestIntegralType check_value_data)
{
	struct rpc_trn_endpoint_t *ep = (struct rpc_trn_endpoint_t *)value;
	struct rpc_trn_endpoint_t *c_ep =
		(struct rpc_trn_endpoint_t *)check_value_data;
	int i;

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

static int check_packet_metadata_equal(const LargestIntegralType value,
			  const LargestIntegralType check_value_data)
{
	struct rpc_trn_packet_metadata_t *packet_metadata = (struct rpc_trn_packet_metadata_t *)value;
	struct rpc_trn_packet_metadata_t *c_packet_metadata =
		(struct rpc_trn_packet_metadata_t *)check_value_data;
	int i;

	if (strcmp(packet_metadata->interface, c_packet_metadata->interface) != 0) {
		return false;
	}

	if (packet_metadata->ip != c_packet_metadata->ip) {
		return false;
	}

	if (packet_metadata->tunid != c_packet_metadata->tunid) {
		return false;
	}

	if (packet_metadata->pod_label_value != c_packet_metadata->pod_label_value) {
		return false;
	}

	if (packet_metadata->namespace_label_value != c_packet_metadata->namespace_label_value) {
		return false;
	}

	return true;
}

static int check_md_equal(const LargestIntegralType value,
			  const LargestIntegralType check_value_data)

{
	struct rpc_trn_agent_metadata_t *md =
		(struct rpc_trn_agent_metadata_t *)value;
	struct rpc_trn_agent_metadata_t *c_md =
		(struct rpc_trn_agent_metadata_t *)check_value_data;

	if (!check_ep_equal((const LargestIntegralType)&md->ep,
			    (const LargestIntegralType)&c_md->ep)) {
		return false;
	}

	if (!check_net_equal((const LargestIntegralType)&md->net,
			     (const LargestIntegralType)&c_md->net)) {
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

static int check_vpc_key_equal(const LargestIntegralType value,
			       const LargestIntegralType check_value_data)
{
	struct rpc_trn_vpc_key_t *vpc_key = (struct rpc_trn_vpc_key_t *)value;
	struct rpc_trn_vpc_key_t *c_vpc_key =
		(struct rpc_trn_vpc_key_t *)check_value_data;

	if (strcmp(vpc_key->interface, c_vpc_key->interface) != 0) {
		return false;
	}

	if (vpc_key->tunid != c_vpc_key->tunid) {
		return false;
	}

	return true;
}

static int check_policy_key_equal(const LargestIntegralType value,
				  const LargestIntegralType check_value_data)
{
	struct rpc_trn_vsip_cidr_key_t *policy_key = (struct rpc_trn_vsip_cidr_key_t *)value;
	struct rpc_trn_vsip_cidr_key_t *c_policy_key =
		(struct rpc_trn_vsip_cidr_key_t *)check_value_data;

	if (strcmp(policy_key->interface, c_policy_key->interface) != 0) {
		return false;
	}

	if (policy_key->tunid != c_policy_key->tunid) {
		return false;
	}

	if (policy_key->local_ip != c_policy_key->local_ip) {
		return false;
	}

	if (policy_key->cidr_prefixlen != c_policy_key->cidr_prefixlen) {
		return false;
	}

	if (policy_key->cidr_ip != c_policy_key->cidr_ip) {
		return false;
	}

	if (policy_key->cidr_type != c_policy_key->cidr_type) {
		return false;
	}

	return true;
}

static int check_net_key_equal(const LargestIntegralType value,
			       const LargestIntegralType check_value_data)
{
	struct rpc_trn_network_key_t *net_key =
		(struct rpc_trn_network_key_t *)value;
	struct rpc_trn_network_key_t *c_net_key =
		(struct rpc_trn_network_key_t *)check_value_data;

	if (strcmp(net_key->interface, c_net_key->interface) != 0) {
		return false;
	}

	if (net_key->tunid != c_net_key->tunid) {
		return false;
	}

	if (net_key->prefixlen != c_net_key->prefixlen) {
		return false;
	}

	if (net_key->netip != c_net_key->netip) {
		return false;
	}

	return true;
}

static int check_ep_key_equal(const LargestIntegralType value,
			      const LargestIntegralType check_value_data)
{
	struct rpc_trn_endpoint_key_t *ep_key =
		(struct rpc_trn_endpoint_key_t *)value;
	struct rpc_trn_endpoint_key_t *c_ep_key =
		(struct rpc_trn_endpoint_key_t *)check_value_data;

	if (strcmp(ep_key->interface, c_ep_key->interface) != 0) {
		return false;
	}

	if (ep_key->tunid != c_ep_key->tunid) {
		return false;
	}

	if (ep_key->ip != c_ep_key->ip) {
		return false;
	}

	return true;
}

static int check_packet_metadata_key_equal(const LargestIntegralType value,
			      const LargestIntegralType check_value_data)
{
	struct rpc_trn_packet_metadata_key_t *key =
		(struct rpc_trn_packet_metadata_key_t *)value;
	struct rpc_trn_packet_metadata_key_t *c_key =
		(struct rpc_trn_packet_metadata_key_t *)check_value_data;

	if (strcmp(key->interface, c_key->interface) != 0) {
		return false;
	}

	if (key->tunid != c_key->tunid) {
		return false;
	}

	if (key->ip != c_key->ip) {
		return false;
	}

	return true;
}

static int check_md_itf_equal(const LargestIntegralType value,
			      const LargestIntegralType check_value_data)
{
	struct rpc_intf_t *md_itf = (struct rpc_intf_t *)value;
	struct rpc_intf_t *c_md_itf = (struct rpc_intf_t *)check_value_data;

	if (strcmp(md_itf->interface, c_md_itf->interface) != 0) {
		return false;
	}

	return true;
}

static int check_policy_equal(const LargestIntegralType value,
			      const LargestIntegralType check_value_data)
{
	struct rpc_trn_vsip_cidr_t *policy = (struct rpc_trn_vsip_cidr_t *)value;
	struct rpc_trn_vsip_cidr_t *c_policy =
		(struct rpc_trn_vsip_cidr_t *)check_value_data;

	if (strcmp(policy->interface, c_policy->interface) != 0) {
		return false;
	}

	if (policy->tunid != c_policy->tunid) {
		return false;
	}

	if (policy->local_ip != c_policy->local_ip) {
		return false;
	}

	if (policy->cidr_prefixlen != c_policy->cidr_prefixlen) {
		return false;
	}

	if (policy->cidr_ip != c_policy->cidr_ip) {
		return false;
	}

	if (policy->cidr_type != c_policy->cidr_type) {
		return false;
	}

	if (policy->bit_val != c_policy->bit_val) {
		return false;
	}

	return true;
}

static int check_policy_enforcement_equal(const LargestIntegralType value,
			      const LargestIntegralType check_value_data)
{
	struct rpc_trn_vsip_enforce_t *enforce = (struct rpc_trn_vsip_enforce_t *)value;
	struct rpc_trn_vsip_enforce_t *c_enforce =
		(struct rpc_trn_vsip_enforce_t *)check_value_data;

	if (strcmp(enforce->interface, c_enforce->interface) != 0) {
		return false;
	}

	if (enforce->tunid != c_enforce->tunid) {
		return false;
	}

	if (enforce->local_ip != c_enforce->local_ip) {
		return false;
	}

	return true;
}

static int check_policy_protocol_port_equal(const LargestIntegralType value,
					    const LargestIntegralType check_value_data)
{
	struct rpc_trn_vsip_ppo_t *policy = (struct rpc_trn_vsip_ppo_t *)value;
	struct rpc_trn_vsip_ppo_t *c_policy =
		(struct rpc_trn_vsip_ppo_t *)check_value_data;

	if (strcmp(policy->interface, c_policy->interface) != 0) {
		return false;
	}

	if (policy->tunid != c_policy->tunid) {
		return false;
	}

	if (policy->local_ip != c_policy->local_ip) {
		return false;
	}

	if (policy->proto != c_policy->proto) {
		return false;
	}

	if (policy->port != c_policy->port) {
		return false;
	}

	if (policy->bit_val != c_policy->bit_val) {
		return false;
	}

	return true;
}

static int check_policy_protocol_port_key_equal(const LargestIntegralType value,
						const LargestIntegralType check_value_data)
{
	struct rpc_trn_vsip_ppo_key_t *policy = (struct rpc_trn_vsip_ppo_key_t *)value;
	struct rpc_trn_vsip_ppo_key_t *c_policy =
		(struct rpc_trn_vsip_ppo_key_t *)check_value_data;

	if (strcmp(policy->interface, c_policy->interface) != 0) {
		return false;
	}

	if (policy->tunid != c_policy->tunid) {
		return false;
	}

	if (policy->local_ip != c_policy->local_ip) {
		return false;
	}

	if (policy->proto != c_policy->proto) {
		return false;
	}

	if (policy->port != c_policy->port) {
		return false;
	}

	return true;
}

static void test_trn_cli_update_vpc_subcmd(void **state)
{
	UNUSED(state);
	int rc;
	int argc = 5;
	int update_vpc_1_ret_val = 0;

	/* Test cases */
	char *argv1[] = { "update-vpc", "-i", "eth0", "-j", QUOTE({
				  "tunnel_id": "3",
				  "routers_ips": ["10.0.0.1", "10.0.0.2"]
			  }) };

	char *argv2[] = { "update-vpc", "-i", "eth0", "-j", QUOTE({
				  "tunnel_id": 3,
				  "routers_ips": ["10.0.0.1", "10.0.0.2"]
			  }) };

	char *argv3[] = { "update-vpc", "-i", "eth0", "-j", QUOTE({
				  "tunnel_id": "3",
				  "routers_ips": [10.0.0.1, "10.0.0.2"]
			  }) };

	char itf[] = "eth0";
	uint32_t routers[] = { 0x100000a, 0x200000a };

	struct rpc_trn_vpc_t exp_vpc = {
		.interface = itf,
		.tunid = 3,
		.routers_ips = { .routers_ips_len = 2,
				 .routers_ips_val = routers }

	};

	/* Test call update_vpc successfully */
	TEST_CASE("update_vpc succeed with well formed vpc json input");
	update_vpc_1_ret_val = 0;
	expect_function_call(__wrap_update_vpc_1);
	will_return(__wrap_update_vpc_1, &update_vpc_1_ret_val);
	expect_check(__wrap_update_vpc_1, vpc, check_vpc_equal, &exp_vpc);
	expect_any(__wrap_update_vpc_1, clnt);
	rc = trn_cli_update_vpc_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, 0);

	/* Test parse vpc input error*/
	TEST_CASE("update_vpc is not called with non-string field");
	rc = trn_cli_update_vpc_subcmd(NULL, argc, argv2);
	assert_int_equal(rc, -EINVAL);

	/* Test parse vpc input error 2*/
	TEST_CASE("update_vpc is not called malformed json");
	rc = trn_cli_update_vpc_subcmd(NULL, argc, argv3);
	assert_int_equal(rc, -EINVAL);

	/* Test call update_vpc_1 return error*/
	TEST_CASE("update-vpc subcommand fails if update_vpc_1 returns error");
	update_vpc_1_ret_val = -EINVAL;
	expect_function_call(__wrap_update_vpc_1);
	will_return(__wrap_update_vpc_1, &update_vpc_1_ret_val);
	expect_any(__wrap_update_vpc_1, vpc);
	expect_any(__wrap_update_vpc_1, clnt);
	rc = trn_cli_update_vpc_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);

	/* Test call update_vpc_1 return NULL*/
	TEST_CASE("update-vpc subcommand fails if update_vpc_1 returns NULl");
	expect_function_call(__wrap_update_vpc_1);
	will_return(__wrap_update_vpc_1, NULL);
	expect_any(__wrap_update_vpc_1, vpc);
	expect_any(__wrap_update_vpc_1, clnt);
	rc = trn_cli_update_vpc_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);
}

static void test_trn_cli_update_net_subcmd(void **state)
{
	UNUSED(state);
	int rc;
	int argc = 5;
	int update_net_1_ret_val = 0;

	/* Test cases */
	char *argv1[] = { "update-net", "-i", "eth0", "-j", QUOTE({
				  "tunnel_id": "3",
				  "nip": "10.0.0.0",
				  "prefixlen": "16",
				  "switches_ips": ["10.0.0.1", "10.0.0.2"]
			  }) };

	char *argv2[] = { "update-net", "-i", "eth0", "-j", QUOTE({
				  "tunnel_id": 3,
				  "nip": "10.0.0.0",
				  "prefixlen": "16",
				  "switches_ips": ["10.0.0.1", "10.0.0.2"]
			  }) };

	char *argv3[] = { "update-net", "-i", "eth0", "-j", QUOTE({
				  "tunnel_id": "3",
				  "nip": "adsfwef",
				  "prefixlen": "16",
				  "switches_ips": [10.0.0.1, "10.0.0.2"]
			  }) };

	char itf[] = "eth0";
	uint32_t switches[] = { 0x100000a, 0x200000a };

	struct rpc_trn_network_t exp_net = {
		.interface = itf,
		.prefixlen = 16,
		.tunid = 3,
		.netip = 0xa,
		.switches_ips = { .switches_ips_len = 2,
				  .switches_ips_val = switches }
	};

	/* Test call update_net successfully */
	TEST_CASE("update_net succeed with well formed network json input");
	update_net_1_ret_val = 0;
	expect_function_call(__wrap_update_net_1);
	will_return(__wrap_update_net_1, &update_net_1_ret_val);
	expect_check(__wrap_update_net_1, net, check_net_equal, &exp_net);
	expect_any(__wrap_update_net_1, clnt);
	rc = trn_cli_update_net_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, 0);

	/* Test parse net input error*/
	TEST_CASE("update_net is not called with non-string field");
	rc = trn_cli_update_net_subcmd(NULL, argc, argv2);
	assert_int_equal(rc, -EINVAL);

	/* Test parse net input error 2*/
	TEST_CASE("update_net is not called malformed json");
	rc = trn_cli_update_net_subcmd(NULL, argc, argv3);
	assert_int_equal(rc, -EINVAL);

	/* Test call update_net_1 return error*/
	TEST_CASE("update-net subcommand fails if update_net_1 returns error");
	update_net_1_ret_val = -EINVAL;
	expect_function_call(__wrap_update_net_1);
	will_return(__wrap_update_net_1, &update_net_1_ret_val);
	expect_any(__wrap_update_net_1, net);
	expect_any(__wrap_update_net_1, clnt);
	rc = trn_cli_update_net_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);

	/* Test call update_net_1 return NULL*/
	TEST_CASE("update-net subcommand fails if update_net_1 returns NULl");
	expect_function_call(__wrap_update_net_1);
	will_return(__wrap_update_net_1, NULL);
	expect_any(__wrap_update_net_1, net);
	expect_any(__wrap_update_net_1, clnt);
	rc = trn_cli_update_net_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);
}

static void test_trn_cli_update_ep_subcmd(void **state)
{
	UNUSED(state);
	int rc;
	int argc = 5;
	int update_ep_1_ret_val = 0;

	/* Test cases */
	char *argv1[] = { "update-ep", "-i", "eth0", "-j", QUOTE({
				  "tunnel_id": "3",
				  "ip": "10.0.0.1",
				  "eptype": "1",
				  "mac": "1:2:3:4:5:6",
				  "veth": "veth0",
				  "remote_ips": ["10.0.0.2"],
				  "hosted_iface": "peer"
			  }) };

	char *argv2[] = { "update-ep", "-i", "eth0", "-j", QUOTE({
				  "tunnel_id": 3,
				  "ip": "10.0.0.1",
				  "eptype": "1",
				  "mac": "1:2:3:4:5:6",
				  "veth": "veth0",
				  "remote_ips": ["10.0.0.2"],
				  "hosted_iface": "peer"
			  }) };

	char *argv3[] = { "update-ep", "-i", "eth0", "-j", QUOTE({
				  "tunnel_id": "3",
				  "mac": "1:2:3:4:5:6",
				  "veth": "veth0",
				  "remote_ips": ["10.0.0.2"],
				  "hosted_iface": "peer"
			  }) };

	char *argv4[] = { "update-ep", "-i", "eth0", "-j", QUOTE({
				  "tunnel_id": "3",
				  "mac": "1:2:3:4:5:6",
				  "veth": "veth0",
				  "remote_ips": [10.0.0.2],
				  "hosted_iface": "peer"
			  }) };

	char itf[] = "eth0";
	char vitf[] = "veth0";
	char hosted_itf[] = "peer";
	uint32_t remote[] = { 0x200000a };
	char mac[6] = { 1, 2, 3, 4, 5, 6 };

	struct rpc_trn_endpoint_t exp_ep = {
		.interface = itf,
		.ip = 0x100000a,
		.eptype = 1,
		.remote_ips = { .remote_ips_len = 1, .remote_ips_val = remote },
		.hosted_interface = hosted_itf,
		.veth = vitf,
		.tunid = 3,
	};

	memcpy(exp_ep.mac, mac, sizeof(char) * 6);

	/* Test call update_ep successfully */
	TEST_CASE("update_ep succeed with well formed endpoint json input");
	update_ep_1_ret_val = 0;
	expect_function_call(__wrap_update_ep_1);
	will_return(__wrap_update_ep_1, &update_ep_1_ret_val);
	expect_check(__wrap_update_ep_1, ep, check_ep_equal, &exp_ep);
	expect_any(__wrap_update_ep_1, clnt);
	rc = trn_cli_update_ep_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, 0);

	/* Test parse ep input error*/
	TEST_CASE("update_ep is not called with non-string field");
	rc = trn_cli_update_ep_subcmd(NULL, argc, argv2);
	assert_int_equal(rc, -EINVAL);

	TEST_CASE("update_ep is not called with missing required field");
	rc = trn_cli_update_ep_subcmd(NULL, argc, argv3);
	assert_int_equal(rc, -EINVAL);

	/* Test parse ep input error 2*/
	TEST_CASE("update_ep is not called malformed json");
	rc = trn_cli_update_ep_subcmd(NULL, argc, argv4);
	assert_int_equal(rc, -EINVAL);

	/* Test call update_ep_1 return error*/
	TEST_CASE("update-ep subcommand fails if update_ep_1 returns error");
	update_ep_1_ret_val = -EINVAL;
	expect_function_call(__wrap_update_ep_1);
	will_return(__wrap_update_ep_1, &update_ep_1_ret_val);
	expect_any(__wrap_update_ep_1, ep);
	expect_any(__wrap_update_ep_1, clnt);
	rc = trn_cli_update_ep_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);

	/* Test call update_ep_1 return NULL*/
	TEST_CASE("update-ep subcommand fails if update_ep_1 returns NULl");
	expect_function_call(__wrap_update_ep_1);
	will_return(__wrap_update_ep_1, NULL);
	expect_any(__wrap_update_ep_1, ep);
	expect_any(__wrap_update_ep_1, clnt);
	rc = trn_cli_update_ep_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);
}

static void test_trn_cli_load_transit_subcmd(void **state)
{
	UNUSED(state);
	int rc;
	int argc = 5;
	int load_transit_xdp_ret_val = 0;

	/* Test cases */
	char *argv1[] = { "load-transit-xdp", "-i", "eth0", "-j", QUOTE({
				  "xdp_path": "/path/to/xdp/object/file",
				  "pcapfile": "/path/to/bpf/pinned/map"
			  }) };

	char *argv2[] = { "load-transit-xdp", "-i", "eth0", "-j",
			  QUOTE({ "pcapfile": "/path/to/bpf/pinned/map" }) };

	char *argv3[] = { "load-transit-xdp", "-i", "eth0", "-j",
			  QUOTE({ "xdp_path": "/path/to/xdp/object/file" }) };

	/* Test call load_transit_xdp_1 successfully */
	TEST_CASE("load_transit_xdp succeed with well formed input");
	load_transit_xdp_ret_val = 0;
	expect_function_call(__wrap_load_transit_xdp_1);
	will_return(__wrap_load_transit_xdp_1, &load_transit_xdp_ret_val);
	rc = trn_cli_load_transit_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, 0);

	TEST_CASE("load_transit_xdp fails if path to object file is missing");
	rc = trn_cli_load_transit_subcmd(NULL, argc, argv2);
	assert_int_equal(rc, -EINVAL);

	TEST_CASE("load_transit_xdp succeed even if pcap map is missing");
	load_transit_xdp_ret_val = 0;
	expect_function_call(__wrap_load_transit_xdp_1);
	will_return(__wrap_load_transit_xdp_1, &load_transit_xdp_ret_val);
	rc = trn_cli_load_transit_subcmd(NULL, argc, argv3);
	assert_int_equal(rc, 0);

	TEST_CASE("load_transit_xdp fails if rpc returns Error");
	load_transit_xdp_ret_val = -EINVAL;
	expect_function_call(__wrap_load_transit_xdp_1);
	will_return(__wrap_load_transit_xdp_1, &load_transit_xdp_ret_val);
	rc = trn_cli_load_transit_subcmd(NULL, argc, argv3);
	assert_int_equal(rc, -EINVAL);

	/* Test call load_transit_xdp return NULL*/
	TEST_CASE("load_transit_xdp subcommand fails if rpc returns NULl");
	expect_function_call(__wrap_load_transit_xdp_1);
	will_return(__wrap_load_transit_xdp_1, NULL);
	rc = trn_cli_load_transit_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);
}

static void test_trn_cli_unload_transit_subcmd(void **state)
{
	UNUSED(state);
	int rc;
	int argc = 5;
	int unload_transit_xdp_ret_val = 0;

	/* Test cases */
	char *argv1[] = { "load-transit-xdp", "-i", "eth0", "-j", QUOTE({}) };

	/* Test call unload_transit_xdp_1 successfully */
	TEST_CASE("unload_transit_xdp succeed with well formed (empty) input");
	unload_transit_xdp_ret_val = 0;
	expect_function_call(__wrap_unload_transit_xdp_1);
	will_return(__wrap_unload_transit_xdp_1, &unload_transit_xdp_ret_val);
	rc = trn_cli_unload_transit_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, 0);

	/* Test call unload_transit_xdp return error*/
	TEST_CASE(
		"unload_transit_xdp subcommand fails if update_net_1 returns error");
	unload_transit_xdp_ret_val = -EINVAL;
	expect_function_call(__wrap_unload_transit_xdp_1);
	will_return(__wrap_unload_transit_xdp_1, &unload_transit_xdp_ret_val);
	rc = trn_cli_unload_transit_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);

	/* Test call unload_transit_xdp_! return NULL*/
	TEST_CASE("unload_transit_xdp subcommand fails if rpc returns NULl");
	expect_function_call(__wrap_unload_transit_xdp_1);
	will_return(__wrap_unload_transit_xdp_1, NULL);
	rc = trn_cli_unload_transit_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);
}

static void test_trn_cli_load_agent_subcmd(void **state)
{
	UNUSED(state);
	int rc;
	int argc = 5;
	int load_agent_xdp_ret_val = 0;

	/* Test cases */
	char *argv1[] = { "load-agent-xdp", "-i", "eth0", "-j", QUOTE({
				  "xdp_path": "/path/to/xdp/object/file",
				  "pcapfile": "/path/to/bpf/pinned/map"
			  }) };

	char *argv2[] = { "load-agent-xdp", "-i", "eth0", "-j",
			  QUOTE({ "pcapfile": "/path/to/bpf/pinned/map" }) };

	char *argv3[] = { "load-agent-xdp", "-i", "eth0", "-j",
			  QUOTE({ "xdp_path": "/path/to/xdp/object/file" }) };

	/* Test call load_transit_xdp_1 successfully */
	TEST_CASE("load_agent_xdp succeed with well formed input");
	load_agent_xdp_ret_val = 0;
	expect_function_call(__wrap_load_transit_agent_xdp_1);
	will_return(__wrap_load_transit_agent_xdp_1, &load_agent_xdp_ret_val);
	rc = trn_cli_load_agent_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, 0);

	TEST_CASE("load_agent_xdp fails if path to object file is missing");
	rc = trn_cli_load_agent_subcmd(NULL, argc, argv2);
	assert_int_equal(rc, -EINVAL);

	TEST_CASE("load_agent_xdp succeed even if pcap map is missing");
	load_agent_xdp_ret_val = 0;
	expect_function_call(__wrap_load_transit_agent_xdp_1);
	will_return(__wrap_load_transit_agent_xdp_1, &load_agent_xdp_ret_val);
	rc = trn_cli_load_agent_subcmd(NULL, argc, argv3);
	assert_int_equal(rc, 0);

	TEST_CASE("load_agent_xdp fails if rpc returns Error");
	load_agent_xdp_ret_val = -EINVAL;
	expect_function_call(__wrap_load_transit_agent_xdp_1);
	will_return(__wrap_load_transit_agent_xdp_1, &load_agent_xdp_ret_val);
	rc = trn_cli_load_agent_subcmd(NULL, argc, argv3);
	assert_int_equal(rc, -EINVAL);

	/* Test call load_transit_xdp return NULL*/
	TEST_CASE("load_agent_xdp subcommand fails if rpc returns NULl");
	expect_function_call(__wrap_load_transit_agent_xdp_1);
	will_return(__wrap_load_transit_agent_xdp_1, NULL);
	rc = trn_cli_load_agent_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);
}

static void test_trn_cli_unload_agent_subcmd(void **state)
{
	UNUSED(state);
	int rc;
	int argc = 5;
	int unload_transit_xdp_ret_val = 0;

	/* Test cases */
	char *argv1[] = { "unload-agent-xdp", "-i", "eth0", "-j", QUOTE({}) };

	/* Test call unload_agent_xdp_1 successfully */
	TEST_CASE("unload_agent_xdp succeed with well formed (empty) input");
	unload_transit_xdp_ret_val = 0;
	expect_function_call(__wrap_unload_transit_agent_xdp_1);
	will_return(__wrap_unload_transit_agent_xdp_1,
		    &unload_transit_xdp_ret_val);
	rc = trn_cli_unload_agent_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, 0);

	/* Test call unload_agent_xdp return error*/
	TEST_CASE(
		"unload_tagent_xdp subcommand fails if update_net_1 returns error");
	unload_transit_xdp_ret_val = -EINVAL;
	expect_function_call(__wrap_unload_transit_agent_xdp_1);
	will_return(__wrap_unload_transit_agent_xdp_1,
		    &unload_transit_xdp_ret_val);
	rc = trn_cli_unload_agent_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);

	/* Test call unload_agent_xdp return NULL*/
	TEST_CASE("unload_agent_xdp subcommand fails if rpc returns NULl");
	expect_function_call(__wrap_unload_transit_agent_xdp_1);
	will_return(__wrap_unload_transit_agent_xdp_1, NULL);
	rc = trn_cli_unload_agent_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);
}

static void test_trn_cli_update_agent_ep_subcmd(void **state)
{
	UNUSED(state);
	int rc;
	int argc = 5;
	int update_agent_ep_1_ret_val = 0;

	/* Test cases */
	char *argv1[] = { "update-agent-ep", "-i", "eth0", "-j", QUOTE({
				  "tunnel_id": "0",
				  "ip": "10.0.0.1",
				  "eptype": "0",
				  "mac": "1:2:3:4:5:6",
				  "veth": "",
				  "remote_ips": [],
				  "hosted_iface": ""
			  }) };

	char *argv2[] = { "update-agent-ep", "-i", "eth0", "-j", QUOTE({
				  "tunnel_id": 0,
				  "ip": "10.0.0.1",
				  "eptype": "0",
				  "mac": "1:2:3:4:5:6",
				  "veth": "",
				  "remote_ips": [],
				  "hosted_iface": ""
			  }) };

	char *argv3[] = { "update-agent-ep", "-i", "eth0", "-j", QUOTE({
				  "tunnel_id": "0",
				  "mac": "1:2:3:4:5:6",
				  "veth": "",
				  "remote_ips": [],
				  "hosted_iface": ""
			  }) };

	char *argv4[] = { "update-agent-ep", "-i", "eth0", "-j", QUOTE({
				  "tunnel_id": "0",
				  "ip": "10.0.0.1",
				  "eptype": "0",
				  "mac": "1:2:3:4:5:6",
				  "veth":,
				  "remote_ips": [],
				  "hosted_iface": ""
			  }) };

	char itf[] = "eth0";
	char vitf[] = "";
	char hosted_itf[] = "";
	char mac[6] = { 1, 2, 3, 4, 5, 6 };

	struct rpc_trn_endpoint_t exp_ep = {
		.interface = itf,
		.ip = 0x100000a,
		.eptype = 0,
		.remote_ips = { .remote_ips_len = 0 },
		.hosted_interface = hosted_itf,
		.veth = vitf,
		.tunid = 0,
	};

	memcpy(exp_ep.mac, mac, sizeof(char) * 6);

	/* Test call update_agent_ep successfully */
	TEST_CASE(
		"update_agent_ep succeed with well formed endpoint json input");
	update_agent_ep_1_ret_val = 0;
	expect_function_call(__wrap_update_agent_ep_1);
	will_return(__wrap_update_agent_ep_1, &update_agent_ep_1_ret_val);
	expect_check(__wrap_update_agent_ep_1, ep, check_ep_equal, &exp_ep);
	expect_any(__wrap_update_agent_ep_1, clnt);
	rc = trn_cli_update_agent_ep_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, 0);

	/* Test parse ep input error*/
	TEST_CASE("update_agent_ep is not called with non-string field");
	rc = trn_cli_update_agent_ep_subcmd(NULL, argc, argv2);
	assert_int_equal(rc, -EINVAL);

	TEST_CASE("update_agent_ep is not called with missing required field");
	rc = trn_cli_update_agent_ep_subcmd(NULL, argc, argv3);
	assert_int_equal(rc, -EINVAL);

	/* Test parse ep input error 2*/
	TEST_CASE("update_agent_ep is not called malformed json");
	rc = trn_cli_update_agent_ep_subcmd(NULL, argc, argv4);
	assert_int_equal(rc, -EINVAL);

	/* Test call update_ep_1 return error*/
	TEST_CASE(
		"update_agent_ep subcommand fails if update_net_1 returns error");
	update_agent_ep_1_ret_val = -EINVAL;
	expect_function_call(__wrap_update_agent_ep_1);
	will_return(__wrap_update_agent_ep_1, &update_agent_ep_1_ret_val);
	expect_any(__wrap_update_agent_ep_1, ep);
	expect_any(__wrap_update_agent_ep_1, clnt);
	rc = trn_cli_update_agent_ep_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);

	/* Test call update_ep_1 return NULL*/
	TEST_CASE(
		"update_agent_ep subcommand fails if update_ep_1 returns NULl");
	expect_function_call(__wrap_update_agent_ep_1);
	will_return(__wrap_update_agent_ep_1, NULL);
	expect_any(__wrap_update_agent_ep_1, ep);
	expect_any(__wrap_update_agent_ep_1, clnt);
	rc = trn_cli_update_agent_ep_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);
}

static void test_trn_cli_update_packet_metadata_subcmd(void **state)
{
	UNUSED(state);
	int rc;
	int argc = 5;
	int update_packet_metadata_1_ret_val = 0;

	/* Test cases */
	char *argv1[] = { "update-packet-metadata", "-i", "eth0", "-j", QUOTE({
				  "tunnel_id": "0",
				  "ip": "10.0.0.1",
				  "pod_label_value": "11",
				  "namespace_label_value": "1"
			  }) };

	char *argv2[] = { "update-packet-metadata", "-i", "eth0", "-j", QUOTE({
				  "tunnel_id": 0,
				  "ip": "10.0.0.1",
				  "pod_label_value": "11",
				  "namespace_label_value": "1"
			  }) };

	char *argv3[] = { "update-packet-metadata", "-i", "eth0", "-j", QUOTE({
				  "tunnel_id": "0",
				  "pod_label_value": "11",
				  "namespace_label_value": "1"
			  }) };

	char *argv4[] = { "update-packet-metadata", "-i", "eth0", "-j", QUOTE({
				  "tunnel_id": "0",
				  "ip": "10.0.0.1",
				  "pod_label_value": ,
				  "namespace_label_value": "1"
			  }) };

	char itf[] = "eth0";

	struct rpc_trn_packet_metadata_t exp_packet_metadata = {
		.interface = itf,
		.ip = 0x100000a,
		.pod_label_value = 11,
		.namespace_label_value = 1,
		.tunid = 0,
	};

	/* Test call update_packet_metadata successfully */
	TEST_CASE(
		"update_packet_metadata succeed with well formed endpoint json input");
	update_packet_metadata_1_ret_val = 0;
	expect_function_call(__wrap_update_packet_metadata_1);
	will_return(__wrap_update_packet_metadata_1, &update_packet_metadata_1_ret_val);
	expect_check(__wrap_update_packet_metadata_1, packet_metadata, check_packet_metadata_equal, &exp_packet_metadata);
	expect_any(__wrap_update_packet_metadata_1, clnt);
	rc = trn_cli_update_packet_metadata_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, 0);

	/* Test parse packet metadata input error*/
	TEST_CASE("update_packet_metadata is not called with non-string field");
	rc = trn_cli_update_packet_metadata_subcmd(NULL, argc, argv2);
	assert_int_equal(rc, -EINVAL);

	TEST_CASE("update_packet_metadata is not called with missing required field");
	rc = trn_cli_update_packet_metadata_subcmd(NULL, argc, argv3);
	assert_int_equal(rc, -EINVAL);

	/* Test parse packet metadata input error 2*/
	TEST_CASE("update_packet_metadata is not called malformed json");
	rc = trn_cli_update_packet_metadata_subcmd(NULL, argc, argv4);
	assert_int_equal(rc, -EINVAL);

	/* Test call update_packet_metadata_1 return error*/
	TEST_CASE(
		"update_packet_metadata subcommand fails if update_packet_metadata_1 returns error");
	update_packet_metadata_1_ret_val = -EINVAL;
	expect_function_call(__wrap_update_packet_metadata_1);
	will_return(__wrap_update_packet_metadata_1, &update_packet_metadata_1_ret_val);
	expect_any(__wrap_update_packet_metadata_1, packet_metadata);
	expect_any(__wrap_update_packet_metadata_1, clnt);
	rc = trn_cli_update_packet_metadata_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);

	/* Test call update_packet_metadata_1 return NULL*/
	TEST_CASE(
		"update_packet_metadata subcommand fails if update_packet_metadata_1 returns NULl");
	expect_function_call(__wrap_update_packet_metadata_1);
	will_return(__wrap_update_packet_metadata_1, NULL);
	expect_any(__wrap_update_packet_metadata_1, packet_metadata);
	expect_any(__wrap_update_packet_metadata_1, clnt);
	rc = trn_cli_update_packet_metadata_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);
}

static void test_trn_cli_update_agent_md_subcmd(void **state)
{
	UNUSED(state);

	int rc;
	int argc = 5;
	int update_agent_md_1_ret_val = 0;

	/* Test cases */
	char *argv1[] = { "update-agent-metadata", "-i", "eth0", "-j", QUOTE({
				  "ep": {
					  "tunnel_id": "3",
					  "ip": "10.0.0.1",
					  "eptype": "1",
					  "mac": "1:2:3:4:5:6",
					  "veth": "veth0",
					  "remote_ips": ["10.0.0.2"],
					  "hosted_iface": "eth0"
				  },
				  "net": {
					  "tunnel_id": "3",
					  "nip": "10.0.0.0",
					  "prefixlen": "16",
					  "switches_ips":
						  ["10.0.0.1", "10.0.0.2"]
				  },
				  "eth": {
					  "ip": "10.0.0.1",
					  "mac": "6:5:4:3:2:1",
					  "iface": "eth0"
				  }
			  }) };

	char *argv2[] = {
		"update-agent-metadata", "-i", "eth0", "-j", QUOTE({
			"ep": {
				"tunnel_id": "3",
				"ip": "10.0.0.1",
				"eptype": "1",
				"mac": "1:2:3:4:5:6",
				"veth": "veth0",
				"remote_ips": ["10.0.0.2"],
				"hosted_iface": "eth0"
			},
			"net": {
				"tunnel_id": "3",
				"nip": "10.0.0.0",
				"prefixlen": "16",
				"switches_ips": ["10.0.0.1", "10.0.0.2"]
			},
			"eth": { "ip": "10.0.0.1", "mac": "6:5:4:3:2:1" }
		})
	};

	char *argv3[] = { "update-agent-metadata", "-i", "eth0", "-j", QUOTE({
				  "ep": {
					  "tunnel_id": "3",
					  "ip": "10.0.0.1",
					  "eptype": "1",
					  "mac": "1:2:3:4:5:6",
					  "veth": "veth0",
					  "remote_ips": ["10.0.0.2"],
					  "hosted_iface": "eth0"
				  },
				  "net": {
					  "tunnel_id": "3",
					  "nip": "10.0.0.0",
					  "prefixlen": "16",
					  "switches_ips":
						  ["10.0.0.1", "10.0.0.2"]
				  },
				  "eth": {
					  "ip": "10.0.0.1",
					  "mac": "6:5:4:3:2:1",
					  "iface": eth0
				  }
			  }) };

	char itf[] = "eth0";
	char vitf[] = "veth0";
	char hosted_itf[] = "eth0";
	uint32_t remote[] = { 0x200000a };
	uint32_t switches[] = { 0x100000a, 0x200000a };
	char mac[6] = { 1, 2, 3, 4, 5, 6 };
	char mac_eth[6] = { 6, 5, 4, 3, 2, 1 };

	struct rpc_trn_agent_metadata_t exp_md = {
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

	memcpy(exp_md.ep.mac, mac, sizeof(char) * 6);
	memcpy(exp_md.eth.mac, mac_eth, sizeof(char) * 6);

	TEST_CASE(
		"update_agent_md succeed with well formed agent metadata json input");
	update_agent_md_1_ret_val = 0;
	expect_function_call(__wrap_update_agent_md_1);
	will_return(__wrap_update_agent_md_1, &update_agent_md_1_ret_val);
	expect_check(__wrap_update_agent_md_1, md, check_md_equal, &exp_md);
	expect_any(__wrap_update_agent_md_1, clnt);
	rc = trn_cli_update_agent_md_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, 0);

	TEST_CASE("update_agent_md is not called with missing eth0 field");
	rc = trn_cli_update_agent_md_subcmd(NULL, argc, argv2);
	assert_int_equal(rc, -EINVAL);

	TEST_CASE("update_agent_md is not called is not called malformed json");
	rc = trn_cli_update_agent_md_subcmd(NULL, argc, argv3);
	assert_int_equal(rc, -EINVAL);

	TEST_CASE("update_agent_md subcommand fails if rpc returns error");
	update_agent_md_1_ret_val = -EINVAL;
	expect_function_call(__wrap_update_agent_md_1);
	will_return(__wrap_update_agent_md_1, &update_agent_md_1_ret_val);
	expect_check(__wrap_update_agent_md_1, md, check_md_equal, &exp_md);
	expect_any(__wrap_update_agent_md_1, clnt);
	rc = trn_cli_update_agent_md_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);

	TEST_CASE("update_agent_md subcommand fails if rpc returns NULl");
	expect_function_call(__wrap_update_agent_md_1);
	will_return(__wrap_update_agent_md_1, NULL);
	expect_check(__wrap_update_agent_md_1, md, check_md_equal, &exp_md);
	expect_any(__wrap_update_agent_md_1, clnt);
	rc = trn_cli_update_agent_md_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);
}

static void test_trn_cli_get_vpc_subcmd(void **state)
{
	UNUSED(state);
	rpc_trn_vpc_t return_vpc;
	int argc = 5;
	int rc;

	char itf[] = "eth0";
	uint32_t routers[] = { 0x100000a, 0x200000a };
	struct rpc_trn_vpc_t get_vpc_1_ret_val = {
		.interface = itf,
		.tunid = 3,
		.routers_ips = { .routers_ips_len = 2,
				 .routers_ips_val = routers }
	};

	struct rpc_trn_vpc_key_t exp_vpc_key = {
		.interface = itf,
		.tunid = 3,
	};
	/* Test cases */
	char *argv1[] = { "get-vpc", "-i", "eth0", "-j",
			  QUOTE({ "tunnel_id": "3" }) };

	char *argv2[] = { "get-vpc", "-i", "eth0", "-j",
			  QUOTE({ "tunnel_id": 3 }) };

	char *argv3[] = { "get-vpc", "-i", "eth0", "-j",
			  QUOTE({ "tunnel_id: 3" }) };

	/* Test call get_vpc successfully */
	TEST_CASE("get_vpc succeed with well formed vpc json input");
	expect_function_call(__wrap_get_vpc_1);
	will_return(__wrap_get_vpc_1, &get_vpc_1_ret_val);
	expect_check(__wrap_get_vpc_1, argp, check_vpc_key_equal, &exp_vpc_key);
	expect_any(__wrap_get_vpc_1, clnt);
	rc = trn_cli_get_vpc_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, 0);

	/* Test parse vpc input error*/
	TEST_CASE("get_vpc is not called with non-string field");
	rc = trn_cli_get_vpc_subcmd(NULL, argc, argv2);
	assert_int_equal(rc, -EINVAL);

	/* Test parse vpc input error 2*/
	TEST_CASE("get_vpc is not called malformed json");
	rc = trn_cli_get_vpc_subcmd(NULL, argc, argv3);
	assert_int_equal(rc, -EINVAL);

	/* Test call get_vpc_1 return error*/
	TEST_CASE(
		"get-vpc subcommand fails if get_vpc_1 returns empty string interface");
	get_vpc_1_ret_val.interface = "";
	expect_function_call(__wrap_get_vpc_1);
	will_return(__wrap_get_vpc_1, &get_vpc_1_ret_val);
	expect_any(__wrap_get_vpc_1, argp);
	expect_any(__wrap_get_vpc_1, clnt);
	rc = trn_cli_get_vpc_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);

	/* Test call get_vpc_1 return NULL*/
	TEST_CASE("get-vpc subcommand fails if get_vpc_1 returns NULl");
	expect_function_call(__wrap_get_vpc_1);
	will_return(__wrap_get_vpc_1, NULL);
	expect_any(__wrap_get_vpc_1, argp);
	expect_any(__wrap_get_vpc_1, clnt);
	rc = trn_cli_get_vpc_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);
}

static void test_trn_cli_get_net_subcmd(void **state)
{
	UNUSED(state);
	int rc;
	int argc = 5;

	char itf[] = "eth0";
	uint32_t switches[] = { 0x100000a, 0x200000a };
	struct rpc_trn_network_t get_net_1_ret_val = {
		.interface = itf,
		.prefixlen = 16,
		.tunid = 3,
		.netip = 0xa,
		.switches_ips = { .switches_ips_len = 2,
				  .switches_ips_val = switches }
	};

	/* Test cases */
	char *argv1[] = { "get-net", "-i", "eth0", "-j", QUOTE({
				  "tunnel_id": "3",
				  "nip": "10.0.0.0",
				  "prefixlen": "16"
			  }) };

	char *argv2[] = {
		"get-net", "-i", "eth0", "-j",
		QUOTE({ "tunnel_id": 3, "nip": "10.0.0.0", "prefixlen": "16" })
	};

	char *argv3[] = {
		"get-net", "-i", "eth0", "-j",
		QUOTE({ "tunnel_id": "3", "nip": "adsfwef", "prefixlen: 16" })
	};

	struct rpc_trn_network_key_t exp_net_key = {
		.interface = itf,
		.prefixlen = 16,
		.tunid = 3,
		.netip = 0xa,
	};

	/* Test call get_net successfully */
	TEST_CASE("get_net succeed with well formed network json input");
	expect_function_call(__wrap_get_net_1);
	will_return(__wrap_get_net_1, &get_net_1_ret_val);
	expect_check(__wrap_get_net_1, argp, check_net_key_equal, &exp_net_key);
	expect_any(__wrap_get_net_1, clnt);
	rc = trn_cli_get_net_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, 0);

	/* Test parse net key input error*/
	TEST_CASE("get_net is not called with non-string field");
	rc = trn_cli_get_net_subcmd(NULL, argc, argv2);
	assert_int_equal(rc, -EINVAL);

	/* Test parse net key input error 2*/
	TEST_CASE("get_net is not called malformed json");
	rc = trn_cli_get_net_subcmd(NULL, argc, argv3);
	assert_int_equal(rc, -EINVAL);

	/* Test call get_net_1 return error*/
	TEST_CASE(
		"get-net subcommand fails if get_net_1 returns empty string interface");
	get_net_1_ret_val.interface = "";
	expect_function_call(__wrap_get_net_1);
	will_return(__wrap_get_net_1, &get_net_1_ret_val);
	expect_any(__wrap_get_net_1, argp);
	expect_any(__wrap_get_net_1, clnt);
	rc = trn_cli_get_net_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);

	/* Test call get_net_1 return NULL*/
	TEST_CASE("get-net subcommand fails if get_net_1 returns NULl");
	expect_function_call(__wrap_get_net_1);
	will_return(__wrap_get_net_1, NULL);
	expect_any(__wrap_get_net_1, argp);
	expect_any(__wrap_get_net_1, clnt);
	rc = trn_cli_get_net_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);
}

static void test_trn_cli_get_ep_subcmd(void **state)
{
	UNUSED(state);
	int rc;
	int argc = 5;

	char itf[] = "eth0";
	char vitf[] = "veth0";
	char hosted_itf[] = "peer";
	uint32_t remote[] = { 0x200000a };
	char mac[6] = { 1, 2, 3, 4, 5, 6 };

	struct rpc_trn_endpoint_t get_ep_1_ret_val = {
		.interface = itf,
		.ip = 0x100000a,
		.eptype = 1,
		.remote_ips = { .remote_ips_len = 1, .remote_ips_val = remote },
		.hosted_interface = hosted_itf,
		.veth = vitf,
		.tunid = 3,
	};
	memcpy(get_ep_1_ret_val.mac, mac, sizeof(char) * 6);

	/* Test cases */
	char *argv1[] = { "get-ep", "-i", "eth0", "-j",
			  QUOTE({ "tunnel_id": "3", "ip": "10.0.0.1" }) };

	char *argv2[] = { "get-ep", "-i", "eth0", "-j",
			  QUOTE({ "tunnel_id": 3, "ip": "10.0.0.1" }) };

	char *argv3[] = { "get-ep", "-i", "eth0", "-j",
			  QUOTE({ "tunnel_id": "3" }) };

	char *argv4[] = { "get-ep", "-i", "eth0", "-j",
			  QUOTE({ "tunnel_id": "3", "ip": [10.0.0.2] }) };

	struct rpc_trn_endpoint_key_t exp_ep_key = {
		.interface = itf,
		.ip = 0x100000a,
		.tunid = 3,
	};

	/* Test call get_ep successfully */
	TEST_CASE("get_ep succeed with well formed endpoint json input");
	expect_function_call(__wrap_get_ep_1);
	will_return(__wrap_get_ep_1, &get_ep_1_ret_val);
	expect_check(__wrap_get_ep_1, argp, check_ep_key_equal, &exp_ep_key);
	expect_any(__wrap_get_ep_1, clnt);
	rc = trn_cli_get_ep_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, 0);

	/* Test parse ep input error*/
	TEST_CASE("get_ep is not called with non-string field");
	rc = trn_cli_get_ep_subcmd(NULL, argc, argv2);
	assert_int_equal(rc, -EINVAL);

	TEST_CASE("get_ep is not called with missing required field");
	rc = trn_cli_get_ep_subcmd(NULL, argc, argv3);
	assert_int_equal(rc, -EINVAL);

	/* Test parse ep input error 2*/
	TEST_CASE("get_ep is not called malformed json");
	rc = trn_cli_get_ep_subcmd(NULL, argc, argv4);
	assert_int_equal(rc, -EINVAL);

	/* Test call get_ep_1 return error*/
	TEST_CASE(
		"get-ep subcommand fails if get_ep_1 returns empty string interface");
	get_ep_1_ret_val.interface = "";
	expect_function_call(__wrap_get_ep_1);
	will_return(__wrap_get_ep_1, &get_ep_1_ret_val);
	expect_any(__wrap_get_ep_1, argp);
	expect_any(__wrap_get_ep_1, clnt);
	rc = trn_cli_get_ep_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);

	/* Test call get_ep_1 return NULL*/
	TEST_CASE("get-ep subcommand fails if get_ep_1 returns NULl");
	expect_function_call(__wrap_get_ep_1);
	will_return(__wrap_get_ep_1, NULL);
	expect_any(__wrap_get_ep_1, argp);
	expect_any(__wrap_get_ep_1, clnt);
	rc = trn_cli_get_ep_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);
}

static void test_trn_cli_get_agent_ep_subcmd(void **state)
{
	UNUSED(state);
	int rc;
	int argc = 5;

	char itf[] = "eth0";
	char vitf[] = "veth0";
	char hosted_itf[] = "peer";
	uint32_t remote[] = { 0x200000a };
	char mac[6] = { 1, 2, 3, 4, 5, 6 };

	struct rpc_trn_endpoint_t get_agent_ep_1_ret_val = {
		.interface = itf,
		.ip = 0x100000a,
		.eptype = 1,
		.remote_ips = { .remote_ips_len = 1, .remote_ips_val = remote },
		.hosted_interface = hosted_itf,
		.veth = vitf,
		.tunid = 3,
	};
	memcpy(get_agent_ep_1_ret_val.mac, mac, sizeof(char) * 6);

	/* Test cases */
	char *argv1[] = { "get-agent-ep", "-i", "eth0", "-j",
			  QUOTE({ "tunnel_id": "3", "ip": "10.0.0.1" }) };

	char *argv2[] = { "get-agent-ep", "-i", "eth0", "-j",
			  QUOTE({ "tunnel_id": 3, "ip": "10.0.0.1" }) };

	char *argv3[] = { "get-agent-ep", "-i", "eth0", "-j",
			  QUOTE({ "tunnel_id": "3" }) };

	char *argv4[] = { "get-agent-ep", "-i", "eth0", "-j",
			  QUOTE({ "tunnel_id": "3", "ip": [10.0.0.2] }) };

	struct rpc_trn_endpoint_key_t exp_agent_ep_key = {
		.interface = itf,
		.ip = 0x100000a,
		.tunid = 3,
	};

	/* Test call get_agent_ep successfully */
	TEST_CASE("get_agent_ep succeed with well formed endpoint json input");
	expect_function_call(__wrap_get_agent_ep_1);
	will_return(__wrap_get_agent_ep_1, &get_agent_ep_1_ret_val);
	expect_check(__wrap_get_agent_ep_1, argp, check_ep_key_equal,
		     &exp_agent_ep_key);
	expect_any(__wrap_get_agent_ep_1, clnt);
	rc = trn_cli_get_agent_ep_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, 0);

	/* Test parse ep input error*/
	TEST_CASE("get_agent_ep is not called with non-string field");
	rc = trn_cli_get_agent_ep_subcmd(NULL, argc, argv2);
	assert_int_equal(rc, -EINVAL);

	TEST_CASE("get_agent_ep is not called with missing required field");
	rc = trn_cli_get_agent_ep_subcmd(NULL, argc, argv3);
	assert_int_equal(rc, -EINVAL);

	/* Test parse ep input error 2*/
	TEST_CASE("get_agent_ep is not called malformed json");
	rc = trn_cli_get_agent_ep_subcmd(NULL, argc, argv4);
	assert_int_equal(rc, -EINVAL);

	/* Test call get_ep_1 return error*/
	TEST_CASE(
		"get_agent_ep subcommand fails if get_ep_1 returns empty string interface");
	get_agent_ep_1_ret_val.interface = "";
	expect_function_call(__wrap_get_agent_ep_1);
	will_return(__wrap_get_agent_ep_1, &get_agent_ep_1_ret_val);
	expect_any(__wrap_get_agent_ep_1, argp);
	expect_any(__wrap_get_agent_ep_1, clnt);
	rc = trn_cli_get_agent_ep_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);

	/* Test call get_agent_ep_1 return NULL*/
	TEST_CASE("get_agent_ep subcommand fails if get_ep_1 returns NULl");
	expect_function_call(__wrap_get_agent_ep_1);
	will_return(__wrap_get_agent_ep_1, NULL);
	expect_any(__wrap_get_agent_ep_1, argp);
	expect_any(__wrap_get_agent_ep_1, clnt);
	rc = trn_cli_get_agent_ep_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);
}

static void test_trn_cli_get_agent_md_subcmd(void **state)
{
	UNUSED(state);
	int rc;
	int argc = 5;

	char itf[] = "eth0";
	char vitf[] = "veth0";
	char hosted_itf[] = "eth0";
	uint32_t remote[] = { 0x200000a };
	uint32_t switches[] = { 0x100000a, 0x200000a };
	char mac[6] = { 1, 2, 3, 4, 5, 6 };
	char mac_eth[6] = { 6, 5, 4, 3, 2, 1 };
	struct rpc_trn_agent_metadata_t get_agent_md_1_ret_val = {
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
	memcpy(get_agent_md_1_ret_val.ep.mac, mac, sizeof(char) * 6);
	memcpy(get_agent_md_1_ret_val.eth.mac, mac_eth, sizeof(char) * 6);

	/* Test cases */
	char *argv1[] = { "get-agent-metadata", "-i", "eth0", "-j", QUOTE({
				  "": "",
			  }) };

	rpc_intf_t exp_md_itf = { .interface = itf };

	TEST_CASE(
		"get_agent_md succeed with well formed agent metadata json input");
	expect_function_call(__wrap_get_agent_md_1);
	will_return(__wrap_get_agent_md_1, &get_agent_md_1_ret_val);
	expect_check(__wrap_get_agent_md_1, argp, check_md_itf_equal,
		     &exp_md_itf);
	expect_any(__wrap_get_agent_md_1, clnt);
	rc = trn_cli_get_agent_md_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, 0);

	TEST_CASE(
		"get_agent_md subcommand fails if rpc returns empty string interfac");
	get_agent_md_1_ret_val.interface = "";
	expect_function_call(__wrap_get_agent_md_1);
	will_return(__wrap_get_agent_md_1, &get_agent_md_1_ret_val);
	expect_check(__wrap_get_agent_md_1, argp, check_md_itf_equal,
		     &exp_md_itf);
	expect_any(__wrap_get_agent_md_1, clnt);
	rc = trn_cli_get_agent_md_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);

	TEST_CASE("get_agent_md subcommand fails if rpc returns NULl");
	expect_function_call(__wrap_get_agent_md_1);
	will_return(__wrap_get_agent_md_1, NULL);
	expect_check(__wrap_get_agent_md_1, argp, check_md_itf_equal,
		     &exp_md_itf);
	expect_any(__wrap_get_agent_md_1, clnt);
	rc = trn_cli_get_agent_md_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);
}

static void test_trn_cli_delete_vpc_subcmd(void **state)
{
	UNUSED(state);
	int argc = 5;
	int rc;

	char itf[] = "eth0";
	struct rpc_trn_vpc_key_t exp_vpc_key = {
		.interface = itf,
		.tunid = 3,
	};
	/* Test cases */
	char *argv1[] = { "delete-vpc", "-i", "eth0", "-j",
			  QUOTE({ "tunnel_id": "3" }) };

	char *argv2[] = { "delete-vpc", "-i", "eth0", "-j",
			  QUOTE({ "tunnel_id": 3 }) };

	char *argv3[] = { "delete-vpc", "-i", "eth0", "-j",
			  QUOTE({ "tunnel_id: 3" }) };

	int delete_vpc_1_ret_val = 0;
	/* Test call delete-vpc successfully */
	TEST_CASE("delete_vpc_1 succeed with well formed vpc json input");
	expect_function_call(__wrap_delete_vpc_1);
	will_return(__wrap_delete_vpc_1, &delete_vpc_1_ret_val);
	expect_check(__wrap_delete_vpc_1, argp, check_vpc_key_equal,
		     &exp_vpc_key);
	expect_any(__wrap_delete_vpc_1, clnt);
	rc = trn_cli_delete_vpc_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, 0);

	/* Test parse vpc input error*/
	TEST_CASE("delete_vpc_1 is not called with non-string field");
	rc = trn_cli_delete_vpc_subcmd(NULL, argc, argv2);
	assert_int_equal(rc, -EINVAL);

	/* Test parse vpc input error 2*/
	TEST_CASE("delete_vpc_1 is not called malformed json");
	rc = trn_cli_delete_vpc_subcmd(NULL, argc, argv3);
	assert_int_equal(rc, -EINVAL);

	/* Test call delete_vpc_1 return error*/
	delete_vpc_1_ret_val = -EINVAL;
	TEST_CASE("delete-vpc subcommand fails if delete_vpc_1 returns error");
	expect_function_call(__wrap_delete_vpc_1);
	will_return(__wrap_delete_vpc_1, &delete_vpc_1_ret_val);
	expect_any(__wrap_delete_vpc_1, argp);
	expect_any(__wrap_delete_vpc_1, clnt);
	rc = trn_cli_delete_vpc_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);

	/* Test call delete_vpc_1 return NULL*/
	TEST_CASE("delete-vpc subcommand fails if delete_vpc_1 returns NULL");
	expect_function_call(__wrap_delete_vpc_1);
	will_return(__wrap_delete_vpc_1, NULL);
	expect_any(__wrap_delete_vpc_1, argp);
	expect_any(__wrap_delete_vpc_1, clnt);
	rc = trn_cli_delete_vpc_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);
}

static void test_trn_cli_delete_net_subcmd(void **state)
{
	UNUSED(state);
	int rc;
	int argc = 5;
	char itf[] = "eth0";

	/* Test cases */
	char *argv1[] = { "delete-net", "-i", "eth0", "-j", QUOTE({
				  "tunnel_id": "3",
				  "nip": "10.0.0.0",
				  "prefixlen": "16"
			  }) };

	char *argv2[] = {
		"delete-net", "-i", "eth0", "-j",
		QUOTE({ "tunnel_id": 3, "nip": "10.0.0.0", "prefixlen": "16" })
	};

	char *argv3[] = {
		"delete-net", "-i", "eth0", "-j",
		QUOTE({ "tunnel_id": "3", "nip": "adsfwef", "prefixlen: 16" })
	};

	struct rpc_trn_network_key_t exp_net_key = {
		.interface = itf,
		.prefixlen = 16,
		.tunid = 3,
		.netip = 0xa,
	};

	int delete_net_1_ret_val = 0;
	/* Test call delete-net successfully */
	TEST_CASE("delete_net_1 succeed with well formed network json input");
	expect_function_call(__wrap_delete_net_1);
	will_return(__wrap_delete_net_1, &delete_net_1_ret_val);
	expect_check(__wrap_delete_net_1, argp, check_net_key_equal,
		     &exp_net_key);
	expect_any(__wrap_delete_net_1, clnt);
	rc = trn_cli_delete_net_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, 0);

	/* Test parse net key input error*/
	TEST_CASE("delete_net_1 is not called with non-string field");
	rc = trn_cli_delete_net_subcmd(NULL, argc, argv2);
	assert_int_equal(rc, -EINVAL);

	/* Test parse net key input error 2*/
	TEST_CASE("delete_net_1 is not called malformed json");
	rc = trn_cli_delete_net_subcmd(NULL, argc, argv3);
	assert_int_equal(rc, -EINVAL);

	/* Test call delete_net_1 return error*/
	TEST_CASE("delete-net subcommand fails if delete_net_1 returns error");
	delete_net_1_ret_val = -EINVAL;
	expect_function_call(__wrap_delete_net_1);
	will_return(__wrap_delete_net_1, &delete_net_1_ret_val);
	expect_any(__wrap_delete_net_1, argp);
	expect_any(__wrap_delete_net_1, clnt);
	rc = trn_cli_delete_net_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);

	/* Test call delete_net_1 return NULL*/
	TEST_CASE("delete-net subcommand fails if delete_net_1 returns NULL");
	expect_function_call(__wrap_delete_net_1);
	will_return(__wrap_delete_net_1, NULL);
	expect_any(__wrap_delete_net_1, argp);
	expect_any(__wrap_delete_net_1, clnt);
	rc = trn_cli_delete_net_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);
}

static void test_trn_cli_delete_ep_subcmd(void **state)
{
	UNUSED(state);
	int rc;
	int argc = 5;

	char itf[] = "eth0";

	/* Test cases */
	char *argv1[] = { "delete-ep", "-i", "eth0", "-j",
			  QUOTE({ "tunnel_id": "3", "ip": "10.0.0.1" }) };

	char *argv2[] = { "delete-ep", "-i", "eth0", "-j",
			  QUOTE({ "tunnel_id": 3, "ip": "10.0.0.1" }) };

	char *argv3[] = { "delete-ep", "-i", "eth0", "-j",
			  QUOTE({ "tunnel_id": "3" }) };

	char *argv4[] = { "delete-ep", "-i", "eth0", "-j",
			  QUOTE({ "tunnel_id": "3", "ip": [10.0.0.2] }) };

	struct rpc_trn_endpoint_key_t exp_ep_key = {
		.interface = itf,
		.ip = 0x100000a,
		.tunid = 3,
	};

	int delete_ep_1_ret_val = 0;
	/* Test call delete_ep_1 successfully */
	TEST_CASE("delete_ep_1 succeed with well formed endpoint json input");
	expect_function_call(__wrap_delete_ep_1);
	will_return(__wrap_delete_ep_1, &delete_ep_1_ret_val);
	expect_check(__wrap_delete_ep_1, argp, check_ep_key_equal, &exp_ep_key);
	expect_any(__wrap_delete_ep_1, clnt);
	rc = trn_cli_delete_ep_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, 0);

	/* Test parse ep input error*/
	TEST_CASE("delete_ep_1 is not called with non-string field");
	rc = trn_cli_delete_ep_subcmd(NULL, argc, argv2);
	assert_int_equal(rc, -EINVAL);

	TEST_CASE("delete_ep_1 is not called with missing required field");
	rc = trn_cli_delete_ep_subcmd(NULL, argc, argv3);
	assert_int_equal(rc, -EINVAL);

	/* Test parse ep input error 2*/
	TEST_CASE("delete_ep_1 is not called malformed json");
	rc = trn_cli_delete_ep_subcmd(NULL, argc, argv4);
	assert_int_equal(rc, -EINVAL);

	/* Test call delete_ep_1 return error*/
	TEST_CASE("delete-ep subcommand fails if delete_ep_1 returns error");
	delete_ep_1_ret_val = -EINVAL;
	expect_function_call(__wrap_delete_ep_1);
	will_return(__wrap_delete_ep_1, &delete_ep_1_ret_val);
	expect_any(__wrap_delete_ep_1, argp);
	expect_any(__wrap_delete_ep_1, clnt);
	rc = trn_cli_delete_ep_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);

	/* Test call delete_ep_1 return NULL*/
	TEST_CASE("delete-ep subcommand fails if delete_ep_1 returns NULL");
	expect_function_call(__wrap_delete_ep_1);
	will_return(__wrap_delete_ep_1, NULL);
	expect_any(__wrap_delete_ep_1, argp);
	expect_any(__wrap_delete_ep_1, clnt);
	rc = trn_cli_delete_ep_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);
}

static void test_trn_cli_delete_agent_ep_subcmd(void **state)
{
	UNUSED(state);
	int rc;
	int argc = 5;

	char itf[] = "eth0";

	/* Test cases */
	char *argv1[] = { "delete-agent-ep", "-i", "eth0", "-j",
			  QUOTE({ "tunnel_id": "3", "ip": "10.0.0.1" }) };

	char *argv2[] = { "delete-agent-ep", "-i", "eth0", "-j",
			  QUOTE({ "tunnel_id": 3, "ip": "10.0.0.1" }) };

	char *argv3[] = { "delete-agent-ep", "-i", "eth0", "-j",
			  QUOTE({ "tunnel_id": "3" }) };

	char *argv4[] = { "delete-agent-ep", "-i", "eth0", "-j",
			  QUOTE({ "tunnel_id": "3", "ip": [10.0.0.2] }) };

	struct rpc_trn_endpoint_key_t exp_ep_key = {
		.interface = itf,
		.ip = 0x100000a,
		.tunid = 3,
	};

	int delete_agent_ep_1_ret_val = 0;
	/* Test call delete_agent_ep_1 successfully */
	TEST_CASE(
		"delete_agent_ep_1 succeed with well formed endpoint json input");
	expect_function_call(__wrap_delete_agent_ep_1);
	will_return(__wrap_delete_agent_ep_1, &delete_agent_ep_1_ret_val);
	expect_check(__wrap_delete_agent_ep_1, argp, check_ep_key_equal,
		     &exp_ep_key);
	expect_any(__wrap_delete_agent_ep_1, clnt);
	rc = trn_cli_delete_agent_ep_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, 0);

	/* Test parse ep input error*/
	TEST_CASE("delete_agent_ep_1 is not called with non-string field");
	rc = trn_cli_delete_agent_ep_subcmd(NULL, argc, argv2);
	assert_int_equal(rc, -EINVAL);

	TEST_CASE(
		"delete_agent_ep_1 is not called with missing required field");
	rc = trn_cli_delete_agent_ep_subcmd(NULL, argc, argv3);
	assert_int_equal(rc, -EINVAL);

	/* Test parse ep input error 2*/
	TEST_CASE("delete_agent_ep_1 is not called malformed json");
	rc = trn_cli_delete_agent_ep_subcmd(NULL, argc, argv4);
	assert_int_equal(rc, -EINVAL);

	/* Test call delete_ep_1 return error*/
	TEST_CASE(
		"delete-agent-ep subcommand fails if delete_agent_ep_1 returns error");
	delete_agent_ep_1_ret_val = -EINVAL;
	expect_function_call(__wrap_delete_agent_ep_1);
	will_return(__wrap_delete_agent_ep_1, &delete_agent_ep_1_ret_val);
	expect_any(__wrap_delete_agent_ep_1, argp);
	expect_any(__wrap_delete_agent_ep_1, clnt);
	rc = trn_cli_delete_agent_ep_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);

	/* Test call delete_agent_ep_1 return NULL*/
	TEST_CASE(
		"delete-agent-ep subcommand fails if delete_agent_ep_1 returns NULL");
	expect_function_call(__wrap_delete_agent_ep_1);
	will_return(__wrap_delete_agent_ep_1, NULL);
	expect_any(__wrap_delete_agent_ep_1, argp);
	expect_any(__wrap_delete_agent_ep_1, clnt);
	rc = trn_cli_delete_agent_ep_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);
}

static void test_trn_cli_delete_packet_metadata_subcmd(void **state)
{
	UNUSED(state);
	int rc;
	int argc = 5;

	char itf[] = "eth0";

	/* Test cases */
	char *argv1[] = { "delete-packet-metadata", "-i", "eth0", "-j",
			  QUOTE({ "tunnel_id": "3", "ip": "10.0.0.1" }) };

	char *argv2[] = { "delete-packet-metadata", "-i", "eth0", "-j",
			  QUOTE({ "tunnel_id": 3, "ip": "10.0.0.1" }) };

	char *argv3[] = { "delete-packet-metadata", "-i", "eth0", "-j",
			  QUOTE({ "tunnel_id": "3" }) };

	char *argv4[] = { "delete-packet-metadata", "-i", "eth0", "-j",
			  QUOTE({ "tunnel_id": "3", "ip": [10.0.0.2] }) };

	struct rpc_trn_packet_metadata_key_t exp_packet_metadata_key = {
		.interface = itf,
		.ip = 0x100000a,
		.tunid = 3,
	};

	int delete_packet_metadata_1_ret_val = 0;
	/* Test call delete_packet_metadata_1 successfully */
	TEST_CASE(
		"delete_packet_metadata_1 succeed with well formed endpoint json input");
	expect_function_call(__wrap_delete_packet_metadata_1);
	will_return(__wrap_delete_packet_metadata_1, &delete_packet_metadata_1_ret_val);
	expect_check(__wrap_delete_packet_metadata_1, argp, check_packet_metadata_key_equal,
		     &exp_packet_metadata_key);
	expect_any(__wrap_delete_packet_metadata_1, clnt);
	rc = trn_cli_delete_packet_metadata_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, 0);

	/* Test parse ep input error*/
	TEST_CASE("delete_packet_metadata_1 is not called with non-string field");
	rc = trn_cli_delete_packet_metadata_subcmd(NULL, argc, argv2);
	assert_int_equal(rc, -EINVAL);

	TEST_CASE(
		"delete_packet_metadata_1 is not called with missing required field");
	rc = trn_cli_delete_packet_metadata_subcmd(NULL, argc, argv3);
	assert_int_equal(rc, -EINVAL);

	/* Test parse ep input error 2*/
	TEST_CASE("delete_packet_metadata_1 is not called malformed json");
	rc = trn_cli_delete_packet_metadata_subcmd(NULL, argc, argv4);
	assert_int_equal(rc, -EINVAL);

	/* Test call delete_ep_1 return error*/
	TEST_CASE(
		"delete-packet-metadata subcommand fails if delete_packet_metadata_1 returns error");
	delete_packet_metadata_1_ret_val = -EINVAL;
	expect_function_call(__wrap_delete_packet_metadata_1);
	will_return(__wrap_delete_packet_metadata_1, &delete_packet_metadata_1_ret_val);
	expect_any(__wrap_delete_packet_metadata_1, argp);
	expect_any(__wrap_delete_packet_metadata_1, clnt);
	rc = trn_cli_delete_packet_metadata_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);

	/* Test call delete_packet_metadata_1 return NULL*/
	TEST_CASE(
		"delete-packet-metadata subcommand fails if delete_packet_metadata_1 returns NULL");
	expect_function_call(__wrap_delete_packet_metadata_1);
	will_return(__wrap_delete_packet_metadata_1, NULL);
	expect_any(__wrap_delete_packet_metadata_1, argp);
	expect_any(__wrap_delete_packet_metadata_1, clnt);
	rc = trn_cli_delete_packet_metadata_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);
}

static void test_trn_cli_delete_agent_md_subcmd(void **state)
{
	UNUSED(state);
	int rc;
	int argc = 5;

	char itf[] = "eth0";

	/* Test cases */
	char *argv1[] = { "delete-agent-metadata", "-i", "eth0", "-j", QUOTE({
				  "": "",
			  }) };

	rpc_intf_t exp_md_itf = { .interface = itf };
	int delete_agent_md_1_ret_val = 0;
	TEST_CASE(
		"delete_agent_md succeed with well formed agent metadata json input");
	expect_function_call(__wrap_delete_agent_md_1);
	will_return(__wrap_delete_agent_md_1, &delete_agent_md_1_ret_val);
	expect_check(__wrap_delete_agent_md_1, argp, check_md_itf_equal,
		     &exp_md_itf);
	expect_any(__wrap_delete_agent_md_1, clnt);
	rc = trn_cli_delete_agent_md_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, 0);

	TEST_CASE(
		"delete-agent-metadata subcommand fails if rpc returns error");
	delete_agent_md_1_ret_val = -EINVAL;
	expect_function_call(__wrap_delete_agent_md_1);
	will_return(__wrap_delete_agent_md_1, &delete_agent_md_1_ret_val);
	expect_check(__wrap_delete_agent_md_1, argp, check_md_itf_equal,
		     &exp_md_itf);
	expect_any(__wrap_delete_agent_md_1, clnt);
	rc = trn_cli_delete_agent_md_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);

	TEST_CASE("delete-agent-metadata subcommand fails if rpc returns NULL");
	expect_function_call(__wrap_delete_agent_md_1);
	will_return(__wrap_delete_agent_md_1, NULL);
	expect_check(__wrap_delete_agent_md_1, argp, check_md_itf_equal,
		     &exp_md_itf);
	expect_any(__wrap_delete_agent_md_1, clnt);
	rc = trn_cli_delete_agent_md_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);
}

static void test_trn_cli_update_transit_network_policy_subcmd(void **state)
{
	UNUSED(state);
	int rc;
	int argc = 5;
	int update_transit_network_policy_1_ret_val = 0;

	/* Test cases */
	char *argv1[] = { "update-net-policy-in", "-i", "eth0", "-j", QUOTE([{
				  "tunnel_id": "3",
				  "local_ip": "10.0.0.3",
				  "cidr_prefixlen": "16",
				  "cidr_ip": "172.0.0.9",
				  "cidr_type": "1",
				  "bit_value": "10"
			  },
			  {
				  "tunnel_id": "1",
				  "local_ip": "10.0.0.1",
				  "cidr_prefixlen": "17",
				  "cidr_ip": "172.0.0.6",
				  "cidr_type": "2",
				  "bit_value": "1"
			  }]) };

	char *argv2[] = { "update-net-policy-in", "-i", "eth0", "-j", QUOTE([{
				  "tunnel_id": "3",
				  "local_ip": 10.0.0.3,
				  "cidr_prefixlen": "16",
				  "cidr_ip": "172.0.0.9",
				  "cidr_type": "1",
				  "bit_value": "10"
			  },
			  {
				  "tunnel_id": "1",
				  "local_ip": 10.0.0.1,
				  "cidr_prefixlen": "17",
				  "cidr_ip": "172.0.0.6",
				  "cidr_type": "2",
				  "bit_value": "1"
			  }]) };
	char itf[] = "eth0";

	struct rpc_trn_vsip_cidr_t policies[2] = {{
		.interface = itf,
		.tunid = 3,
		.local_ip = 0x300000a,
		.cidr_prefixlen = 16,
		.cidr_ip = 0x90000ac,
		.cidr_type = 1,
		.bit_val = 10
	},
	{
		.interface = itf,
		.tunid = 1,
		.local_ip = 0x100000a,
		.cidr_prefixlen = 17,
		.cidr_ip = 0x60000ac,
		.cidr_type = 2,
		.bit_val = 1
	}};

	/* Test call update_transit_network_policy successfully */
	TEST_CASE("update-net-policy-in succeed with well formed policy json input");
	update_transit_network_policy_1_ret_val = 0;
	expect_function_call(__wrap_update_transit_network_policy_1);
	will_return(__wrap_update_transit_network_policy_1, &update_transit_network_policy_1_ret_val);
	expect_check(__wrap_update_transit_network_policy_1, policy, check_policy_equal, policies);
	expect_any(__wrap_update_transit_network_policy_1, clnt);
	rc = trn_cli_update_transit_network_policy_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, 0);

	/* Test parse network policy input error 2*/
	TEST_CASE("update-net-policy-in is not called malformed json");
	rc = trn_cli_update_transit_network_policy_subcmd(NULL, argc, argv2);
	assert_int_equal(rc, -EINVAL);

	/* Test call update_transit_network_policy_1 return error*/
	TEST_CASE("update-net-policy-in subcommand fails if update_transit_network_policy_1 returns error");
	update_transit_network_policy_1_ret_val = -EINVAL;
	expect_function_call(__wrap_update_transit_network_policy_1);
	will_return(__wrap_update_transit_network_policy_1, &update_transit_network_policy_1_ret_val);
	expect_any(__wrap_update_transit_network_policy_1, policy);
	expect_any(__wrap_update_transit_network_policy_1, clnt);
	rc = trn_cli_update_transit_network_policy_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);

	/* Test call update_transit_network_policy_1 return NULL*/
	TEST_CASE("update-net-policy-in subcommand fails if update_transit_network_policy_1 returns NULL");
	expect_function_call(__wrap_update_transit_network_policy_1);
	will_return(__wrap_update_transit_network_policy_1, NULL);
	expect_any(__wrap_update_transit_network_policy_1, policy);
	expect_any(__wrap_update_transit_network_policy_1, clnt);
	rc = trn_cli_update_transit_network_policy_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);
}

static void test_trn_cli_update_agent_network_policy_subcmd(void **state)
{
	UNUSED(state);
	int rc;
	int argc = 5;
	int update_agent_network_policy_1_ret_val = 0;

	/* Test cases */
	char *argv1[] = { "update-net-policy-out", "-i", "eth0", "-j", QUOTE([{
				  "tunnel_id": "3",
				  "local_ip": "10.0.0.3",
				  "cidr_prefixlen": "16",
				  "cidr_ip": "172.0.0.9",
				  "cidr_type": "1",
				  "bit_value": "10"
			  },
			  {
				  "tunnel_id": "1",
				  "local_ip": "10.0.0.1",
				  "cidr_prefixlen": "17",
				  "cidr_ip": "172.0.0.6",
				  "cidr_type": "2",
				  "bit_value": "1"
			  }]) };

	char *argv2[] = { "update-net-policy-out", "-i", "eth0", "-j", QUOTE([{
				  "tunnel_id": "3",
				  "local_ip": 10.0.0.3,
				  "cidr_prefixlen": "16",
				  "cidr_ip": "172.0.0.9",
				  "cidr_type": "1",
				  "bit_value": "10"
			  },
			  {
				  "tunnel_id": "1",
				  "local_ip": 10.0.0.1,
				  "cidr_prefixlen": "17",
				  "cidr_ip": "172.0.0.6",
				  "cidr_type": "2",
				  "bit_value": "1"
			  }]) };
	char itf[] = "eth0";

	struct rpc_trn_vsip_cidr_t policies[2] = {{
		.interface = itf,
		.tunid = 3,
		.local_ip = 0x300000a,
		.cidr_prefixlen = 16,
		.cidr_ip = 0x90000ac,
		.cidr_type = 1,
		.bit_val = 10
	},
	{
		.interface = itf,
		.tunid = 1,
		.local_ip = 0x100000a,
		.cidr_prefixlen = 17,
		.cidr_ip = 0x60000ac,
		.cidr_type = 2,
		.bit_val = 1
	}};

	/* Test call update_agent_network_policy successfully */
	TEST_CASE("update-net-policy-out succeed with well formed policy json input");
	update_agent_network_policy_1_ret_val = 0;
	expect_function_call(__wrap_update_agent_network_policy_1);
	will_return(__wrap_update_agent_network_policy_1, &update_agent_network_policy_1_ret_val);
	expect_check(__wrap_update_agent_network_policy_1, policy, check_policy_equal, policies);
	expect_any(__wrap_update_agent_network_policy_1, clnt);
	rc = trn_cli_update_agent_network_policy_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, 0);

	/* Test parse network policy input error 2*/
	TEST_CASE("update-net-policy-out is not called malformed json");
	rc = trn_cli_update_agent_network_policy_subcmd(NULL, argc, argv2);
	assert_int_equal(rc, -EINVAL);

	/* Test call update_agent_network_policy_1 return error*/
	TEST_CASE("update-net-policy-out subcommand fails if update_agent_network_policy_1 returns error");
	update_agent_network_policy_1_ret_val = -EINVAL;
	expect_function_call(__wrap_update_agent_network_policy_1);
	will_return(__wrap_update_agent_network_policy_1, &update_agent_network_policy_1_ret_val);
	expect_any(__wrap_update_agent_network_policy_1, policy);
	expect_any(__wrap_update_agent_network_policy_1, clnt);
	rc = trn_cli_update_agent_network_policy_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);

	/* Test call update_agent_network_policy_1 return NULL*/
	TEST_CASE("update-net-policy-out subcommand fails if update_agent_network_policy_1 returns NULL");
	expect_function_call(__wrap_update_agent_network_policy_1);
	will_return(__wrap_update_agent_network_policy_1, NULL);
	expect_any(__wrap_update_agent_network_policy_1, policy);
	expect_any(__wrap_update_agent_network_policy_1, clnt);
	rc = trn_cli_update_agent_network_policy_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);
}

static void test_trn_cli_delete_transit_network_policy_subcmd(void **state)
{
	UNUSED(state);
	int rc;
	int argc = 5;
	char itf[] = "eth0";
	int delete_transit_network_policy_1_ret_val;

	/* Test cases */
	char *argv1[] = { "delete-network-policy-in", "-i", "eth0", "-j", QUOTE([{
				  "tunnel_id": "3",
				  "local_ip": "10.0.0.3",
				  "cidr_prefixlen": "16",
				  "cidr_ip": "172.0.0.9",
				  "cidr_type": "1"
			  },
			  {
				  "tunnel_id": "1",
				  "local_ip": "10.0.0.1",
				  "cidr_prefixlen": "17",
				  "cidr_ip": "172.0.0.6",
				  "cidr_type": "2"
			  }]) };

	char *argv2[] = { "delete-network-policy-in", "-i", "eth0", "-j", QUOTE([{
				  "tunnel_id": "3",
				  "local_ip": 10.0.0.3,
				  "cidr_prefixlen": "16",
				  "cidr_ip": "172.0.0.9",
				  "cidr_type": "1"
			  },
			  {
				  "tunnel_id": "1",
				  "local_ip": 10.0.0.1,
				  "cidr_prefixlen": "17",
				  "cidr_ip": "172.0.0.6",
				  "cidr_type": "2"
			  }]) };

	struct rpc_trn_vsip_cidr_key_t exp_policy_key[2] = {{
		.interface = itf,
		.tunid = 3,
		.local_ip = 0x300000a,
		.cidr_prefixlen = 16,
		.cidr_ip = 0x90000ac,
		.cidr_type = 1
	},
	{
		.interface = itf,
		.tunid = 1,
		.local_ip = 0x100000a,
		.cidr_prefixlen = 17,
		.cidr_ip = 0x60000ac,
		.cidr_type = 2
	}};

	/* Test call delete_transit_network_policy successfully */
	TEST_CASE("delete-network-policy-in succeed with well formed policy json input");
	delete_transit_network_policy_1_ret_val = 0;
	expect_function_call(__wrap_delete_transit_network_policy_1);
	will_return(__wrap_delete_transit_network_policy_1, &delete_transit_network_policy_1_ret_val);
	expect_check(__wrap_delete_transit_network_policy_1, policy, check_policy_key_equal, exp_policy_key);
	expect_any(__wrap_delete_transit_network_policy_1, clnt);
	rc = trn_cli_delete_transit_network_policy_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, 0);

	/* Test parse network policy input error 2*/
	TEST_CASE("delete-network-policy-in is not called malformed json");
	rc = trn_cli_delete_transit_network_policy_subcmd(NULL, argc, argv2);
	assert_int_equal(rc, -EINVAL);

	/* Test call delete_transit_network_policy_1 return error*/
	TEST_CASE("delete-network-policy-in subcommand fails if delete_transit_network_policy_1 returns error");
	delete_transit_network_policy_1_ret_val = -EINVAL;
	expect_function_call(__wrap_delete_transit_network_policy_1);
	will_return(__wrap_delete_transit_network_policy_1, &delete_transit_network_policy_1_ret_val);
	expect_any(__wrap_delete_transit_network_policy_1, policy);
	expect_any(__wrap_delete_transit_network_policy_1, clnt);
	rc = trn_cli_delete_transit_network_policy_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);

	/* Test call delete_transit_network_policy_1 return NULL*/
	TEST_CASE("delete-network-policy-in subcommand fails if delete_transit_network_policy_1 returns NULL");
	expect_function_call(__wrap_delete_transit_network_policy_1);
	will_return(__wrap_delete_transit_network_policy_1, NULL);
	expect_any(__wrap_delete_transit_network_policy_1, policy);
	expect_any(__wrap_delete_transit_network_policy_1, clnt);
	rc = trn_cli_delete_transit_network_policy_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);
}

static void test_trn_cli_delete_agent_network_policy_subcmd(void **state)
{
	UNUSED(state);
	int rc;
	int argc = 5;
	char itf[] = "eth0";
	int delete_agent_network_policy_1_ret_val;

	/* Test cases */
	char *argv1[] = { "delete-network-policy-out", "-i", "eth0", "-j", QUOTE([{
				  "tunnel_id": "3",
				  "local_ip": "10.0.0.3",
				  "cidr_prefixlen": "16",
				  "cidr_ip": "172.0.0.9",
				  "cidr_type": "1"
			  },
			  {
				  "tunnel_id": "1",
				  "local_ip": "10.0.0.1",
				  "cidr_prefixlen": "17",
				  "cidr_ip": "172.0.0.6",
				  "cidr_type": "2"
			  }]) };

	char *argv2[] = { "delete-network-policy-out", "-i", "eth0", "-j", QUOTE([{
				  "tunnel_id": "3",
				  "local_ip": 10.0.0.3,
				  "cidr_prefixlen": "16",
				  "cidr_ip": "172.0.0.9",
				  "cidr_type": "1"
			  },
			  {
				  "tunnel_id": "1",
				  "local_ip": 10.0.0.1,
				  "cidr_prefixlen": "17",
				  "cidr_ip": "172.0.0.6",
				  "cidr_type": "2"
			  }]) };

	struct rpc_trn_vsip_cidr_key_t exp_policy_key[2] = {{
		.interface = itf,
		.tunid = 3,
		.local_ip = 0x300000a,
		.cidr_prefixlen = 16,
		.cidr_ip = 0x90000ac,
		.cidr_type = 1
	},
	{
		.interface = itf,
		.tunid = 1,
		.local_ip = 0x100000a,
		.cidr_prefixlen = 17,
		.cidr_ip = 0x60000ac,
		.cidr_type = 2
	}};

	/* Test call delete_agent_network_policy successfully */
	TEST_CASE("delete-network-policy-out succeed with well formed policy json input");
	delete_agent_network_policy_1_ret_val = 0;
	expect_function_call(__wrap_delete_agent_network_policy_1);
	will_return(__wrap_delete_agent_network_policy_1, &delete_agent_network_policy_1_ret_val);
	expect_check(__wrap_delete_agent_network_policy_1, policy, check_policy_key_equal, exp_policy_key);
	expect_any(__wrap_delete_agent_network_policy_1, clnt);
	rc = trn_cli_delete_agent_network_policy_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, 0);

	/* Test parse network policy input error 2*/
	TEST_CASE("delete-network-policy-out is not called malformed json");
	rc = trn_cli_delete_agent_network_policy_subcmd(NULL, argc, argv2);
	assert_int_equal(rc, -EINVAL);

	/* Test call delete_transit_network_policy_1 return error*/
	TEST_CASE("delete-network-policy-out subcommand fails if delete_transit_network_policy_1 returns error");
	delete_agent_network_policy_1_ret_val = -EINVAL;
	expect_function_call(__wrap_delete_agent_network_policy_1);
	will_return(__wrap_delete_agent_network_policy_1, &delete_agent_network_policy_1_ret_val);
	expect_any(__wrap_delete_agent_network_policy_1, policy);
	expect_any(__wrap_delete_agent_network_policy_1, clnt);
	rc = trn_cli_delete_agent_network_policy_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);

	/* Test call delete_transit_network_policy_1 return NULL*/
	TEST_CASE("delete-network-policy-out subcommand fails if delete_transit_network_policy_1 returns NULL");
	expect_function_call(__wrap_delete_agent_network_policy_1);
	will_return(__wrap_delete_agent_network_policy_1, NULL);
	expect_any(__wrap_delete_agent_network_policy_1, policy);
	expect_any(__wrap_delete_agent_network_policy_1, clnt);
	rc = trn_cli_delete_agent_network_policy_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);
}

static void test_trn_cli_update_transit_network_policy_enforcement_subcmd(void **state)
{
	UNUSED(state);
	int rc;
	int argc = 5;
	int update_transit_network_policy_enforcement_1_ret_val = 0;

	/* Test cases */
	char *argv1[] = { "update-net-policy-enforce-in", "-i", "eth0", "-j", QUOTE({
				  "tunnel_id": "3",
				  "ip": "10.0.0.3"
			  }) };

	char *argv2[] = { "update-net-policy-enforce-in", "-i", "eth0", "-j", QUOTE({
				  "tunnel_id": "3",
				  "ip": 10.0.0.3
			  }) };
	char itf[] = "eth0";

	struct rpc_trn_vsip_enforce_t exp_enforce = {
		.interface = itf,
		.tunid = 3,
		.local_ip = 0x300000a
	};

	/* Test call update_transit_network_policy_enforcement successfully */
	TEST_CASE("update-net-policy-enforce-in succeed with well formed policy json input");
	update_transit_network_policy_enforcement_1_ret_val = 0;
	expect_function_call(__wrap_update_transit_network_policy_enforcement_1);
	will_return(__wrap_update_transit_network_policy_enforcement_1, &update_transit_network_policy_enforcement_1_ret_val);
	expect_check(__wrap_update_transit_network_policy_enforcement_1, enforce, check_policy_enforcement_equal, &exp_enforce);
	expect_any(__wrap_update_transit_network_policy_enforcement_1, clnt);
	rc = trn_cli_update_transit_network_policy_enforcement_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, 0);

	/* Test parse network policy input error 2*/
	TEST_CASE("update-net-policy-enforce-in is not called malformed json");
	rc = trn_cli_update_transit_network_policy_enforcement_subcmd(NULL, argc, argv2);
	assert_int_equal(rc, -EINVAL);

	/* Test call update_transit_network_policy_enforcement_1 return error*/
	TEST_CASE("update-net-policy-enforce-in subcommand fails if update_transit_network_policy_enforcement_1 returns error");
	update_transit_network_policy_enforcement_1_ret_val = -EINVAL;
	expect_function_call(__wrap_update_transit_network_policy_enforcement_1);
	will_return(__wrap_update_transit_network_policy_enforcement_1, &update_transit_network_policy_enforcement_1_ret_val);
	expect_any(__wrap_update_transit_network_policy_enforcement_1, enforce);
	expect_any(__wrap_update_transit_network_policy_enforcement_1, clnt);
	rc = trn_cli_update_transit_network_policy_enforcement_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);

	/* Test call update_transit_network_policy_enforcement_1 return NULL*/
	TEST_CASE("update-net-policy-enforce-in subcommand fails if update_transit_network_policy_enforcement_1 returns NULL");
	expect_function_call(__wrap_update_transit_network_policy_enforcement_1);
	will_return(__wrap_update_transit_network_policy_enforcement_1, NULL);
	expect_any(__wrap_update_transit_network_policy_enforcement_1, enforce);
	expect_any(__wrap_update_transit_network_policy_enforcement_1, clnt);
	rc = trn_cli_update_transit_network_policy_enforcement_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);
}

static void test_trn_cli_update_agent_network_policy_enforcement_subcmd(void **state)
{
	UNUSED(state);
	int rc;
	int argc = 5;
	int update_agent_network_policy_enforcement_1_ret_val = 0;

	/* Test cases */
	char *argv1[] = { "update-net-policy-enforce-out", "-i", "eth0", "-j", QUOTE({
				  "tunnel_id": "3",
				  "ip": "10.0.0.3"
			  }) };

	char *argv2[] = { "update-net-policy-enforce-out", "-i", "eth0", "-j", QUOTE({
				  "tunnel_id": "3",
				  "ip": 10.0.0.3
			  }) };
	char itf[] = "eth0";

	struct rpc_trn_vsip_enforce_t exp_enforce = {
		.interface = itf,
		.tunid = 3,
		.local_ip = 0x300000a
	};

	/* Test call update_agent_network_policy_enforcement successfully */
	TEST_CASE("update-net-policy-enforce-out succeed with well formed policy json input");
	update_agent_network_policy_enforcement_1_ret_val = 0;
	expect_function_call(__wrap_update_agent_network_policy_enforcement_1);
	will_return(__wrap_update_agent_network_policy_enforcement_1, &update_agent_network_policy_enforcement_1_ret_val);
	expect_check(__wrap_update_agent_network_policy_enforcement_1, enforce, check_policy_enforcement_equal, &exp_enforce);
	expect_any(__wrap_update_agent_network_policy_enforcement_1, clnt);
	rc = trn_cli_update_agent_network_policy_enforcement_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, 0);

	/* Test parse network policy input error 2*/
	TEST_CASE("update-net-policy-enforce-out is not called malformed json");
	rc = trn_cli_update_agent_network_policy_enforcement_subcmd(NULL, argc, argv2);
	assert_int_equal(rc, -EINVAL);

	/* Test call update_agent_network_policy_enforcement_1 return error*/
	TEST_CASE("update-net-policy-enforce-out subcommand fails if update_transit_network_policy_enforcement_1 returns error");
	update_agent_network_policy_enforcement_1_ret_val = -EINVAL;
	expect_function_call(__wrap_update_agent_network_policy_enforcement_1);
	will_return(__wrap_update_agent_network_policy_enforcement_1, &update_agent_network_policy_enforcement_1_ret_val);
	expect_any(__wrap_update_agent_network_policy_enforcement_1, enforce);
	expect_any(__wrap_update_agent_network_policy_enforcement_1, clnt);
	rc = trn_cli_update_agent_network_policy_enforcement_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);

	/* Test call update_agent_network_policy_enforcement_1 return NULL*/
	TEST_CASE("update-net-policy-enforce-out subcommand fails if update_agent_network_policy_enforcement_1 returns NULL");
	expect_function_call(__wrap_update_agent_network_policy_enforcement_1);
	will_return(__wrap_update_agent_network_policy_enforcement_1, NULL);
	expect_any(__wrap_update_agent_network_policy_enforcement_1, enforce);
	expect_any(__wrap_update_agent_network_policy_enforcement_1, clnt);
	rc = trn_cli_update_agent_network_policy_enforcement_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);
}

static void test_trn_cli_delete_transit_network_policy_enforcement_subcmd(void **state)
{
	UNUSED(state);
	int rc;
	int argc = 5;
	char itf[] = "eth0";
	int delete_transit_network_policy_enforcement_1_ret_val;

	/* Test cases */
	char *argv1[] = { "delete-net-policy-enforce-in", "-i", "eth0", "-j", QUOTE({
				  "tunnel_id": "3",
				  "ip": "10.0.0.3"
			  }) };

	char *argv2[] = { "delete-net-policy-enforce-in", "-i", "eth0", "-j", QUOTE({
				  "tunnel_id": "3",
				  "ip": 10.0.0.3
			  }) };

	struct rpc_trn_vsip_enforce_t exp_enforce = {
		.interface = itf,
		.tunid = 3,
		.local_ip = 0x300000a
	};

	/* Test call delete_transit_network_policy_enforcement successfully */
	TEST_CASE("delete-net-policy-enforce-in succeed with well formed policy json input");
	delete_transit_network_policy_enforcement_1_ret_val = 0;
	expect_function_call(__wrap_delete_transit_network_policy_enforcement_1);
	will_return(__wrap_delete_transit_network_policy_enforcement_1, &delete_transit_network_policy_enforcement_1_ret_val);
	expect_check(__wrap_delete_transit_network_policy_enforcement_1, enforce, check_policy_enforcement_equal, &exp_enforce);
	expect_any(__wrap_delete_transit_network_policy_enforcement_1, clnt);
	rc = trn_cli_delete_transit_network_policy_enforcement_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, 0);

	/* Test parse network policy input error 2*/
	TEST_CASE("delete-net-policy-enforce-in is not called malformed json");
	rc = trn_cli_delete_transit_network_policy_enforcement_subcmd(NULL, argc, argv2);
	assert_int_equal(rc, -EINVAL);

	/* Test call delete_transit_network_policy_enforcement_1 return error*/
	TEST_CASE("delete-net-policy-enforce-in subcommand fails if delete-network-policy-enforcement-ingress_1 returns error");
	delete_transit_network_policy_enforcement_1_ret_val = -EINVAL;
	expect_function_call(__wrap_delete_transit_network_policy_enforcement_1);
	will_return(__wrap_delete_transit_network_policy_enforcement_1, &delete_transit_network_policy_enforcement_1_ret_val);
	expect_any(__wrap_delete_transit_network_policy_enforcement_1, enforce);
	expect_any(__wrap_delete_transit_network_policy_enforcement_1, clnt);
	rc = trn_cli_delete_transit_network_policy_enforcement_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);

	/* Test call delete_transit_network_policy_enforcement_1 return NULL*/
	TEST_CASE("delete-net-policy-enforce-in subcommand fails if delete-network-policy-enforcement-ingress_1 returns NULL");
	expect_function_call(__wrap_delete_transit_network_policy_enforcement_1);
	will_return(__wrap_delete_transit_network_policy_enforcement_1, NULL);
	expect_any(__wrap_delete_transit_network_policy_enforcement_1, enforce);
	expect_any(__wrap_delete_transit_network_policy_enforcement_1, clnt);
	rc = trn_cli_delete_transit_network_policy_enforcement_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);
}

static void test_trn_cli_delete_agent_network_policy_enforcement_subcmd(void **state)
{
	UNUSED(state);
	int rc;
	int argc = 5;
	char itf[] = "eth0";
	int delete_agent_network_policy_enforcement_1_ret_val;

	/* Test cases */
	char *argv1[] = { "delete-net-policy-enforce-out", "-i", "eth0", "-j", QUOTE({
				  "tunnel_id": "3",
				  "ip": "10.0.0.3"
			  }) };

	char *argv2[] = { "delete-net-policy-enforce-out", "-i", "eth0", "-j", QUOTE({
				  "tunnel_id": "3",
				  "ip": 10.0.0.3
			  }) };

	struct rpc_trn_vsip_enforce_t exp_enforce = {
		.interface = itf,
		.tunid = 3,
		.local_ip = 0x300000a
	};

	/* Test call delete_agent_network_policy_enforcement successfully */
	TEST_CASE("delete-net-policy-enforce-out succeed with well formed policy json input");
	delete_agent_network_policy_enforcement_1_ret_val = 0;
	expect_function_call(__wrap_delete_agent_network_policy_enforcement_1);
	will_return(__wrap_delete_agent_network_policy_enforcement_1, &delete_agent_network_policy_enforcement_1_ret_val);
	expect_check(__wrap_delete_agent_network_policy_enforcement_1, enforce, check_policy_enforcement_equal, &exp_enforce);
	expect_any(__wrap_delete_agent_network_policy_enforcement_1, clnt);
	rc = trn_cli_delete_agent_network_policy_enforcement_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, 0);

	/* Test parse network policy input error 2*/
	TEST_CASE("delete-net-policy-enforce-out is not called malformed json");
	rc = trn_cli_delete_agent_network_policy_enforcement_subcmd(NULL, argc, argv2);
	assert_int_equal(rc, -EINVAL);

	/* Test call delete_agent_network_policy_enforcement_1 return error*/
	TEST_CASE("delete-net-policy-enforce-out subcommand fails if delete_agent_network_policy_enforcement_1 returns error");
	delete_agent_network_policy_enforcement_1_ret_val = -EINVAL;
	expect_function_call(__wrap_delete_agent_network_policy_enforcement_1);
	will_return(__wrap_delete_agent_network_policy_enforcement_1, &delete_agent_network_policy_enforcement_1_ret_val);
	expect_any(__wrap_delete_agent_network_policy_enforcement_1, enforce);
	expect_any(__wrap_delete_agent_network_policy_enforcement_1, clnt);
	rc = trn_cli_delete_agent_network_policy_enforcement_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);

	/* Test call delete_agent_network_policy_enforcement_1 return NULL*/
	TEST_CASE("delete-net-policy-enforce-out subcommand fails if delete_agent_network_policy_enforcement_1 returns NULL");
	expect_function_call(__wrap_delete_agent_network_policy_enforcement_1);
	will_return(__wrap_delete_agent_network_policy_enforcement_1, NULL);
	expect_any(__wrap_delete_agent_network_policy_enforcement_1, enforce);
	expect_any(__wrap_delete_agent_network_policy_enforcement_1, clnt);
	rc = trn_cli_delete_agent_network_policy_enforcement_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);
}

static void test_trn_cli_update_transit_network_policy_protocol_port_subcmd(void **state)
{
	UNUSED(state);
	int rc;
	int argc = 5;
	int update_transit_network_policy_proto_port_1_ret_val = 0;

	/* Test cases */
	char *argv1[] = { "update-net-policy-protocol-port-in", "-i", "eth0", "-j", QUOTE([{
				  "tunnel_id": "3",
				  "local_ip": "10.0.0.3",
				  "protocol": "6",
				  "port": "6379",
				  "bit_value": "10"
			  },
			  {
				  "tunnel_id": "3",
				  "local_ip": "10.0.0.3",
				  "protocol": "6",
				  "port": "6379",
				  "bit_value": "10"
			  }]) };

	char *argv2[] = { "update-net-policy-protocol-port-in", "-i", "eth0", "-j", QUOTE([{
				  "tunnel_id": "3",
				  "local_ip": 10.0.0.3,
				  "protocol": "6",
				  "port": "6379",
				  "bit_value": "10"
			  },
			  {
				  "tunnel_id": "3",
				  "local_ip": 10.0.0.3,
				  "protocol": "6",
				  "port": "6379",
				  "bit_value": "10"
			  }]) };
	char itf[] = "eth0";

	struct rpc_trn_vsip_ppo_t exp_ppo[2] = {{
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

	/* Test call update_transit_network_policy_protocol_port successfully */
	TEST_CASE("update-net-policy-protocol-port-in succeed with well formed policy json input");
	update_transit_network_policy_proto_port_1_ret_val = 0;
	expect_function_call(__wrap_update_transit_network_policy_protocol_port_1);
	will_return(__wrap_update_transit_network_policy_protocol_port_1, &update_transit_network_policy_proto_port_1_ret_val);
	expect_check(__wrap_update_transit_network_policy_protocol_port_1, ppo, check_policy_protocol_port_equal, exp_ppo);
	expect_any(__wrap_update_transit_network_policy_protocol_port_1, clnt);
	rc = trn_cli_update_transit_network_policy_protocol_port_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, 0);

	/* Test parse network policy input error 2*/
	TEST_CASE("update-net-policy-protocol-port-in is not called malformed json");
	rc = trn_cli_update_transit_network_policy_protocol_port_subcmd(NULL, argc, argv2);
	assert_int_equal(rc, -EINVAL);

	/* Test call update_transit_network_policy_1 return error*/
	TEST_CASE("update-net-policy-protocol-port-in subcommand fails if update_transit_network_policy_protocol_port_1 returns error");
	update_transit_network_policy_proto_port_1_ret_val = -EINVAL;
	expect_function_call(__wrap_update_transit_network_policy_protocol_port_1);
	will_return(__wrap_update_transit_network_policy_protocol_port_1, &update_transit_network_policy_proto_port_1_ret_val);
	expect_any(__wrap_update_transit_network_policy_protocol_port_1, ppo);
	expect_any(__wrap_update_transit_network_policy_protocol_port_1, clnt);
	rc = trn_cli_update_transit_network_policy_protocol_port_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);

	/* Test call update_transit_network_policy_1 return NULL*/
	TEST_CASE("update-net-policy-protocol-port-in subcommand fails if update_transit_network_policy_protocol_port_1 returns NULL");
	expect_function_call(__wrap_update_transit_network_policy_protocol_port_1);
	will_return(__wrap_update_transit_network_policy_protocol_port_1, NULL);
	expect_any(__wrap_update_transit_network_policy_protocol_port_1, ppo);
	expect_any(__wrap_update_transit_network_policy_protocol_port_1, clnt);
	rc = trn_cli_update_transit_network_policy_protocol_port_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);
}

static void test_trn_cli_update_agent_network_policy_protocol_port_subcmd(void **state)
{
	UNUSED(state);
	int rc;
	int argc = 5;
	int update_agent_network_policy_proto_port_1_ret_val = 0;

	/* Test cases */
	char *argv1[] = { "update-net-policy-protocol-port-out", "-i", "eth0", "-j", QUOTE([{
				  "tunnel_id": "3",
				  "local_ip": "10.0.0.3",
				  "protocol": "6",
				  "port": "6379",
				  "bit_value": "10"
			  },
			  {
				  "tunnel_id": "3",
				  "local_ip": "10.0.0.3",
				  "protocol": "6",
				  "port": "6379",
				  "bit_value": "10"
			  }]) };

	char *argv2[] = { "update-net-policy-protocol-port-out", "-i", "eth0", "-j", QUOTE([{
				  "tunnel_id": "3",
				  "local_ip": 10.0.0.3,
				  "protocol": "6",
				  "port": "6379",
				  "bit_value": "10"
			  },
			  {
				  "tunnel_id": "3",
				  "local_ip": 10.0.0.3,
				  "protocol": "6",
				  "port": "6379",
				  "bit_value": "10"
			  }]) };
	char itf[] = "eth0";

	struct rpc_trn_vsip_ppo_t exp_ppo[2] = {{
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

	/* Test call update_agent_network_policy_protocol_port successfully */
	TEST_CASE("update-net-policy-protocol-port-out succeed with well formed policy json input");
	update_agent_network_policy_proto_port_1_ret_val = 0;
	expect_function_call(__wrap_update_agent_network_policy_protocol_port_1);
	will_return(__wrap_update_agent_network_policy_protocol_port_1, &update_agent_network_policy_proto_port_1_ret_val);
	expect_check(__wrap_update_agent_network_policy_protocol_port_1, ppo, check_policy_protocol_port_equal, exp_ppo);
	expect_any(__wrap_update_agent_network_policy_protocol_port_1, clnt);
	rc = trn_cli_update_agent_network_policy_protocol_port_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, 0);

	/* Test parse network policy input error 2*/
	TEST_CASE("update-net-policy-protocol-port-out is not called malformed json");
	rc = trn_cli_update_agent_network_policy_protocol_port_subcmd(NULL, argc, argv2);
	assert_int_equal(rc, -EINVAL);

	/* Test call update_agent_network_policy_1 return error*/
	TEST_CASE("update-net-policy-protocol-port-out subcommand fails if update_agent_network_policy_protocol_port_1 returns error");
	update_agent_network_policy_proto_port_1_ret_val = -EINVAL;
	expect_function_call(__wrap_update_agent_network_policy_protocol_port_1);
	will_return(__wrap_update_agent_network_policy_protocol_port_1, &update_agent_network_policy_proto_port_1_ret_val);
	expect_any(__wrap_update_agent_network_policy_protocol_port_1, ppo);
	expect_any(__wrap_update_agent_network_policy_protocol_port_1, clnt);
	rc = trn_cli_update_agent_network_policy_protocol_port_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);

	/* Test call update_agent_network_policy_1 return NULL*/
	TEST_CASE("update-net-policy-protocol-port-out subcommand fails if update_agent_network_policy_protocol_port_1 returns NULL");
	expect_function_call(__wrap_update_agent_network_policy_protocol_port_1);
	will_return(__wrap_update_agent_network_policy_protocol_port_1, NULL);
	expect_any(__wrap_update_agent_network_policy_protocol_port_1, ppo);
	expect_any(__wrap_update_agent_network_policy_protocol_port_1, clnt);
	rc = trn_cli_update_agent_network_policy_protocol_port_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);
}

static void test_trn_cli_delete_transit_network_policy_protocol_port_subcmd(void **state)
{
	UNUSED(state);
	int rc;
	int argc = 5;
	char itf[] = "eth0";
	int delete_transit_network_policy_protocol_port_1_ret_val;

	/* Test cases */
	char *argv1[] = { "delete-net-policy-protocol-port-in", "-i", "eth0", "-j", QUOTE([{
				  "tunnel_id": "3",
				  "local_ip": "10.0.0.3",
				  "protocol": "6",
				  "port": "6379"
			  },
			  {
				  "tunnel_id": "2",
				  "local_ip": "10.0.0.2",
				  "protocol": "6",
				  "port": "6379"
			  }]) };

	char *argv2[] = { "delete-net-policy-protocol-port-in", "-i", "eth0", "-j", QUOTE([{
				  "tunnel_id": "3",
				  "local_ip": 10.0.0.3,
				  "protocol": "6",
				  "port": "6379"
			  },
			  {
				  "tunnel_id": "2",
				  "local_ip": 10.0.0.2,
				  "protocol": "6",
				  "port": "6379"
			  }]) };

	struct rpc_trn_vsip_ppo_t exp_ppo[2] = {{
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

	/* Test call delete_transit_network_policy_protocol_port successfully */
	TEST_CASE("delete-net-policy-protocol-port-in succeed with well formed policy json input");
	delete_transit_network_policy_protocol_port_1_ret_val = 0;
	expect_function_call(__wrap_delete_transit_network_policy_protocol_port_1);
	will_return(__wrap_delete_transit_network_policy_protocol_port_1, &delete_transit_network_policy_protocol_port_1_ret_val);
	expect_check(__wrap_delete_transit_network_policy_protocol_port_1, ppo, check_policy_protocol_port_key_equal, exp_ppo);
	expect_any(__wrap_delete_transit_network_policy_protocol_port_1, clnt);
	rc = trn_cli_delete_transit_network_policy_protocol_port_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, 0);

	/* Test parse network policy input error 2*/
	TEST_CASE("delete-net-policy-protocol-port-in is not called malformed json");
	rc = trn_cli_delete_transit_network_policy_protocol_port_subcmd(NULL, argc, argv2);
	assert_int_equal(rc, -EINVAL);

	/* Test call delete_transit_network_policy_protocol_port_1 return error*/
	TEST_CASE("delete-net-policy-protocol-port-in subcommand fails if delete_transit_network_policy_protocol_port_1 returns error");
	delete_transit_network_policy_protocol_port_1_ret_val = -EINVAL;
	expect_function_call(__wrap_delete_transit_network_policy_protocol_port_1);
	will_return(__wrap_delete_transit_network_policy_protocol_port_1, &delete_transit_network_policy_protocol_port_1_ret_val);
	expect_any(__wrap_delete_transit_network_policy_protocol_port_1, ppo);
	expect_any(__wrap_delete_transit_network_policy_protocol_port_1, clnt);
	rc = trn_cli_delete_transit_network_policy_protocol_port_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);

	/* Test call delete_transit_network_policy_protocol_port_1 return NULL*/
	TEST_CASE("delete-net-policy-protocol-port-in subcommand fails if delete_transit_network_policy_protocol_port_1 returns NULL");
	expect_function_call(__wrap_delete_transit_network_policy_protocol_port_1);
	will_return(__wrap_delete_transit_network_policy_protocol_port_1, NULL);
	expect_any(__wrap_delete_transit_network_policy_protocol_port_1, ppo);
	expect_any(__wrap_delete_transit_network_policy_protocol_port_1, clnt);
	rc = trn_cli_delete_transit_network_policy_protocol_port_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);
}

static void test_trn_cli_delete_agent_network_policy_protocol_port_subcmd(void **state)
{
	UNUSED(state);
	int rc;
	int argc = 5;
	char itf[] = "eth0";
	int delete_agent_network_policy_protocol_port_1_ret_val;

	/* Test cases */
	char *argv1[] = { "delete-net-policy-protocol-port-out", "-i", "eth0", "-j", QUOTE([{
				  "tunnel_id": "3",
				  "local_ip": "10.0.0.3",
				  "protocol": "6",
				  "port": "6379"
			  },
			  {
				  "tunnel_id": "2",
				  "local_ip": "10.0.0.2",
				  "protocol": "6",
				  "port": "6379"
			  }]) };

	char *argv2[] = { "delete-net-policy-protocol-port-out", "-i", "eth0", "-j", QUOTE([{
				  "tunnel_id": "3",
				  "local_ip": 10.0.0.3,
				  "protocol": "6",
				  "port": "6379"
			  },
			  {
				  "tunnel_id": "2",
				  "local_ip": 10.0.0.2,
				  "protocol": "6",
				  "port": "6379"
			  }]) };

	struct rpc_trn_vsip_ppo_t exp_ppo[2] = {{
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

	/* Test call delete_agent_network_policy_protocol_port successfully */
	TEST_CASE("delete-net-policy-protocol-port-out succeed with well formed policy json input");
	delete_agent_network_policy_protocol_port_1_ret_val = 0;
	expect_function_call(__wrap_delete_agent_network_policy_protocol_port_1);
	will_return(__wrap_delete_agent_network_policy_protocol_port_1, &delete_agent_network_policy_protocol_port_1_ret_val);
	expect_check(__wrap_delete_agent_network_policy_protocol_port_1, ppo, check_policy_protocol_port_key_equal, exp_ppo);
	expect_any(__wrap_delete_agent_network_policy_protocol_port_1, clnt);
	rc = trn_cli_delete_agent_network_policy_protocol_port_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, 0);

	/* Test parse network policy input error 2*/
	TEST_CASE("delete-net-policy-protocol-port-out is not called malformed json");
	rc = trn_cli_delete_agent_network_policy_protocol_port_subcmd(NULL, argc, argv2);
	assert_int_equal(rc, -EINVAL);

	/* Test call delete_agent_network_policy_protocol_port_1 return error*/
	TEST_CASE("delete-net-policy-protocol-port-out subcommand fails if delete_agent_network_policy_protocol_port_1 returns error");
	delete_agent_network_policy_protocol_port_1_ret_val = -EINVAL;
	expect_function_call(__wrap_delete_agent_network_policy_protocol_port_1);
	will_return(__wrap_delete_agent_network_policy_protocol_port_1, &delete_agent_network_policy_protocol_port_1_ret_val);
	expect_any(__wrap_delete_agent_network_policy_protocol_port_1, ppo);
	expect_any(__wrap_delete_agent_network_policy_protocol_port_1, clnt);
	rc = trn_cli_delete_agent_network_policy_protocol_port_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);

	/* Test call delete_agent_network_policy_protocol_port_1 return NULL*/
	TEST_CASE("delete-net-policy-protocol-port-out subcommand fails if delete_agent_network_policy_protocol_port_1 returns NULL");
	expect_function_call(__wrap_delete_agent_network_policy_protocol_port_1);
	will_return(__wrap_delete_agent_network_policy_protocol_port_1, NULL);
	expect_any(__wrap_delete_agent_network_policy_protocol_port_1, ppo);
	expect_any(__wrap_delete_agent_network_policy_protocol_port_1, clnt);
	rc = trn_cli_delete_agent_network_policy_protocol_port_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);
}

int main()
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_trn_cli_update_vpc_subcmd),
		cmocka_unit_test(test_trn_cli_update_net_subcmd),
		cmocka_unit_test(test_trn_cli_update_ep_subcmd),
		cmocka_unit_test(test_trn_cli_load_transit_subcmd),
		cmocka_unit_test(test_trn_cli_unload_transit_subcmd),
		cmocka_unit_test(test_trn_cli_load_agent_subcmd),
		cmocka_unit_test(test_trn_cli_unload_agent_subcmd),
		cmocka_unit_test(test_trn_cli_update_agent_ep_subcmd),
		cmocka_unit_test(test_trn_cli_update_packet_metadata_subcmd),
		cmocka_unit_test(test_trn_cli_update_agent_md_subcmd),
		cmocka_unit_test(test_trn_cli_get_vpc_subcmd),
		cmocka_unit_test(test_trn_cli_get_net_subcmd),
		cmocka_unit_test(test_trn_cli_get_ep_subcmd),
		cmocka_unit_test(test_trn_cli_get_agent_ep_subcmd),
		cmocka_unit_test(test_trn_cli_get_agent_md_subcmd),
		cmocka_unit_test(test_trn_cli_delete_vpc_subcmd),
		cmocka_unit_test(test_trn_cli_delete_net_subcmd),
		cmocka_unit_test(test_trn_cli_delete_ep_subcmd),
		cmocka_unit_test(test_trn_cli_delete_agent_ep_subcmd),
		cmocka_unit_test(test_trn_cli_delete_packet_metadata_subcmd),
		cmocka_unit_test(test_trn_cli_delete_agent_md_subcmd),
		cmocka_unit_test(test_trn_cli_update_transit_network_policy_subcmd),
		cmocka_unit_test(test_trn_cli_update_agent_network_policy_subcmd),
		cmocka_unit_test(test_trn_cli_delete_transit_network_policy_subcmd),
		cmocka_unit_test(test_trn_cli_delete_agent_network_policy_subcmd),
		cmocka_unit_test(test_trn_cli_update_transit_network_policy_enforcement_subcmd),
		cmocka_unit_test(test_trn_cli_update_agent_network_policy_enforcement_subcmd),
		cmocka_unit_test(test_trn_cli_delete_transit_network_policy_enforcement_subcmd),
		cmocka_unit_test(test_trn_cli_delete_agent_network_policy_enforcement_subcmd),
		cmocka_unit_test(test_trn_cli_update_transit_network_policy_protocol_port_subcmd),
		cmocka_unit_test(test_trn_cli_update_agent_network_policy_protocol_port_subcmd),
		cmocka_unit_test(test_trn_cli_delete_transit_network_policy_protocol_port_subcmd),
		cmocka_unit_test(test_trn_cli_delete_agent_network_policy_protocol_port_subcmd)
	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}
