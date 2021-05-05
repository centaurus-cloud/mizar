// SPDX-License-Identifier: GPL-2.0
/**
 * @file trn_rpc_protocol_handlers_1.c
 * @author Sherif Abdelwahab (@zasherif)
 *         Phu Tran          (@phudtran)
 *
 * @brief RPC handlers. Primarly allocate and populate data structs,
 * and update the ebpf maps through user space APIs.
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

#include <stdio.h>
#include <stdlib.h>
#include <rpc/pmap_clnt.h>
#include <string.h>
#include <memory.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <syslog.h>
#include <search.h>
#include <stdlib.h>
#include <stdint.h>

#include "rpcgen/trn_rpc_protocol.h"
#include "trn_transit_xdp_usr.h"
#include "trn_agent_xdp_usr.h"
#include "trn_log.h"
#include "trn_transitd.h"

#define TRANSITLOGNAME "transit"
#define TRN_MAX_ITF 265
#define TRN_MAX_VETH 2048
#define PRIMARY 0
#define SUPPLEMENTARY 1
#define EXCEPTION 2

void rpc_transit_remote_protocol_1(struct svc_req *rqstp,
				   register SVCXPRT *transp);

int trn_itf_table_init()
{
	int rc;
	rc = hcreate((TRN_MAX_VETH + TRN_MAX_ITF) * 1.3);
	return rc;
}

void trn_itf_table_free()
{
	/* TODO: At the moment, this is only called before exit, so there
     *  is no actual need to free table elements one by one. If this
     *  is being called while the dameon remains running, we will need
     *  to maintain the keys in a separate data-structure and free
     *  them one-by-one. */

	hdestroy();
}

int trn_itf_table_insert(char *itf, struct user_metadata_t *md)
{
	INTF_INSERT();
}

struct user_metadata_t *trn_itf_table_find(char *itf)
{
	INTF_FIND();
}

void trn_itf_table_delete(char *itf)
{
	INTF_DELETE();
}

int trn_vif_table_insert(char *itf, struct agent_user_metadata_t *md)
{
	INTF_INSERT();
}

struct agent_user_metadata_t *trn_vif_table_find(char *itf)
{
	INTF_FIND();
}

void trn_vif_table_delete(char *itf)
{
	INTF_DELETE();
}

int *update_vpc_1_svc(rpc_trn_vpc_t *vpc, struct svc_req *rqstp)
{
	UNUSED(rqstp);
	static int result = -1;
	result = 0;
	int rc;
	char *itf = vpc->interface;
	struct vpc_key_t vpckey;
	struct vpc_t vpcval;

	TRN_LOG_DEBUG(
		"update_vpc_1 VPC tunid: %ld with %d routers on interface: %s",
		vpc->tunid, vpc->routers_ips.routers_ips_len, vpc->interface);

	struct user_metadata_t *md = trn_itf_table_find(itf);

	if (!md) {
		TRN_LOG_ERROR("Cannot find interface metadata for %s", itf);
		result = RPC_TRN_ERROR;
		goto error;
	}

	vpckey.tunnel_id = vpc->tunid;
	vpcval.nrouters = vpc->routers_ips.routers_ips_len;

	if (vpcval.nrouters > TRAN_MAX_NROUTER) {
		TRN_LOG_WARN("Number of maximum transit routers exceeded,"
			     " configuring only the first %d transit routers.",
			     TRAN_MAX_NROUTER);
		vpcval.nrouters = TRAN_MAX_NROUTER;
		result = RPC_TRN_WARN;
	}
	memcpy(vpcval.routers_ips, vpc->routers_ips.routers_ips_val,
	       sizeof(vpcval.routers_ips[0]) * vpcval.nrouters);

	rc = trn_update_vpc(md, &vpckey, &vpcval);

	if (rc != 0) {
		TRN_LOG_ERROR("Failure updating vpc %ld data on interface: %s",
			      vpc->tunid, vpc->interface);
		result = RPC_TRN_FATAL;
		goto error;
	}

	return &result;

error:
	return &result;
}

int *update_net_1_svc(rpc_trn_network_t *net, struct svc_req *rqstp)
{
	UNUSED(rqstp);
	static int result = -1;
	result = 0;
	int rc;
	char *itf = net->interface;
	struct network_key_t netkey;
	struct network_t netval;

	TRN_LOG_DEBUG("update_net_1 net tunid: %ld, netip: 0x%x, "
		      "prefixlen: %d, with %d switches",
		      net->tunid, net->netip, net->prefixlen,
		      net->switches_ips.switches_ips_len);

	struct user_metadata_t *md = trn_itf_table_find(itf);

	if (!md) {
		TRN_LOG_ERROR("Cannot find interface metadata for %s", itf);
		result = RPC_TRN_ERROR;
		goto error;
	}

	netkey.prefixlen = net->prefixlen;
	memcpy(netkey.nip, &net->tunid, sizeof(net->tunid));
	netkey.nip[2] = net->netip;

	netval.nswitches = net->switches_ips.switches_ips_len;
	if (netval.nswitches > TRAN_MAX_NSWITCH) {
		TRN_LOG_WARN("Number of maximum transit switches exceeded,"
			     " configuring only the first %d transit switches.",
			     TRAN_MAX_NSWITCH);
		netval.nswitches = TRAN_MAX_NSWITCH;
	}
	memcpy(netval.switches_ips, net->switches_ips.switches_ips_val,
	       netval.nswitches * sizeof(netval.switches_ips[0]));

	netval.prefixlen = netkey.prefixlen;
	netval.nip[0] = netkey.nip[0];
	netval.nip[1] = netkey.nip[1];
	netval.nip[2] = netkey.nip[2];

	rc = trn_update_network(md, &netkey, &netval);

	if (rc != 0) {
		TRN_LOG_ERROR(
			"Failure updating net %ld, %d data on interface: %s",
			net->tunid, net->netip, itf);
		result = RPC_TRN_ERROR;
		goto error;
	}

	result = 0;
	return &result;

error:
	return &result;
}

int *update_ep_1_svc(rpc_trn_endpoint_t *ep, struct svc_req *rqstp)
{
	UNUSED(rqstp);
	static int result = -1;
	result = 0;
	int rc;
	char *itf = ep->interface;
	struct endpoint_key_t epkey;
	struct endpoint_t epval;

	TRN_LOG_DEBUG("update_ep_1 ep tunid: %ld, ip: 0x%x,"
		      " type: %d, veth: %s, hosted_interface:%s",
		      ep->tunid, ep->ip, ep->eptype, ep->veth,
		      ep->hosted_interface);

	struct user_metadata_t *md = trn_itf_table_find(itf);

	if (!md) {
		TRN_LOG_ERROR("Cannot find interface metadata for %s", itf);
		result = RPC_TRN_ERROR;
		goto error;
	}

	memcpy(epkey.tunip, &ep->tunid, sizeof(ep->tunid));
	epkey.tunip[2] = ep->ip;
	epval.eptype = ep->eptype;

	epval.nremote_ips = ep->remote_ips.remote_ips_len;
	if (epval.nremote_ips > TRAN_MAX_REMOTES) {
		TRN_LOG_WARN("Number of maximum remote IPs exceeded,"
			     " configuring only the first %d remote IPs.",
			     TRAN_MAX_REMOTES);
		result = RPC_TRN_WARN;
		goto error;
	}

	if (epval.nremote_ips > 0) {
		memcpy(epval.remote_ips, ep->remote_ips.remote_ips_val,
		       epval.nremote_ips * sizeof(epval.remote_ips[0]));
	}
	memcpy(epval.mac, ep->mac, 6 * sizeof(epval.mac[0]));

	if (strcmp(ep->hosted_interface, "") != 0) {
		epval.hosted_iface = if_nametoindex(ep->hosted_interface);
	} else {
		epval.hosted_iface = -1;
	}

	/* in case if_nameindex fails!! */
	if (!epval.hosted_iface) {
		TRN_LOG_WARN(
			"**Failed to map interface name [%s] to an index,"
			" for the given hosted_interface name traffic may"
			" not be routed correctly to ep [0x%x] hosted on this host.",
			ep->hosted_interface, ep->ip);
		result = RPC_TRN_WARN;
		epval.hosted_iface = -1;
	}

	rc = trn_update_endpoint(md, &epkey, &epval);

	if (rc != 0) {
		TRN_LOG_ERROR(
			"Cannot update transit XDP with ep %d on interface %s",
			epkey.tunip[2], itf);
		result = RPC_TRN_ERROR;
		goto error;
	}

	return &result;

error:
	return &result;
}

int *update_port_1_svc(rpc_trn_port_t *port, struct svc_req *rqstp)
{
	UNUSED(rqstp);
	static int result = -1;
	result = 0;
	int rc;
	char *itf = port->interface;
	struct port_key_t portkey;
	struct port_t portval;

	TRN_LOG_DEBUG(
		"update_port_1 Port tunid: %ld IP: 0x%x, with %u port for target_port: %u and protocol: %u",
		port->tunid, port->ip, port->port, port->target_port,
		port->protocol);

	struct user_metadata_t *md = trn_itf_table_find(itf);

	if (!md) {
		TRN_LOG_ERROR("Cannot find interface metadata for %s", itf);
		result = RPC_TRN_ERROR;
		goto error;
	}

	memcpy(portkey.tunip, &port->tunid, sizeof(port->tunid));
	portkey.tunip[2] = port->ip;
	portkey.port = port->port;
	portkey.protocol = port->protocol;

	portval.target_port = port->target_port;
	rc = trn_update_port(md, &portkey, &portval);

	if (rc != 0) {
		TRN_LOG_ERROR("Failure updating port %ld data on interface: %s",
			      port->tunid, port->interface);
		result = RPC_TRN_FATAL;
		goto error;
	}

	return &result;

error:
	return &result;
}

int *delete_vpc_1_svc(rpc_trn_vpc_key_t *argp, struct svc_req *rqstp)
{
	UNUSED(rqstp);
	static int result = -1;
	result = 0;
	int rc;
	struct vpc_key_t vpckey;

	TRN_LOG_DEBUG("delete_vpc_1 VPC tunid: %ld with on interface: %s",
		      argp->tunid, argp->interface);

	struct user_metadata_t *md = trn_itf_table_find(argp->interface);

	if (!md) {
		TRN_LOG_ERROR("Cannot find interface metadata for %s",
			      argp->interface);
		result = RPC_TRN_ERROR;
		goto error;
	}

	vpckey.tunnel_id = argp->tunid;

	rc = trn_delete_vpc(md, &vpckey);

	if (rc != 0) {
		TRN_LOG_ERROR("Failure deleting vpc %ld data on interface: %s",
			      argp->tunid, argp->interface);
		result = RPC_TRN_FATAL;
		goto error;
	}

	result = 0;

	return &result;
error:
	return &result;
}

int *delete_net_1_svc(rpc_trn_network_key_t *argp, struct svc_req *rqstp)
{
	UNUSED(rqstp);

	static int result = -1;
	result = 0;
	int rc;
	struct network_key_t netkey;
	char buffer[INET_ADDRSTRLEN];
	const char *parsed_ip =
		inet_ntop(AF_INET, &argp->netip, buffer, sizeof(buffer));

	TRN_LOG_DEBUG("delete_net_1 net tunid: %ld, netip: %s, "
		      "prefixlen: %d, on interface %s",
		      argp->tunid, parsed_ip, argp->prefixlen, argp->interface);

	struct user_metadata_t *md = trn_itf_table_find(argp->interface);

	if (!md) {
		TRN_LOG_ERROR("Cannot find interface metadata for %s",
			      argp->interface);
		result = RPC_TRN_ERROR;
		goto error;
	}

	netkey.prefixlen = argp->prefixlen;
	memcpy(netkey.nip, &argp->tunid, sizeof(argp->tunid));
	netkey.nip[2] = argp->netip;

	rc = trn_delete_network(md, &netkey);

	if (rc != 0) {
		TRN_LOG_ERROR(
			"Failure deleting net %ld, %d data on interface: %s",
			argp->tunid, argp->netip, argp->interface);
		result = RPC_TRN_ERROR;
		goto error;
	}

	return &result;
error:
	return &result;
}

int *delete_ep_1_svc(rpc_trn_endpoint_key_t *argp, struct svc_req *rqstp)
{
	UNUSED(rqstp);
	static int result = -1;
	result = 0;
	int rc;
	struct endpoint_key_t epkey;
	char buffer[INET_ADDRSTRLEN];
	const char *parsed_ip =
		inet_ntop(AF_INET, &argp->ip, buffer, sizeof(buffer));

	TRN_LOG_DEBUG("delete_ep_1 ep tunid: %ld, ip: 0x%x,"
		      " on interface: %s",
		      argp->tunid, parsed_ip, argp->interface);

	struct user_metadata_t *md = trn_itf_table_find(argp->interface);

	if (!md) {
		TRN_LOG_ERROR("Cannot find interface metadata for %s",
			      argp->interface);
		result = RPC_TRN_ERROR;
		goto error;
	}

	memcpy(epkey.tunip, &argp->tunid, sizeof(argp->tunid));
	epkey.tunip[2] = argp->ip;

	rc = trn_delete_endpoint(md, &epkey);

	if (rc != 0) {
		TRN_LOG_ERROR("Failure deleting ep %d on interface %s",
			      epkey.tunip[2], argp->interface);
		result = RPC_TRN_ERROR;
		goto error;
	}

	return &result;
error:
	return &result;
}

rpc_trn_vpc_t *get_vpc_1_svc(rpc_trn_vpc_key_t *argp, struct svc_req *rqstp)
{
	UNUSED(rqstp);
	static rpc_trn_vpc_t result;
	result.routers_ips.routers_ips_len = 0;
	int rc;
	struct vpc_key_t vpckey;
	struct vpc_t vpcval;

	TRN_LOG_DEBUG("get_vpc_1 VPC tunid: %ld on interface: %s", argp->tunid,
		      argp->interface);

	struct user_metadata_t *md = trn_itf_table_find(argp->interface);

	if (!md) {
		TRN_LOG_ERROR("Cannot find interface metadata for %s",
			      argp->interface);
		goto error;
	}
	vpckey.tunnel_id = argp->tunid;
	rc = trn_get_vpc(md, &vpckey, &vpcval);

	if (rc != 0) {
		TRN_LOG_ERROR("Failure getting vpc %ld data on interface: %s",
			      argp->tunid, argp->interface);
		goto error;
	}
	result.interface = argp->interface;
	result.routers_ips.routers_ips_val = vpcval.routers_ips;
	result.routers_ips.routers_ips_len = vpcval.nrouters;
	result.tunid = argp->tunid;

	return &result;

error:
	result.interface = "";
	return &result;
}

rpc_trn_network_t *get_net_1_svc(rpc_trn_network_key_t *argp,
				 struct svc_req *rqstp)
{
	UNUSED(rqstp);
	static rpc_trn_network_t result;
	result.switches_ips.switches_ips_len = 0;
	int rc;
	struct network_key_t netkey;
	struct network_t netval;
	char buffer[INET_ADDRSTRLEN];
	const char *parsed_netip =
		inet_ntop(AF_INET, &argp->netip, buffer, sizeof(buffer));

	TRN_LOG_DEBUG("get_net_1 net tunid: %ld, netip: %s, "
		      "prefixlen: %d, on interface %s",
		      argp->tunid, parsed_netip, argp->prefixlen,
		      argp->interface);

	struct user_metadata_t *md = trn_itf_table_find(argp->interface);
	if (!md) {
		TRN_LOG_ERROR("Cannot find interface metadata for %s",
			      argp->interface);
		goto error;
	}
	netkey.prefixlen = argp->prefixlen;
	memcpy(&netkey.nip, &argp->tunid, sizeof(argp->tunid));
	netkey.nip[2] = argp->netip;

	rc = trn_get_network(md, &netkey, &netval);
	if (rc != 0) {
		TRN_LOG_ERROR(
			"Failure getting net %ld, with ip %s on interface: %s",
			argp->tunid, parsed_netip, argp->interface);
		goto error;
	}
	result.interface = argp->interface;
	result.netip = argp->netip;
	result.tunid = argp->tunid;
	result.prefixlen = netval.prefixlen;
	result.switches_ips.switches_ips_val = netval.switches_ips;
	result.switches_ips.switches_ips_len = netval.nswitches;
	return &result;

error:
	result.interface = "";
	return &result;
}

rpc_trn_endpoint_t *get_ep_1_svc(rpc_trn_endpoint_key_t *argp,
				 struct svc_req *rqstp)
{
	UNUSED(rqstp);
	static rpc_trn_endpoint_t result;
	result.remote_ips.remote_ips_len = 0;
	result.hosted_interface = "";
	memset(result.mac, 0, sizeof(result.mac));
	int rc;
	struct endpoint_key_t epkey;
	struct endpoint_t epval;

	TRN_LOG_DEBUG("get_ep_1 ep tunid: %ld, ip: 0x%x,"
		      " on interface: %s",
		      argp->tunid, argp->ip, argp->interface);

	struct user_metadata_t *md = trn_itf_table_find(argp->interface);

	if (!md) {
		TRN_LOG_ERROR("Cannot find interface metadata for %s",
			      argp->interface);
		goto error;
	}

	memcpy(epkey.tunip, &argp->tunid, sizeof(argp->tunid));
	epkey.tunip[2] = argp->ip;

	rc = trn_get_endpoint(md, &epkey, &epval);

	if (rc != 0) {
		TRN_LOG_ERROR(
			"Cannot update transit XDP with ep %d on interface %s",
			epkey.tunip[2], argp->interface);
		goto error;
	}

	result.interface = argp->interface;
	char buf[IF_NAMESIZE];
	result.tunid = argp->tunid;
	result.ip = argp->ip;
	if (epval.hosted_iface != -1) {
		result.hosted_interface =
			if_indextoname(epval.hosted_iface, buf);
		if (result.hosted_interface == NULL) {
			TRN_LOG_ERROR(
				"The interface at index %d does not exist.",
				epval.hosted_iface);
			goto error;
		}
	} else {
		result.hosted_interface = "";
	}
	result.eptype = epval.eptype;
	memcpy(result.mac, epval.mac, sizeof(epval.mac));
	result.remote_ips.remote_ips_len = epval.nremote_ips;
	result.remote_ips.remote_ips_val = epval.remote_ips;
	result.veth = ""; // field to be removed
	return &result;

error:
	result.interface = "";
	return &result;
}

int *load_transit_xdp_1_svc(rpc_trn_xdp_intf_t *xdp_intf, struct svc_req *rqstp)
{
	UNUSED(rqstp);
	static int result = -1;

	int rc;
	bool unload_error = false;
	char *itf = xdp_intf->interface;
	int xdp_flag = xdp_intf->xdp_flag;
	char *kern_path = xdp_intf->xdp_path;
	struct user_metadata_t empty_md;
	struct user_metadata_t *md = trn_itf_table_find(itf);

	if (md) {
		TRN_LOG_INFO("meatadata for interface %s already exist.", itf);
	} else {
		TRN_LOG_INFO("creating meatadata for interface %s.", itf);
		md = malloc(sizeof(struct user_metadata_t));
	}

	if (!md) {
		TRN_LOG_ERROR("Failure allocating memory for user_metadata_t");
		result = RPC_TRN_FATAL;
		goto error;
	}

	memset(md, 0, sizeof(struct user_metadata_t));

	// Set all interface index slots to unused
	int i;
	for (i = 0; i < TRAN_MAX_ITF; i++) {
		md->itf_idx[i] = TRAN_UNUSED_ITF_IDX;
	}

	strcpy(md->pcapfile, xdp_intf->pcapfile);
	md->pcapfile[255] = '\0';
	md->xdp_flags = xdp_intf->xdp_flag;

	TRN_LOG_DEBUG("load_transit_xdp_1 path: %s, pcap: %s",
		      xdp_intf->xdp_path, xdp_intf->pcapfile);

	rc = trn_user_metadata_init(md, itf, kern_path, md->xdp_flags);

	if (rc != 0) {
		TRN_LOG_ERROR(
			"Failure initializing or loading transit XDP program for interface %s",
			itf);
		result = RPC_TRN_FATAL;
		goto error;
	}

	rc = trn_itf_table_insert(itf, md);
	if (rc != 0) {
		TRN_LOG_ERROR(
			"Failure populating interface table when loading XDP program on %s",
			itf);
		result = RPC_TRN_ERROR;
		unload_error = true;
		goto error;
	}

	TRN_LOG_INFO("Successfully loaded transit XDP on interface %s", itf);

	result = 0;
	return &result;

error:
	if (unload_error) {
		trn_user_metadata_free(md);
	}
	free(md);
	return &result;
}

int *unload_transit_xdp_1_svc(rpc_intf_t *argp, struct svc_req *rqstp)
{
	UNUSED(rqstp);
	static int result = -1;
	int rc;
	char *itf = argp->interface;

	TRN_LOG_DEBUG("unload_transit_xdp_1 interface: %s", itf);

	struct user_metadata_t *md = trn_itf_table_find(itf);

	if (!md) {
		TRN_LOG_ERROR("Cannot find interface metadata for %s", itf);
		result = RPC_TRN_ERROR;
		goto error;
	}

	rc = trn_user_metadata_free(md);

	if (rc != 0) {
		TRN_LOG_ERROR(
			"Cannot free XDP metadata, transit program may still be running");
		result = RPC_TRN_ERROR;
		goto error;
	}
	trn_itf_table_delete(itf);

	result = 0;
	return &result;

error:
	return &result;
}

int *load_transit_agent_xdp_1_svc(rpc_trn_xdp_intf_t *xdp_intf,
				  struct svc_req *rqstp)
{
	UNUSED(rqstp);
	static int result = -1;

	int rc;
	bool unload_error = false;
	/* The transit agent always runs on vif */
	char *itf = xdp_intf->interface;
	char *kern_path = xdp_intf->xdp_path;
	int xdp_flag = xdp_intf->xdp_flag;

	struct agent_user_metadata_t *md = trn_vif_table_find(itf);
	if (md) {
		TRN_LOG_INFO("meatadata for interface %s already exist.", itf);
	} else {
		md = malloc(sizeof(struct agent_user_metadata_t));
	}
	if (!md) {
		TRN_LOG_ERROR(
			"Failure allocating memory for agent_user_metadata_t");
		result = RPC_TRN_FATAL;
		goto error;
	}

	memset(md, 0, sizeof(struct agent_user_metadata_t));

	strcpy(md->pcapfile, xdp_intf->pcapfile);
	md->pcapfile[255] = '\0';
	md->xdp_flags = xdp_intf->xdp_flag;

	TRN_LOG_DEBUG("load_transit_agent_xdp_1 path: %s, pcap: %s",
		      xdp_intf->xdp_path, xdp_intf->pcapfile);

	rc = trn_agent_metadata_init(md, itf, kern_path, md->xdp_flags);

	if (rc != 0) {
		TRN_LOG_ERROR("Failure initializing or loading transit agent "
			      "XDP program for interface %s",
			      itf);
		result = RPC_TRN_FATAL;
		goto error;
	}

	rc = trn_vif_table_insert(itf, md);
	if (rc != 0) {
		TRN_LOG_ERROR("Failure populating interface table when "
			      "loading agent XDP program on %s",
			      itf);
		result = RPC_TRN_ERROR;
		unload_error = true;
		goto error;
	}

	result = 0;
	return &result;

error:
	if (unload_error) {
		trn_agent_user_metadata_free(md);
	}
	free(md);
	return &result;
}

int *unload_transit_agent_xdp_1_svc(rpc_intf_t *argp, struct svc_req *rqstp)
{
	UNUSED(rqstp);
	static int result = -1;
	int rc;
	char *itf = argp->interface;

	TRN_LOG_DEBUG("unload_transit_agent_xdp_1 interface: %s", itf);

	struct agent_user_metadata_t *md = trn_vif_table_find(itf);

	if (!md) {
		TRN_LOG_ERROR("Cannot find virtual interface metadata for %s",
			      itf);
		result = RPC_TRN_ERROR;
		goto error;
	}

	rc = trn_agent_user_metadata_free(md);

	if (rc != 0) {
		TRN_LOG_ERROR("Cannot free XDP metadata,"
			      " transit agent program may still be running");
		result = RPC_TRN_ERROR;
		goto error;
	}

	trn_vif_table_delete(itf);

	result = 0;
	return &result;

error:
	return &result;
}

int *update_agent_ep_1_svc(rpc_trn_endpoint_t *ep, struct svc_req *rqstp)
{
	UNUSED(rqstp);
	static int result = -1;
	result = 0;
	int rc;
	char *itf = ep->interface;
	struct endpoint_key_t epkey;
	struct endpoint_t epval;

	TRN_LOG_DEBUG("update_agent_ep_1 ep tunid: %ld, ip: 0x%x,"
		      " type: %d, veth: %s, hosted_interface:%s",
		      ep->tunid, ep->ip, ep->eptype, ep->veth,
		      ep->hosted_interface);

	struct agent_user_metadata_t *md = trn_vif_table_find(itf);

	if (!md) {
		TRN_LOG_ERROR("Cannot find virtual interface metadata for %s",
			      itf);
		result = RPC_TRN_ERROR;
		goto error;
	}

	memcpy(epkey.tunip, &ep->tunid, sizeof(ep->tunid));
	epkey.tunip[2] = ep->ip;
	epval.eptype = ep->eptype;

	epval.nremote_ips = ep->remote_ips.remote_ips_len;

	if (epval.nremote_ips > TRAN_MAX_REMOTES) {
		TRN_LOG_WARN("Number of maximum remote IPs exceeded,"
			     " configuring only the first %d remote IPs.",
			     TRAN_MAX_REMOTES);
		result = RPC_TRN_WARN;
		goto error;
	}

	if (epval.nremote_ips > 0) {
		memcpy(epval.remote_ips, ep->remote_ips.remote_ips_val,
		       epval.nremote_ips * sizeof(epval.remote_ips[0]));
	}

	memcpy(epval.mac, ep->mac, 6 * sizeof(epval.mac[0]));

	if (strcmp(ep->hosted_interface, "") != 0) {
		epval.hosted_iface = if_nametoindex(ep->hosted_interface);
	} else {
		epval.hosted_iface = -1;
	}

	/* in case if_nameindex fails!! */
	if (!epval.hosted_iface) {
		TRN_LOG_WARN(
			"Failed to map interface name to an index,"
			" for the given hosted_interface name traffic may"
			" not be routed correctly to ep %d hosted on this host.",
			ep->ip);
		result = RPC_TRN_WARN;
		epval.hosted_iface = -1;
	}

	rc = trn_agent_update_endpoint(md, &epkey, &epval);

	if (rc != 0) {
		TRN_LOG_ERROR("Cannot update agent with ep %d on interface %s",
			      epkey.tunip[2], itf);
		result = RPC_TRN_ERROR;
		goto error;
	}

	TRN_LOG_DEBUG("update_agent_ep_1 Success!");

	return &result;

error:
	return &result;
}

int *delete_agent_ep_1_svc(rpc_trn_endpoint_key_t *argp, struct svc_req *rqstp)
{
	UNUSED(rqstp);
	static int result = -1;
	result = 0;
	struct endpoint_key_t epkey;
	int rc;

	TRN_LOG_DEBUG("delete_agent_ep_1 ep tunid: %ld, ip: 0x%x,"
		      " on interface:%s",
		      argp->tunid, argp->ip, argp->interface);

	struct agent_user_metadata_t *md = trn_vif_table_find(argp->interface);

	if (!md) {
		TRN_LOG_ERROR("Cannot find interface metadata for %s",
			      argp->interface);
		result = RPC_TRN_ERROR;
		goto error;
	}

	memcpy(epkey.tunip, &argp->tunid, sizeof(argp->tunid));
	epkey.tunip[2] = argp->ip;

	rc = trn_agent_delete_endpoint(md, &epkey);

	if (rc != 0) {
		TRN_LOG_ERROR("Cannot delete agent ep %d on interface %s",
			      epkey.tunip[2], argp->interface);
		result = RPC_TRN_ERROR;
		goto error;
	}

	return &result;
error:
	return &result;
}

rpc_trn_endpoint_t *get_agent_ep_1_svc(rpc_trn_endpoint_key_t *argp,
				       struct svc_req *rqstp)
{
	UNUSED(rqstp);
	static rpc_trn_endpoint_t result;
	result.remote_ips.remote_ips_len = 0;
	result.hosted_interface = "";
	memset(result.mac, 0, sizeof(result.mac));
	struct endpoint_key_t epkey;
	struct endpoint_t epval;
	int rc;

	TRN_LOG_DEBUG("get_agent_ep_1 ep tunid: %ld, ip: 0x%x,"
		      " on interface:%s",
		      argp->tunid, argp->ip, argp->interface);

	struct agent_user_metadata_t *md = trn_vif_table_find(argp->interface);

	if (!md) {
		TRN_LOG_ERROR("Cannot find interface metadata for %s",
			      argp->interface);
		goto error;
	}

	memcpy(epkey.tunip, &argp->tunid, sizeof(argp->tunid));
	epkey.tunip[2] = argp->ip;

	rc = trn_agent_get_endpoint(md, &epkey, &epval);

	if (rc != 0) {
		TRN_LOG_ERROR("Cannot get agent ep %d on interface %s",
			      epkey.tunip[2], argp->interface);
		goto error;
	}

	result.interface = argp->interface;
	char buf[IF_NAMESIZE];
	result.tunid = argp->tunid;
	result.ip = argp->ip;
	if (epval.hosted_iface != -1) {
		result.hosted_interface =
			if_indextoname(epval.hosted_iface, buf);
		if (result.hosted_interface == NULL) {
			TRN_LOG_ERROR(
				"The interface at index %d does not exist.",
				epval.hosted_iface);
			goto error;
		}
	} else {
		result.hosted_interface = "";
	}
	TRN_LOG_DEBUG("get_agent_ep1 success!");
	result.eptype = epval.eptype;
	memcpy(result.mac, epval.mac, sizeof(epval.mac));
	result.remote_ips.remote_ips_len = epval.nremote_ips;
	result.remote_ips.remote_ips_val = epval.remote_ips;
	result.veth = ""; //field to be removed
	return &result;

error:
	result.interface = "";
	return &result;
}

int *update_packet_metadata_1_svc(rpc_trn_packet_metadata_t *packet_metadata, struct svc_req *rqstp)
{
	UNUSED(rqstp);
	static int result = -1;
	result = 0;
	int rc;
	char *itf = packet_metadata->interface;
	struct packet_metadata_key_t key;
	struct packet_metadata_t value;

	TRN_LOG_DEBUG("update_packet_metadata_1 packet metadata tunid: %ld, ip: 0x%x,"
		      " pod_label_value: %d, namespace_label_value: %d",
		      packet_metadata->tunid, packet_metadata->ip,
		      packet_metadata->pod_label_value,
			  packet_metadata->namespace_label_value);

	struct agent_user_metadata_t *md = trn_vif_table_find(itf);

	if (!md) {
		TRN_LOG_ERROR("Cannot find virtual interface metadata for %s",
			      itf);
		result = RPC_TRN_ERROR;
		goto error;
	}

	memcpy(key.tunip, &packet_metadata->tunid, sizeof(packet_metadata->tunid));	
	key.tunip[2] = packet_metadata->ip;
	value.pod_label_value = packet_metadata->pod_label_value;
	value.namespace_label_value = packet_metadata->namespace_label_value;	

	rc = trn_agent_update_packet_metadata(md, &key, &value);

	if (rc != 0) {
		TRN_LOG_ERROR("Cannot update agent with packet metadata %d on interface %s",
			      key.tunip[2], itf);
		result = RPC_TRN_ERROR;
		goto error;
	}

	TRN_LOG_DEBUG("update_packet_metadata_1 Success!");

	return &result;

error:
	return &result;
}

int *delete_packet_metadata_1_svc(rpc_trn_packet_metadata_key_t *argp, struct svc_req *rqstp)
{
	UNUSED(rqstp);
	static int result = -1;
	result = 0;
	struct packet_metadata_key_t key;
	int rc;

	TRN_LOG_DEBUG("delete_packet_metadata_1 ep tunid: %ld, ip: 0x%x,"
		      " on interface:%s",
		      argp->tunid, argp->ip, argp->interface);

	struct agent_user_metadata_t *md = trn_vif_table_find(argp->interface);

	if (!md) {
		TRN_LOG_ERROR("Cannot find interface metadata for %s",
			      argp->interface);
		result = RPC_TRN_ERROR;
		goto error;
	}

	memcpy(key.tunip, &argp->tunid, sizeof(argp->tunid));
	key.tunip[2] = argp->ip;

	rc = trn_agent_delete_packet_metadata(md, &key);

	if (rc != 0) {
		TRN_LOG_ERROR("Cannot delete agent packet metadata %d on interface %s",
			      key.tunip[2], argp->interface);
		result = RPC_TRN_ERROR;
		goto error;
	}

	return &result;
error:
	return &result;
}

int *update_agent_md_1_svc(rpc_trn_agent_metadata_t *agent_md,
			   struct svc_req *rqstp)
{
	UNUSED(rqstp);
	static int result = -1;
	int rc;
	char *itf = agent_md->interface;

	TRN_LOG_DEBUG("update_agent_md_1 interface: [%s]", itf);
	struct agent_user_metadata_t *md = trn_vif_table_find(itf);

	if (!md) {
		TRN_LOG_ERROR("Cannot find interface metadata for [%s]", itf);
		result = RPC_TRN_ERROR;
		goto error;
	}

	/* retrieve substrate interface metadata */
	struct user_metadata_t *eth_md =
		trn_itf_table_find(agent_md->eth.interface);

	if (!eth_md) {
		TRN_LOG_ERROR(
			"Cannot find substrate interface metadata for [%s]. Required for agent XDP program.",
			agent_md->eth.interface);
		result = RPC_TRN_ERROR;
		goto error;
	}

	struct agent_metadata_t amd;

	amd.eth.iface_index = if_nametoindex(agent_md->eth.interface);
	amd.eth.ip = agent_md->eth.ip;
	memcpy(amd.eth.mac, agent_md->eth.mac, 6 * sizeof(amd.eth.mac[0]));

	amd.nkey.prefixlen = agent_md->net.prefixlen;
	memcpy(&amd.nkey.nip[0], &agent_md->net.tunid, sizeof(uint64_t));
	amd.nkey.nip[2] = agent_md->net.netip;

	amd.net.nswitches = agent_md->net.switches_ips.switches_ips_len;
	memcpy(&amd.net.switches_ips,
	       agent_md->net.switches_ips.switches_ips_val,
	       sizeof(uint32_t) * amd.net.nswitches);

	amd.epkey.tunip[0] = amd.nkey.nip[0];
	amd.epkey.tunip[1] = amd.nkey.nip[1];
	amd.epkey.tunip[2] = agent_md->ep.ip;

	amd.ep.eptype = agent_md->ep.eptype;
	amd.ep.nremote_ips = agent_md->ep.remote_ips.remote_ips_len;
	if (amd.ep.nremote_ips > 0) {
		memcpy(amd.ep.remote_ips,
		       agent_md->ep.remote_ips.remote_ips_val,
		       sizeof(uint32_t) * amd.ep.nremote_ips);
	}

	amd.ep.hosted_iface = amd.eth.iface_index;
	memcpy(amd.ep.mac, agent_md->ep.mac, 6 * sizeof(amd.ep.mac[0]));

	rc = trn_agent_update_agent_metadata(md, &amd, eth_md);

	if (rc != 0) {
		TRN_LOG_ERROR("Cannot update agent metadata on interface %s",
			      itf);
		result = RPC_TRN_ERROR;
		goto error;
	}

	rc = trn_agent_add_prog(md, XDP_TRANSIT, eth_md->prog_fd);

	if (rc != 0) {
		TRN_LOG_ERROR("Cannot update agent jump table %s", itf);
		result = RPC_TRN_ERROR;
		goto error;
	}

	result = 0;
	return &result;

error:
	return &result;
}

int *delete_agent_md_1_svc(rpc_intf_t *argp, struct svc_req *rqstp)
{
	UNUSED(rqstp);

	static int result = -1;
	result = 0;
	int rc;

	TRN_LOG_DEBUG("delete_agent_md_1 interface: [%s]", argp->interface);
	struct agent_user_metadata_t *md = trn_vif_table_find(argp->interface);
	if (!md) {
		TRN_LOG_ERROR("Cannot find interface metadata for [%s]",
			      argp->interface);
		result = RPC_TRN_ERROR;
		goto error;
	}

	rc = trn_agent_delete_agent_metadata(md);
	if (rc != 0) {
		TRN_LOG_ERROR("Cannot delete agent metadata on interface %s",
			      argp->interface);
		result = RPC_TRN_ERROR;
		goto error;
	}

	return &result;
error:
	return &result;
}

rpc_trn_agent_metadata_t *get_agent_md_1_svc(rpc_intf_t *argp,
					     struct svc_req *rqstp)
{
	UNUSED(rqstp);
	static rpc_trn_agent_metadata_t result;
	int rc;
	char *itf = argp->interface;
	struct agent_metadata_t amd;

	TRN_LOG_DEBUG("get_agent_md_1 interface: [%s]", itf);
	struct agent_user_metadata_t *md = trn_vif_table_find(itf);
	if (!md) {
		TRN_LOG_ERROR("Cannot find interface metadata for [%s]", itf);
		goto error;
	}

	rc = trn_agent_get_agent_metadata(md, &amd);
	if (rc != 0) {
		TRN_LOG_ERROR("Cannot get agent metadata on interface %s", itf);
		goto error;
	}

	result.interface = itf;
	char buf[IF_NAMESIZE];
	result.eth.ip = amd.eth.ip;
	memcpy(result.eth.mac, amd.eth.mac, sizeof(amd.eth.mac));
	result.eth.interface = if_indextoname(amd.eth.iface_index, buf);
	if (result.eth.interface == NULL) {
		TRN_LOG_ERROR("The interface at index %d does not exist.",
			      amd.eth.iface_index);
		goto error;
	}

	result.net.interface = "";
	result.net.prefixlen = amd.net.prefixlen;
	memcpy(&result.net.tunid, &amd.nkey.nip[0], sizeof(uint64_t));
	result.net.netip = amd.nkey.nip[2];
	result.net.switches_ips.switches_ips_len = amd.net.nswitches;
	result.net.switches_ips.switches_ips_val = amd.net.switches_ips;

	result.ep.interface = "";
	memcpy(&result.ep.tunid, &amd.epkey.tunip[0], sizeof(uint64_t));
	result.ep.ip = amd.epkey.tunip[2];
	result.ep.eptype = amd.ep.eptype;
	memcpy(result.ep.mac, amd.ep.mac, sizeof(amd.ep.mac));
	result.ep.veth = "";
	result.ep.remote_ips.remote_ips_len = amd.ep.nremote_ips;
	result.ep.remote_ips.remote_ips_val = amd.ep.remote_ips;

	result.ep.hosted_interface = if_indextoname(amd.ep.hosted_iface, buf);
	if (result.ep.hosted_interface == NULL) {
		TRN_LOG_ERROR("The interface at index %d does not exist.",
			      amd.ep.hosted_iface);
		goto error;
	}

	return &result;

error:
	result.interface = "";
	return &result;
}

int *load_transit_xdp_pipeline_stage_1_svc(rpc_trn_ebpf_prog_t *argp,
					   struct svc_req *rqstp)
{
	UNUSED(rqstp);

	static int result = -1;
	int rc;
	struct user_metadata_t *md;
	char *prog_path = argp->xdp_path;
	unsigned int prog_idx = argp->stage;

	switch (prog_idx) {
	case ON_XDP_TX:
	case ON_XDP_PASS:
	case ON_XDP_REDIRECT:
	case ON_XDP_DROP:
	case ON_XDP_SCALED_EP:
		break;
	default:
		TRN_LOG_ERROR("Unsupported program stage %s", argp->interface);
		result = RPC_TRN_ERROR;
		goto error;
	}

	md = trn_itf_table_find(argp->interface);

	if (!md) {
		TRN_LOG_ERROR("Cannot find interface metadata for %s",
			      argp->interface);
		goto error;
	}

	rc = trn_add_prog(md, prog_idx, prog_path);

	if (rc != 0) {
		TRN_LOG_ERROR("Failed to insert XDP stage %d for interface %s",
			      prog_idx, argp->interface);
		result = RPC_TRN_ERROR;
		goto error;
	}

	result = 0;
	return &result;

error:
	return &result;
}

int *unload_transit_xdp_pipeline_stage_1_svc(rpc_trn_ebpf_prog_stage_t *argp,
					     struct svc_req *rqstp)
{
	UNUSED(rqstp);
	static int result = -1;
	int rc;
	struct user_metadata_t *md;
	unsigned int prog_idx = argp->stage;

	switch (prog_idx) {
	case ON_XDP_TX:
	case ON_XDP_PASS:
	case ON_XDP_REDIRECT:
	case ON_XDP_DROP:
	case ON_XDP_SCALED_EP:
		break;
	default:
		TRN_LOG_ERROR("Unsupported program stage %s", argp->interface);
		result = RPC_TRN_ERROR;
		goto error;
	}

	md = trn_itf_table_find(argp->interface);

	if (!md) {
		TRN_LOG_ERROR("Cannot find interface metadata for %s",
			      argp->interface);
		goto error;
	}

	rc = trn_remove_prog(md, prog_idx);

	if (rc != 0) {
		TRN_LOG_ERROR("Failed to remove XDP stage %d for interface %s",
			      prog_idx, argp->interface);
		result = RPC_TRN_ERROR;
		goto error;
	}

	result = 0;
	return &result;

error:
	return &result;
}

int *update_transit_network_policy_1_svc(rpc_trn_vsip_cidr_t *policy, struct svc_req *rqstp)
{
	UNUSED(rqstp);
	static int result = -1;
	int map_fd = -1;
	int rc;
	char *itf = policy->interface;
	int type = policy->cidr_type;
	struct vsip_cidr_t cidr;

	TRN_LOG_INFO("update_transit_network_policy_1_svc service");

	struct user_metadata_t *md = trn_itf_table_find(itf);
	if (!md) {
		TRN_LOG_ERROR("Cannot find interface metadata for %s", itf);
		result = RPC_TRN_ERROR;
		goto error;
	}

	cidr.tunnel_id = policy->tunid;
	// cidr-related maps have tunnel-id(64 bits),
	// local-ip(32 bits) prior to destination cidr;
	// hence the final prefix length is 64+32+{cidr prefix}
	cidr.prefixlen = policy->cidr_prefixlen + 96;
	cidr.local_ip = policy->local_ip;
	cidr.remote_ip = policy->cidr_ip;
	TRN_LOG_INFO("policy: tunnel_id  %ld; local ip  0x%x; cidr ip  0x%x\n", 
			policy->tunid, policy->local_ip, policy->cidr_ip);

	if (type == PRIMARY) {
		map_fd = md->ing_vsip_prim_map_fd;
	}else if (type == SUPPLEMENTARY){
		map_fd = md->ing_vsip_supp_map_fd;
	}else if (type == EXCEPTION){
		map_fd = md->ing_vsip_except_map_fd;
	}else {
		TRN_LOG_ERROR("Wrong network policy CIDR type. \n");
		result = RPC_TRN_ERROR;
		goto error;
	}

	rc = trn_update_transit_network_policy_map(map_fd, &cidr, policy->bit_val);

	if (rc != 0) {
		TRN_LOG_ERROR("Failure updating transit network policy map cidr.");
		result = RPC_TRN_FATAL;
		goto error;
	}

	result = 0;
	return &result;

error:
	return &result;
}

int *update_agent_network_policy_1_svc(rpc_trn_vsip_cidr_t *policy, struct svc_req *rqstp)
{
	UNUSED(rqstp);
	static int result = -1;
	int rc;
	int map_fd = -1;
	char *itf = policy->interface;
	int type = policy->cidr_type;
	struct vsip_cidr_t cidr;

	TRN_LOG_INFO("update_agent_network_policy_1_svc service");

	struct agent_user_metadata_t *md = trn_vif_table_find(itf);
	if (!md) {
		TRN_LOG_ERROR("Cannot find interface metadata for %s", itf);
		result = RPC_TRN_ERROR;
		goto error;
	}

	cidr.tunnel_id = policy->tunid;
	// cidr-related maps have tunnel-id(64 bits),
	// local-ip(32 bits) prior to destination cidr;
	// hence the final prefix length is 64+32+{cidr prefix}
	cidr.prefixlen = policy->cidr_prefixlen + 96;
	cidr.local_ip = policy->local_ip;
	cidr.remote_ip = policy->cidr_ip;
	TRN_LOG_INFO("policy: tunnel_id  %ld; local ip  0x%x; cidr ip  0x%x \n", 
			policy->tunid, policy->local_ip, policy->cidr_ip);

	if (type == PRIMARY) {
		map_fd = md->eg_vsip_prim_map_fd;
	}else if (type == SUPPLEMENTARY){
		map_fd = md->eg_vsip_supp_map_fd;
	}else if (type == EXCEPTION){
		map_fd = md->eg_vsip_except_map_fd;
	}else {
		TRN_LOG_ERROR("Wrong network policy CIDR type. \n");
		result = RPC_TRN_ERROR;
		goto error;
	}

	rc = trn_update_agent_network_policy_map(map_fd, &cidr, policy->bit_val);
	if (rc != 0) {
		TRN_LOG_ERROR("Failure updating agent network policy map cidr.");
		result = RPC_TRN_FATAL;
		goto error;
	}

	result = 0;
	return &result;

error:
	return &result;
}

int *delete_transit_network_policy_1_svc(rpc_trn_vsip_cidr_key_t *policy_key, struct svc_req *rqstp)
{
	UNUSED(rqstp);
	static int result = -1;
	int rc;
	int map_fd = -1;
	char *itf = policy_key->interface;
	int type = policy_key->cidr_type;
	struct vsip_cidr_t cidr;

	TRN_LOG_INFO("delete_transit_network_policy_1_svc service");

	struct user_metadata_t *md = trn_itf_table_find(itf);
	if (!md) {
		TRN_LOG_ERROR("Cannot find interface metadata for %s", itf);
		result = RPC_TRN_ERROR;
		goto error;
	}

	cidr.tunnel_id = policy_key->tunid;
	// cidr-related maps have tunnel-id(64 bits),
	// local-ip(32 bits) prior to destination cidr;
	// hence the final prefix length is 64+32+{cidr prefix}
	cidr.prefixlen = policy_key->cidr_prefixlen + 96;
	cidr.local_ip = policy_key->local_ip;
	cidr.remote_ip = policy_key->cidr_ip;
	TRN_LOG_INFO("policy: tunnel_id  %ld; local ip  0x%x; cidr ip  0x%x\n", 
			policy_key->tunid, policy_key->local_ip, policy_key->cidr_ip);

	if (type == PRIMARY) {
		map_fd = md->ing_vsip_prim_map_fd;
	}else if (type == SUPPLEMENTARY){
		map_fd = md->ing_vsip_supp_map_fd;
	}else if (type == EXCEPTION){
		map_fd = md->ing_vsip_except_map_fd;
	}else {
		TRN_LOG_ERROR("Wrong network policy CIDR type. \n");
		result = RPC_TRN_ERROR;
		goto error;
	}

	rc = trn_delete_transit_network_policy_map(map_fd, &cidr);

	if (rc != 0) {
		TRN_LOG_ERROR("Failure deleting transit network policy map cidr");
		result = RPC_TRN_FATAL;
		goto error;
	}

	result = 0;
	return &result;

error:
	return &result;
}

int *delete_agent_network_policy_1_svc(rpc_trn_vsip_cidr_key_t *policy_key, struct svc_req *rqstp)
{
	UNUSED(rqstp);
	static int result = -1;
	int rc = -1;
	int map_fd = -1;
	char *itf = policy_key->interface;
	int type = policy_key->cidr_type;
	TRN_LOG_INFO("delete_agent_network_policy_1_svc service");

	struct vsip_cidr_t cidr;
	struct agent_user_metadata_t *md = trn_vif_table_find(itf);
	if (!md) {
		TRN_LOG_ERROR("Cannot find interface metadata for %s", itf);
		result = RPC_TRN_ERROR;
		goto error;
	}

	cidr.tunnel_id = policy_key->tunid;
	// cidr-related maps have tunnel-id(64 bits),
	// local-ip(32 bits) prior to destination cidr;
	// hence the final prefix length is 64+32+{cidr prefix}
	cidr.prefixlen = policy_key->cidr_prefixlen + 96;
	cidr.local_ip = policy_key->local_ip;
	cidr.remote_ip = policy_key->cidr_ip;
	TRN_LOG_INFO("policy: tunnel_id  %ld; local ip  0x%x; cidr ip  0x%x\n", 
			policy_key->tunid, policy_key->local_ip, policy_key->cidr_ip);

	if (type == PRIMARY) {
		map_fd = md->eg_vsip_prim_map_fd;
	}else if (type == SUPPLEMENTARY){
		map_fd = md->eg_vsip_supp_map_fd;
	}else if (type == EXCEPTION){
		map_fd = md->eg_vsip_except_map_fd;
	}else {
		TRN_LOG_ERROR("Wrong network policy CIDR type. \n");
		result = RPC_TRN_ERROR;
		goto error;
	}

	rc = trn_delete_agent_network_policy_map(map_fd, &cidr);

	if (rc != 0) {
		TRN_LOG_ERROR("Failure deleting agent network policy map cidr");
		result = RPC_TRN_FATAL;
		goto error;
	}

	result = 0;
	return &result;

error:
	return &result;
}

int *update_transit_network_policy_enforcement_1_svc(rpc_trn_vsip_enforce_t *policy, struct svc_req *rqstp)
{
	UNUSED(rqstp);
	static int result = -1;
	int rc;
	char *itf = policy->interface;
	struct vsip_enforce_t enforce;
	__u8 val;

	TRN_LOG_INFO("update_transit_network_policy_enforcement_1_svc service");

	struct user_metadata_t *md = trn_itf_table_find(itf);
	if (!md) {
		TRN_LOG_ERROR("Cannot find interface metadata for %s", itf);
		result = RPC_TRN_ERROR;
		goto error;
	}

	enforce.tunnel_id = policy->tunid;
	enforce.local_ip = policy->local_ip;
	val = 1;

	rc = trn_update_transit_network_policy_enforcement_map(md, &enforce, val);

	if (rc != 0) {
		TRN_LOG_ERROR("Failure updating transit network policy enforcement map ip address: 0x%x, for interface %s",
					policy->local_ip, policy->interface);
		result = RPC_TRN_FATAL;
		goto error;
	}

	result = 0;
	return &result;

error:
	return &result;
}

int *update_agent_network_policy_enforcement_1_svc(rpc_trn_vsip_enforce_t *policy, struct svc_req *rqstp)
{
	UNUSED(rqstp);
	static int result = -1;
	int rc;
	char *itf = policy->interface;
	struct vsip_enforce_t enforce;
	__u8 val;

	TRN_LOG_INFO("update_agent_network_policy_enforcement_1_svc service");

	struct agent_user_metadata_t *md = trn_vif_table_find(itf);
	if (!md) {
		TRN_LOG_ERROR("Cannot find interface metadata for %s", itf);
		result = RPC_TRN_ERROR;
		goto error;
	}

	enforce.tunnel_id = policy->tunid;
	enforce.local_ip = policy->local_ip;
	val = 1;

	rc = trn_update_agent_network_policy_enforcement_map(md, &enforce, val);

	if (rc != 0) {
		TRN_LOG_ERROR("Failure updating agnet network policy enforcement map ip address: 0x%x, for interface %s",
					policy->local_ip, policy->interface);
		result = RPC_TRN_FATAL;
		goto error;
	}

	result = 0;
	return &result;

error:
	return &result;
}

int *delete_transit_network_policy_enforcement_1_svc(rpc_trn_vsip_enforce_t *policy, struct svc_req *rqstp)
{
	UNUSED(rqstp);
	static int result;
	int rc;
	char *itf = policy->interface;
	struct vsip_enforce_t enf;

	TRN_LOG_INFO("delete_transit_network_policy_enforcement_1_svc service");

	struct user_metadata_t *md = trn_itf_table_find(itf);
	if (!md) {
		TRN_LOG_ERROR("Cannot find interface metadata for %s", itf);
		result = RPC_TRN_ERROR;
		goto error;
	}

	enf.tunnel_id = policy->tunid;
	enf.local_ip = policy->local_ip;

	rc = trn_delete_transit_network_policy_enforcement_map(md, &enf);

	if (rc != 0) {
		TRN_LOG_ERROR("Failure deleting transit network policy enforcement map ip address: 0x%x, for interface %s",
					policy->local_ip, policy->interface);
		result = RPC_TRN_FATAL;
		goto error;
	}

	result = 0;
	return &result;

error:
	return &result;
}

int *delete_agent_network_policy_enforcement_1_svc(rpc_trn_vsip_enforce_t *policy, struct svc_req *rqstp)
{
	UNUSED(rqstp);
	static int result;
	int rc;
	char *itf = policy->interface;
	struct vsip_enforce_t enf;

	TRN_LOG_INFO("delete_agent_network_policy_enforcement_1_svc service");

	struct agent_user_metadata_t *md = trn_vif_table_find(itf);
	if (!md) {
		TRN_LOG_ERROR("Cannot find interface metadata for %s", itf);
		result = RPC_TRN_ERROR;
		goto error;
	}

	enf.tunnel_id = policy->tunid;
	enf.local_ip = policy->local_ip;

	rc = trn_delete_agent_network_policy_enforcement_map(md, &enf);

	if (rc != 0) {
		TRN_LOG_ERROR("Failure deleting agent network policy enforcement map ip address: 0x%x, for interface %s",
					policy->local_ip, policy->interface);
		result = RPC_TRN_FATAL;
		goto error;
	}

	result = 0;
	return &result;

error:
	return &result;
}

int *update_transit_network_policy_protocol_port_1_svc(rpc_trn_vsip_ppo_t *ppo, struct svc_req *rqstp)
{
	UNUSED(rqstp);
	static int result = -1;
	int rc;
	char *itf = ppo->interface;
	
	TRN_LOG_INFO("update_transit_network_policy_protocol_port_1_svc service");
	struct vsip_ppo_t policy;
	struct user_metadata_t *md = trn_itf_table_find(itf);
	if (!md) {
		TRN_LOG_ERROR("Cannot find interface metadata for %s", itf);
		result = RPC_TRN_ERROR;
		goto error;
	}

	policy.tunnel_id = ppo->tunid;
	policy.local_ip = ppo->local_ip;
	policy.proto = ppo->proto;
	policy.port = ppo->port;
	TRN_LOG_INFO("ppo: tunnel_id  %ld; local ip  0x%x; protocol  %d; port  %d\n", 
				ppo->tunid, ppo->local_ip, ppo->proto, ppo->port);

	rc = trn_update_transit_network_policy_protocol_port_map(md, &policy, ppo->bit_val);

	if (rc != 0) {
		TRN_LOG_ERROR("Failure updating transit network policy protocol port map\n");
		result = RPC_TRN_FATAL;
		goto error;
	}

	result = 0;
	return &result;

error:
	return &result;
}

int *update_agent_network_policy_protocol_port_1_svc(rpc_trn_vsip_ppo_t *ppo, struct svc_req *rqstp)
{
	UNUSED(rqstp);
	static int result = -1;
	int rc;
	char *itf = ppo->interface;

	TRN_LOG_INFO("update_agent_network_policy_protocol_port_1_svc service");
	struct vsip_ppo_t policy;
	struct agent_user_metadata_t *md = trn_vif_table_find(itf);
	if (!md) {
		TRN_LOG_ERROR("Cannot find interface metadata for %s", itf);
		result = RPC_TRN_ERROR;
		goto error;
	}

	policy.tunnel_id = ppo->tunid;
	policy.local_ip = ppo->local_ip;
	policy.proto = ppo->proto;
	policy.port = ppo->port;
	TRN_LOG_INFO("ppo: tunnel_id  %ld; local ip  0x%x; protocol  %d; port  %d\n", 
				ppo->tunid, ppo->local_ip, ppo->proto, ppo->port);

	rc = trn_update_agent_network_policy_protocol_port_map(md, &policy, ppo->bit_val);

	if (rc != 0) {
		TRN_LOG_ERROR("Failure updating agent network policy protocol port map\n");
		result = RPC_TRN_FATAL;
		goto error;
	}

	result = 0;
	return &result;

error:
	return &result;
}

int *delete_transit_network_policy_protocol_port_1_svc(rpc_trn_vsip_ppo_key_t *ppo_key, struct svc_req *rqstp)
{
	UNUSED(rqstp);
	static int result = -1;
	int rc;
	char *itf = ppo_key->interface;
	
	TRN_LOG_INFO("delete_transit_network_policy_protocol_port_1 service");
	struct vsip_ppo_t policy;
	struct user_metadata_t *md = trn_itf_table_find(itf);
	if (!md) {
		TRN_LOG_ERROR("Cannot find interface metadata for %s", itf);
		result = RPC_TRN_ERROR;
		goto error;
	}

	policy.tunnel_id = ppo_key->tunid;
	policy.local_ip = ppo_key->local_ip;
	policy.proto = ppo_key->proto;
	policy.port = ppo_key->port;
	TRN_LOG_INFO("ppo: tunnel_id  %ld; local ip  0x%x; protocol  %d; port  %d\n", 
				ppo_key->tunid, ppo_key->local_ip, ppo_key->proto, ppo_key->port);

	rc = trn_delete_transit_network_policy_protocol_port_map(md, &policy);

	if (rc != 0) {
		TRN_LOG_ERROR("Failure deleting transit network policy protocol port map\n");
		result = RPC_TRN_FATAL;
		goto error;
	}

	result = 0;
	return &result;

error:
	return &result;
}

int *delete_agent_network_policy_protocol_port_1_svc(rpc_trn_vsip_ppo_key_t *ppo_key, struct svc_req *rqstp)
{
	UNUSED(rqstp);
	static int result = -1;
	int rc;
	char *itf = ppo_key->interface;

	TRN_LOG_INFO("delete_agent_network_policy_protocol_port_1 service");
	struct vsip_ppo_t policy;
	struct agent_user_metadata_t *md = trn_vif_table_find(itf);
	if (!md) {
		TRN_LOG_ERROR("Cannot find interface metadata for %s", itf);
		result = RPC_TRN_ERROR;
		goto error;
	}

	policy.tunnel_id = ppo_key->tunid;
	policy.local_ip = ppo_key->local_ip;
	policy.proto = ppo_key->proto;
	policy.port = ppo_key->port;

	TRN_LOG_INFO("ppo: tunnel_id  %ld; local ip  0x%x; protocol  %d; port  %d \n", 
				ppo_key->tunid, ppo_key->local_ip, ppo_key->proto, ppo_key->port);

	rc = trn_delete_agent_network_policy_protocol_port_map(md, &policy);

	if (rc != 0) {
		TRN_LOG_ERROR("Failure updating agent network policy protocol port map \n");
		result = RPC_TRN_FATAL;
		goto error;
	}

	result = 0;
	return &result;

error:
	return &result;
}
