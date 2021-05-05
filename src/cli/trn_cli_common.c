// SPDX-License-Identifier: GPL-2.0-or-later
/**
 * @file trn_cli_common.c
 * @author Sherif Abdelwahab (@zasherif)
 *         Phu Tran          (@phudtran)
 *
 * @brief Defines common functions for parsing CLI input
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
#include "trn_cli.h"

cJSON *trn_cli_parse_json(const char *buf)
{
	cJSON *conf_json = cJSON_Parse(buf);
	if (conf_json == NULL) {
		const char *error_ptr = cJSON_GetErrorPtr();
		if (error_ptr != NULL) {
			print_err("Error: parsing json string before: %s.\n",
				  error_ptr);
		}
		return NULL;
	}
	return conf_json;
}

int trn_cli_parse_vpc(const cJSON *jsonobj, rpc_trn_vpc_t *vpc)
{
	cJSON *tunnel_id = cJSON_GetObjectItem(jsonobj, "tunnel_id");
	cJSON *routers_ips = cJSON_GetObjectItem(jsonobj, "routers_ips");
	cJSON *router_ip = NULL;
	int retval = 0;

	if (tunnel_id == NULL) {
		vpc->tunid = 0;
	} else if (cJSON_IsString(tunnel_id)) {
		vpc->tunid = atoi(tunnel_id->valuestring);
	} else {
		print_err("Error: vpc tunnel_id is non-string.\n");
		return -EINVAL;
	}

	int i = 0;
	vpc->routers_ips.routers_ips_len = 0;
	cJSON_ArrayForEach(router_ip, routers_ips)
	{
		if (cJSON_IsString(router_ip)) {
			struct sockaddr_in sa;
			inet_pton(AF_INET, router_ip->valuestring,
				  &(sa.sin_addr));
			vpc->routers_ips.routers_ips_val[i] =
				sa.sin_addr.s_addr;
			vpc->routers_ips.routers_ips_len++;
		} else {
			print_err("Error: vpc router ip is non-string.\n");
			return -EINVAL;
		}
		i++;
		if (i == RPC_TRN_MAX_REMOTE_IPS) {
			print_err(
				"Warning: router ip addresses reached max limited, continuing with sublist.\n");
			retval = -EMSGSIZE;
			break;
		}
	}
	return retval;
}

int trn_cli_parse_vpc_key(const cJSON *jsonobj, rpc_trn_vpc_key_t *vpc)
{
	cJSON *tunnel_id = cJSON_GetObjectItem(jsonobj, "tunnel_id");
	cJSON *routers_ips = cJSON_GetObjectItem(jsonobj, "routers_ips");
	cJSON *router_ip = NULL;
	int retval = 0;

	if (tunnel_id == NULL) {
		vpc->tunid = 0;
	} else if (cJSON_IsString(tunnel_id)) {
		vpc->tunid = atoi(tunnel_id->valuestring);
	} else {
		print_err("Error: vpc tunnel_id is non-string.\n");
		return -EINVAL;
	}
	return retval;
}

int trn_cli_parse_net(const cJSON *jsonobj, struct rpc_trn_network_t *net)
{
	cJSON *tunnel_id = cJSON_GetObjectItem(jsonobj, "tunnel_id");
	cJSON *nip = cJSON_GetObjectItem(jsonobj, "nip");
	cJSON *prefixlen = cJSON_GetObjectItem(jsonobj, "prefixlen");
	cJSON *switches_ips = cJSON_GetObjectItem(jsonobj, "switches_ips");
	cJSON *switch_ip = NULL;

	if (cJSON_IsString(tunnel_id)) {
		uint64_t tid = atoi(tunnel_id->valuestring);
		net->tunid = tid;
	} else {
		print_err("Error: Net Tunnel ID Error\n");
		return -EINVAL;
	}

	if (nip != NULL && cJSON_IsString(nip)) {
		struct sockaddr_in sa;
		inet_pton(AF_INET, nip->valuestring, &(sa.sin_addr));
		net->netip = sa.sin_addr.s_addr;
	} else {
		print_err("Error: Net IP is missing or non-string\n");
		return -EINVAL;
	}

	if (cJSON_IsString(prefixlen)) {
		net->prefixlen = atoi(prefixlen->valuestring);
	} else {
		print_err("Error: Net prefixlen Error\n");
		return -EINVAL;
	}

	int i = 0;
	net->switches_ips.switches_ips_len = 0;
	cJSON_ArrayForEach(switch_ip, switches_ips)
	{
		if (cJSON_IsString(switch_ip)) {
			struct sockaddr_in sa;
			inet_pton(AF_INET, switch_ip->valuestring,
				  &(sa.sin_addr));
			net->switches_ips.switches_ips_val[i] =
				sa.sin_addr.s_addr;
			i++;
		} else {
			print_err("Error: Net Switch IP is non-string\n");
			return -EINVAL;
		}
		if (i == RPC_TRN_MAX_NET_SWITCHES) {
			print_err("(Warning) Switch IPS reached max limited\n");
			break;
		}
	}
	net->switches_ips.switches_ips_len = i;
	return 0;
}

int trn_cli_parse_net_key(const cJSON *jsonobj,
			  struct rpc_trn_network_key_t *net)
{
	cJSON *tunnel_id = cJSON_GetObjectItem(jsonobj, "tunnel_id");
	cJSON *nip = cJSON_GetObjectItem(jsonobj, "nip");
	cJSON *prefixlen = cJSON_GetObjectItem(jsonobj, "prefixlen");
	cJSON *switches_ips = cJSON_GetObjectItem(jsonobj, "switches_ips");
	cJSON *switch_ip = NULL;

	if (cJSON_IsString(tunnel_id)) {
		uint64_t tid = atoi(tunnel_id->valuestring);
		net->tunid = tid;
	} else {
		print_err("Error: Net Tunnel ID Error\n");
		return -EINVAL;
	}

	if (nip != NULL && cJSON_IsString(nip)) {
		struct sockaddr_in sa;
		inet_pton(AF_INET, nip->valuestring, &(sa.sin_addr));
		net->netip = sa.sin_addr.s_addr;
	} else {
		print_err("Error: Net IP is missing or non-string\n");
		return -EINVAL;
	}

	if (cJSON_IsString(prefixlen)) {
		net->prefixlen = atoi(prefixlen->valuestring);
	} else {
		print_err("Error: Net prefixlen Error\n");
		return -EINVAL;
	}
	return 0;
}

int trn_cli_parse_xdp(const cJSON *jsonobj, rpc_trn_xdp_intf_t *xdp_intf)
{
	cJSON *xdp_path = cJSON_GetObjectItem(jsonobj, "xdp_path");
	cJSON *pcapfile = cJSON_GetObjectItem(jsonobj, "pcapfile");
	cJSON *xdp_flag = cJSON_GetObjectItem(jsonobj, "xdp_flag");

	if (xdp_path == NULL) {
		print_err("Missing path for xdp program to load.\n");
		return -EINVAL;
	} else if (cJSON_IsString(xdp_path)) {
		strcpy(xdp_intf->xdp_path, xdp_path->valuestring);
	}

	if (pcapfile == NULL) {
		xdp_intf->pcapfile = NULL;
		print_err(
			"Warning: Missing pcapfile. Packet capture will not be available for the interface.\n");
	} else if (cJSON_IsString(pcapfile)) {
		strcpy(xdp_intf->pcapfile, pcapfile->valuestring);
	}
	if (xdp_flag == NULL) {
		print_err(
			"Warning: Missing XDP program load mode.\n");
		return -EINVAL;
	} else if (cJSON_IsString(xdp_flag)) {
		xdp_intf->xdp_flag = atoi(xdp_flag->valuestring);
	}
	return 0;
}

int trn_cli_parse_port_key(const cJSON *jsonobj,
			   struct rpc_trn_port_key_t *port)
{
	cJSON *tunnel_id = cJSON_GetObjectItem(jsonobj, "tunnel_id");
	cJSON *ip = cJSON_GetObjectItem(jsonobj, "ip");
	cJSON *front_port = cJSON_GetObjectItem(jsonobj, "port");
	cJSON *protocol = cJSON_GetObjectItem(jsonobj, "protocol");

	if (tunnel_id == NULL) {
		port->tunid = 0;
		print_err("Warning: Tunnel ID default is used: %ld\n",
			  port->tunid);
	} else if (cJSON_IsString(tunnel_id)) {
		port->tunid = atoi(tunnel_id->valuestring);
	} else {
		print_err("Error: Tunnel ID Error\n");
		return -EINVAL;
	}

	if (ip != NULL && cJSON_IsString(ip)) {
		struct sockaddr_in sa;
		inet_pton(AF_INET, ip->valuestring, &(sa.sin_addr));
		port->ip = sa.sin_addr.s_addr;
	} else {
		print_err("Error: IP is missing or non-string\n");
		return -EINVAL;
	}

	if (front_port != NULL && cJSON_IsString(front_port)) {
		port->port = htons(atoi(front_port->valuestring));
	} else {
		print_err("Error: Port is missing or non-string\n");
		return -EINVAL;
	}

	if (protocol != NULL && cJSON_IsString(protocol)) {
		port->protocol = htons(atoi(protocol->valuestring));
	} else {
		print_err("Error: Port is missing or non-string\n");
		return -EINVAL;
	}

	return 0;
}

int trn_cli_parse_port(const cJSON *jsonobj, struct rpc_trn_port_t *port)
{
	cJSON *tunnel_id = cJSON_GetObjectItem(jsonobj, "tunnel_id");
	cJSON *ip = cJSON_GetObjectItem(jsonobj, "ip");
	cJSON *front_port = cJSON_GetObjectItem(jsonobj, "port");
	cJSON *target_port = cJSON_GetObjectItem(jsonobj, "target_port");
	cJSON *protocol = cJSON_GetObjectItem(jsonobj, "protocol");

	if (tunnel_id == NULL) {
		port->tunid = 0;
		print_err("Warning: Tunnel ID default is used: %ld\n",
			  port->tunid);
	} else if (cJSON_IsString(tunnel_id)) {
		port->tunid = atoi(tunnel_id->valuestring);
	} else {
		print_err("Error: Tunnel ID Error\n");
		return -EINVAL;
	}

	if (ip != NULL && cJSON_IsString(ip)) {
		struct sockaddr_in sa;
		inet_pton(AF_INET, ip->valuestring, &(sa.sin_addr));
		port->ip = sa.sin_addr.s_addr;
	} else {
		print_err("Error: IP is missing or non-string\n");
		return -EINVAL;
	}

	if (front_port != NULL && cJSON_IsString(front_port)) {
		port->port = htons(atoi(front_port->valuestring));
	} else {
		print_err("Error: Port is missing or non-string\n");
		return -EINVAL;
	}

	if (target_port != NULL && cJSON_IsString(target_port)) {
		port->target_port = htons(atoi(target_port->valuestring));
	} else {
		print_err("Error: Target Port is missing or non-string\n");
		return -EINVAL;
	}

	if (protocol != NULL && cJSON_IsString(protocol)) {
		port->protocol = atoi(protocol->valuestring);
	} else {
		print_err("Error: Port Protocol is missing or non-string\n");
		return -EINVAL;
	}

	return 0;
}

int trn_cli_parse_ep(const cJSON *jsonobj, struct rpc_trn_endpoint_t *ep)
{
	cJSON *tunnel_id = cJSON_GetObjectItem(jsonobj, "tunnel_id");
	cJSON *ip = cJSON_GetObjectItem(jsonobj, "ip");
	cJSON *eptype = cJSON_GetObjectItem(jsonobj, "eptype");
	cJSON *mac = cJSON_GetObjectItem(jsonobj, "mac");
	cJSON *veth = cJSON_GetObjectItem(jsonobj, "veth");
	cJSON *remote_ips = cJSON_GetObjectItem(jsonobj, "remote_ips");
	cJSON *remote_ip = NULL;
	cJSON *hosted_iface = cJSON_GetObjectItem(jsonobj, "hosted_iface");

	if (veth == NULL) {
		ep->veth = NULL;
		print_err("Warning: missing veth.\n");
	} else if (cJSON_IsString(
			   veth)) { // This transit is hosting the endpoint
		strcpy(ep->veth, veth->valuestring);
	}

	if (hosted_iface == NULL) {
		ep->hosted_interface = NULL;
		print_err(
			"Warning: missing hosted interface, using default.\n");
	} else if (cJSON_IsString(
			   hosted_iface)) { // This transit is hosting the endpoint
		strcpy(ep->hosted_interface, hosted_iface->valuestring);
	}

	if (tunnel_id == NULL) {
		ep->tunid = 0;
		print_err("Warning: Tunnel ID default is used: %ld\n",
			  ep->tunid);
	} else if (cJSON_IsString(tunnel_id)) {
		ep->tunid = atoi(tunnel_id->valuestring);
	} else {
		print_err("Error: Tunnel ID Error\n");
		return -EINVAL;
	}

	if (ip != NULL && cJSON_IsString(ip)) {
		struct sockaddr_in sa;
		inet_pton(AF_INET, ip->valuestring, &(sa.sin_addr));
		ep->ip = sa.sin_addr.s_addr;
	} else {
		print_err("Error: IP is missing or non-string\n");
		return -EINVAL;
	}

	if (eptype == NULL) {
		ep->eptype = 0;
		print_err("Warning: EP TYpe (default): %d\n", ep->eptype);
	} else if (cJSON_IsString(eptype)) {
		ep->eptype = atoi(eptype->valuestring);
	} else {
		print_err("Error: eptype ID Error\n");
		return -EINVAL;
	}

	if (mac != NULL && cJSON_IsString(mac)) {
		if (6 == sscanf(mac->valuestring,
				"%hhx:%hhx:%hhx:%hhx:%hhx:%hhx%*c", &ep->mac[0],
				&ep->mac[1], &ep->mac[2], &ep->mac[3],
				&ep->mac[4], &ep->mac[5])) {
		} else {
			/* invalid mac */
			print_err("Error: Invalid MAC\n");
			return -EINVAL;
		}
	} else {
		print_err("MAC is missing or non-string\n");
		return -EINVAL;
	}

	int i = 0;
	ep->remote_ips.remote_ips_len = 0;
	cJSON_ArrayForEach(remote_ip, remote_ips)
	{
		if (cJSON_IsString(remote_ip)) {
			struct sockaddr_in sa;
			inet_pton(AF_INET, remote_ip->valuestring,
				  &(sa.sin_addr));
			ep->remote_ips.remote_ips_val[i] = sa.sin_addr.s_addr;
			ep->remote_ips.remote_ips_len++;
		} else {
			print_err("Error: Remote IP is non-string\n");
			return -EINVAL;
		}
		i++;
		if (i == RPC_TRN_MAX_REMOTE_IPS) {
			print_err("Warning: Remote IPS reached max limited\n");
			break;
		}
	}

	return 0;
}

int trn_cli_parse_ep_key(const cJSON *jsonobj,
			 struct rpc_trn_endpoint_key_t *ep)
{
	cJSON *tunnel_id = cJSON_GetObjectItem(jsonobj, "tunnel_id");
	cJSON *ip = cJSON_GetObjectItem(jsonobj, "ip");

	if (tunnel_id == NULL) {
		ep->tunid = 0;
		print_err("Warning: Tunnel ID default is used: %ld\n",
			  ep->tunid);
	} else if (cJSON_IsString(tunnel_id)) {
		ep->tunid = atoi(tunnel_id->valuestring);
	} else {
		print_err("Error: Tunnel ID Error\n");
		return -EINVAL;
	}

	if (ip != NULL && cJSON_IsString(ip)) {
		struct sockaddr_in sa;
		inet_pton(AF_INET, ip->valuestring, &(sa.sin_addr));
		ep->ip = sa.sin_addr.s_addr;
	} else {
		print_err("Error: IP is missing or non-string\n");
		return -EINVAL;
	}

	return 0;
}

int trn_cli_parse_packet_metadata(const cJSON *jsonobj, struct rpc_trn_packet_metadata_t *packet_metadata)
{
	cJSON *tunnel_id = cJSON_GetObjectItem(jsonobj, "tunnel_id");
	cJSON *ip = cJSON_GetObjectItem(jsonobj, "ip");
	cJSON *pod_label_value = cJSON_GetObjectItem(jsonobj, "pod_label_value");
	cJSON *namespace_label_value = cJSON_GetObjectItem(jsonobj, "namespace_label_value");

	if (cJSON_IsString(tunnel_id)) {
		packet_metadata->tunid = atoi(tunnel_id->valuestring);
	} else {
		print_err("Error: Tunnel ID Error\n");
		return -EINVAL;
	}

	if (ip != NULL && cJSON_IsString(ip)) {
		struct sockaddr_in sa;
		inet_pton(AF_INET, ip->valuestring, &(sa.sin_addr));
		packet_metadata->ip = sa.sin_addr.s_addr;
	} else {
		print_err("Error: IP is missing or non-string\n");
		return -EINVAL;
	}

	if (pod_label_value == NULL) {
		packet_metadata->pod_label_value = 0;
	} else if (cJSON_IsString(pod_label_value)) {
		packet_metadata->pod_label_value = atoi(pod_label_value->valuestring);
	} else if (cJSON_IsNumber(pod_label_value)) {
		packet_metadata->pod_label_value = pod_label_value->valueint;
	} else {
		print_err("Error: pod_label_value Error\n");
		return -EINVAL;
	}

	if (namespace_label_value == NULL) {
		packet_metadata->namespace_label_value = 0;
	} else if (cJSON_IsString(namespace_label_value)) {
		packet_metadata->namespace_label_value = atoi(namespace_label_value->valuestring);
	} else if (cJSON_IsNumber(namespace_label_value)) {
		packet_metadata->namespace_label_value = namespace_label_value->valueint;	
	} else {
		print_err("Error: namespace_label_value Error\n");
		return -EINVAL;
	}

	return 0;
}

int trn_cli_parse_packet_metadata_key(const cJSON *jsonobj,
			 struct rpc_trn_packet_metadata_key_t *packet_metadata)
{
	cJSON *tunnel_id = cJSON_GetObjectItem(jsonobj, "tunnel_id");
	cJSON *ip = cJSON_GetObjectItem(jsonobj, "ip");

	if (cJSON_IsString(tunnel_id)) {
		packet_metadata->tunid = atoi(tunnel_id->valuestring);
	} else {
		print_err("Error: Tunnel ID Error\n");
		return -EINVAL;
	}

	if (ip != NULL && cJSON_IsString(ip)) {
		struct sockaddr_in sa;
		inet_pton(AF_INET, ip->valuestring, &(sa.sin_addr));
		packet_metadata->ip = sa.sin_addr.s_addr;
	} else {
		print_err("Error: IP is missing or non-string\n");
		return -EINVAL;
	}

	return 0;
}

int trn_cli_parse_tun_intf(const cJSON *jsonobj, rpc_trn_tun_intf_t *itf)
{
	cJSON *ip = cJSON_GetObjectItem(jsonobj, "ip");
	cJSON *mac = cJSON_GetObjectItem(jsonobj, "mac");
	cJSON *iface = cJSON_GetObjectItem(jsonobj, "iface");

	if (iface == NULL) {
		itf->interface = NULL;
		print_err("Error: Missing hosted interface, using default.\n");
		return -EINVAL;
	} else if (cJSON_IsString(iface)) {
		strcpy(itf->interface, iface->valuestring);
	}

	if (mac != NULL && cJSON_IsString(mac)) {
		if (6 == sscanf(mac->valuestring,
				"%hhx:%hhx:%hhx:%hhx:%hhx:%hhx%*c",
				&itf->mac[0], &itf->mac[1], &itf->mac[2],
				&itf->mac[3], &itf->mac[4], &itf->mac[5])) {
		} else {
			/* invalid mac */
			print_err("Error: Invalid MAC\n");
			return -EINVAL;
		}
	} else {
		print_err("Error: MAC is missing or non-string\n");
		return -EINVAL;
	}

	if (ip != NULL && cJSON_IsString(ip)) {
		struct sockaddr_in sa;
		inet_pton(AF_INET, ip->valuestring, &(sa.sin_addr));
		itf->ip = sa.sin_addr.s_addr;
	} else {
		print_err("Error: IP is missing or non-string\n");
		return -EINVAL;
	}
	return 0;
}

int trn_cli_parse_agent_md(const cJSON *jsonobj,
			   rpc_trn_agent_metadata_t *agent_md)
{
	cJSON *ep = cJSON_GetObjectItem(jsonobj, "ep");
	cJSON *net = cJSON_GetObjectItem(jsonobj, "net");
	cJSON *eth = cJSON_GetObjectItem(jsonobj, "eth");
	int err_ep, err_net, err_eth;
	err_ep = trn_cli_parse_ep(ep, &agent_md->ep);
	err_net = trn_cli_parse_net(net, &agent_md->net);
	err_eth = trn_cli_parse_tun_intf(eth, &agent_md->eth);

	if (err_ep || err_net || err_eth) {
		return -EINVAL;
	}

	return 0;
}

int trn_cli_read_conf_str(ketopt_t *om, int argc, char *argv[],
			  struct cli_conf_data_t *conf)
{
	int i, c;
	if (conf == NULL) {
		return -EINVAL;
	}

	char conf_file[4096] = "";

	while ((c = ketopt(om, argc, argv, 0, "f:j:i:", 0)) >= 0) {
		if (c == 'j') {
			strcpy(conf->conf_str, om->arg);
		} else if (c == 'f') {
			strcpy(conf_file, om->arg);
		} else if (c == 'i') {
			strcpy(conf->intf, om->arg);
		}
	}

	if (conf->intf[0] == 0) {
		fprintf(stderr, "Missing interface.\n");
		return -EINVAL;
	}

	if (conf->conf_str[0] == 0 && conf_file[0] == 0) {
		fprintf(stderr,
			"Missing configuration string or configuration file.\n");
		return -EINVAL;
	}

	if (conf->conf_str[0] && conf_file[0]) {
		fprintf(stderr,
			"Either configuration string or configuration file is expected. Providing both is ambiguous.\n");
		return -EINVAL;
	}

	if (conf_file[0]) {
		printf("Reading Configuration file: %s\n", conf_file);

		FILE *f = fopen(conf_file, "rb");
		fseek(f, 0, SEEK_END);
		size_t fsize = ftell(f);
		size_t size = fsize + 1;
		fseek(f, 0, SEEK_SET);

		if (fsize > 4096 ||
		    fread(conf->conf_str, 1, fsize, f) < fsize) {
			fprintf(stderr,
				"Configuration file partially loaded.\n");
			exit(1);
		}
		fclose(f);

		conf->conf_str[fsize] = 0;
	}
	return 0;
}

int trn_cli_parse_ebpf_prog(const cJSON *jsonobj, rpc_trn_ebpf_prog_t *prog)
{
	cJSON *xdp_path = cJSON_GetObjectItem(jsonobj, "xdp_path");
	cJSON *stage = cJSON_GetObjectItem(jsonobj, "stage");

	if (xdp_path == NULL) {
		print_err("Missing path for xdp program to load.\n");
		return -EINVAL;
	} else if (cJSON_IsString(xdp_path)) {
		strcpy(prog->xdp_path, xdp_path->valuestring);
	}

	if (stage == NULL) {
		print_err("Error missing pipeline stage.\n");
		return -EINVAL;
	} else if (cJSON_IsString(stage)) {
		const char *stage_str = stage->valuestring;

		if (strcmp(stage_str, "ON_XDP_TX") == 0) {
			prog->stage = ON_XDP_TX;
		} else if (strcmp(stage_str, "ON_XDP_PASS") == 0) {
			prog->stage = ON_XDP_PASS;
		} else if (strcmp(stage_str, "ON_XDP_REDIRECT") == 0) {
			prog->stage = ON_XDP_REDIRECT;
		} else if (strcmp(stage_str, "ON_XDP_DROP") == 0) {
			prog->stage = ON_XDP_DROP;
		} else if (strcmp(stage_str, "ON_XDP_SCALED_EP") == 0) {
			prog->stage = ON_XDP_SCALED_EP;
		} else {
			print_err("Unsupported pipeline stage %s.\n",
				  stage_str);
			return -EINVAL;
		}
	}
	return 0;
}

int trn_cli_parse_ebpf_prog_stage(const cJSON *jsonobj,
				  rpc_trn_ebpf_prog_stage_t *prog_stage)
{
	cJSON *stage = cJSON_GetObjectItem(jsonobj, "stage");

	if (stage == NULL) {
		print_err("Error missing pipeline stage.\n");
		return -EINVAL;
	} else if (cJSON_IsString(stage)) {
		const char *stage_str = stage->valuestring;

		if (strcmp(stage_str, "ON_XDP_TX") == 0) {
			prog_stage->stage = ON_XDP_TX;
		} else if (strcmp(stage_str, "ON_XDP_PASS") == 0) {
			prog_stage->stage = ON_XDP_PASS;
		} else if (strcmp(stage_str, "ON_XDP_REDIRECT") == 0) {
			prog_stage->stage = ON_XDP_REDIRECT;
		} else if (strcmp(stage_str, "ON_XDP_DROP") == 0) {
			prog_stage->stage = ON_XDP_DROP;
		} else if (strcmp(stage_str, "ON_XDP_SCALED_EP") == 0) {
			prog_stage->stage = ON_XDP_SCALED_EP;
		} else {
			print_err("Unsupported pipeline stage %s.\n",
				  stage_str);
			return -EINVAL;
		}
	}
	return 0;
}

int trn_cli_parse_network_policy_cidr(const cJSON *jsonobj,
				      struct rpc_trn_vsip_cidr_t *cidrval)
{
	cJSON *tunnel_id = cJSON_GetObjectItem(jsonobj, "tunnel_id");
	cJSON *local_ip = cJSON_GetObjectItem(jsonobj, "local_ip");
	cJSON *cidr_prefixlen = cJSON_GetObjectItem(jsonobj, "cidr_prefixlen");
	cJSON *cidr_ip = cJSON_GetObjectItem(jsonobj, "cidr_ip");
	cJSON *cidr_type = cJSON_GetObjectItem(jsonobj, "cidr_type");
	cJSON *bit_val = cJSON_GetObjectItem(jsonobj, "bit_value");

	if (tunnel_id == NULL) {
		cidrval->tunid = 0;
	} else if (cJSON_IsString(tunnel_id)) {
		cidrval->tunid = atoi(tunnel_id->valuestring);
	} else {
		print_err("Error: Network policy tunnel_id is non-string.\n");
		return -EINVAL;
	}

	if (local_ip != NULL && cJSON_IsString(local_ip)) {
		cidrval->local_ip = parse_ip_address(local_ip);
	} else {
		print_err("Error: Network policy local IP is missing or non-string\n");
		return -EINVAL;
	}

	if (cidr_ip != NULL && cJSON_IsString(cidr_ip)) {
		cidrval->cidr_ip = parse_ip_address(cidr_ip);
	} else {
		print_err("Error: Network policy CIDR IP is missing or non-string\n");
		return -EINVAL;
	}

	if (cJSON_IsString(cidr_prefixlen)) {
		cidrval->cidr_prefixlen = atoi(cidr_prefixlen->valuestring);
	} else {
		print_err("Error: Network policy prefixlen Error\n");
		return -EINVAL;
	}

	if (cJSON_IsString(cidr_type)) {
		cidrval->cidr_type = atoi(cidr_type->valuestring);
	} else {
		print_err("Error: Network Policy cidr type Error\n");
		return -EINVAL;
	}

	if (cJSON_IsString(bit_val)) {
		cidrval->bit_val = atoi(bit_val->valuestring);
	} else {
		print_err("Error: Network policy bit map Error\n");
		return -EINVAL;
	}

	return 0;
}

int trn_cli_parse_network_policy_cidr_key(const cJSON *jsonobj,
					  struct rpc_trn_vsip_cidr_key_t *cidrkey)
{
	cJSON *tunnel_id = cJSON_GetObjectItem(jsonobj, "tunnel_id");
	cJSON *local_ip = cJSON_GetObjectItem(jsonobj, "local_ip");
	cJSON *cidr_prefixlen = cJSON_GetObjectItem(jsonobj, "cidr_prefixlen");
	cJSON *cidr_ip = cJSON_GetObjectItem(jsonobj, "cidr_ip");
	cJSON *cidr_type = cJSON_GetObjectItem(jsonobj, "cidr_type");

	if (tunnel_id == NULL) {
		cidrkey->tunid = 0;
	} else if (cJSON_IsString(tunnel_id)) {
		cidrkey->tunid = atoi(tunnel_id->valuestring);
	} else {
		print_err("Error: Network policy tunnel_id is non-string.\n");
		return -EINVAL;
	}

	if (local_ip != NULL && cJSON_IsString(local_ip)) {
		cidrkey->local_ip = parse_ip_address(local_ip);
	} else {
		print_err("Error: Network policy local IP is missing or non-string\n");
		return -EINVAL;
	}

	if (cidr_ip != NULL && cJSON_IsString(cidr_ip)) {
		cidrkey->cidr_ip = parse_ip_address(cidr_ip);
	} else {
		print_err("Error: Network policy CIDR IP is missing or non-string\n");
		return -EINVAL;
	}

	if (cJSON_IsString(cidr_prefixlen)) {
		cidrkey->cidr_prefixlen = atoi(cidr_prefixlen->valuestring);
	} else {
		print_err("Error: Network policy prefixlen Error\n");
		return -EINVAL;
	}

	if (cJSON_IsString(cidr_type)) {
		cidrkey->cidr_type = atoi(cidr_type->valuestring);
	} else {
		print_err("Error: Network Policy cidr type Error\n");
		return -EINVAL;
	}

	return 0;
}

int trn_cli_parse_network_policy_enforcement(const cJSON *jsonobj,
					     struct rpc_trn_vsip_enforce_t *enforce)
{
	cJSON *tunnel_id = cJSON_GetObjectItem(jsonobj, "tunnel_id");
	cJSON *ip = cJSON_GetObjectItem(jsonobj, "ip");

	if (tunnel_id == NULL) {
		enforce->tunid = 0;
	} else if (cJSON_IsString(tunnel_id)) {
		enforce->tunid = atoi(tunnel_id->valuestring);
	} else {
		print_err("Error: Network policy enforcement tunnel_id is non-string.\n");
		return -EINVAL;
	}

	if (ip != NULL && cJSON_IsString(ip)) {
		enforce->local_ip = parse_ip_address(ip);
	} else {
		print_err("Error: Network policy enforcement IP is missing or non-string\n");
		return -EINVAL;
	}

	return 0;
}

int trn_cli_parse_network_policy_protocol_port(const cJSON *jsonobj,
				      	       struct rpc_trn_vsip_ppo_t *ppo)
{
	cJSON *tunnel_id = cJSON_GetObjectItem(jsonobj, "tunnel_id");
	cJSON *local_ip = cJSON_GetObjectItem(jsonobj, "local_ip");
	cJSON *protocol = cJSON_GetObjectItem(jsonobj, "protocol");
	cJSON *port = cJSON_GetObjectItem(jsonobj, "port");
	cJSON *bit_val = cJSON_GetObjectItem(jsonobj, "bit_value");

	if (tunnel_id == NULL) {
		ppo->tunid = 0;
	} else if (cJSON_IsString(tunnel_id)) {
		ppo->tunid = atoi(tunnel_id->valuestring);
	} else {
		print_err("Error: Network policy tunnel_id is non-string.\n");
		return -EINVAL;
	}

	if (local_ip != NULL && cJSON_IsString(local_ip)) {
		ppo->local_ip = parse_ip_address(local_ip);
	} else {
		print_err("Error: Network policy local IP is missing or non-string\n");
		return -EINVAL;
	}

	if (cJSON_IsString(protocol)) {
		ppo->proto = atoi(protocol->valuestring);
	} else {
		print_err("Error: Network policy protocol Error\n");
		return -EINVAL;
	}

	if (cJSON_IsString(port)) {
		ppo->port = htons(atoi(port->valuestring));
	} else {
		print_err("Error: Network policy port Error\n");
		return -EINVAL;
	}

	if (cJSON_IsString(bit_val)) {
		ppo->bit_val = atoi(bit_val->valuestring);
	} else {
		print_err("Error: Network policy bit map Error\n");
		return -EINVAL;
	}

	return 0;
}

int trn_cli_parse_network_policy_protocol_port_key(const cJSON *jsonobj,
						   struct rpc_trn_vsip_ppo_key_t *ppo_key)
{
	cJSON *tunnel_id = cJSON_GetObjectItem(jsonobj, "tunnel_id");
	cJSON *local_ip = cJSON_GetObjectItem(jsonobj, "local_ip");
	cJSON *protocol = cJSON_GetObjectItem(jsonobj, "protocol");
	cJSON *port = cJSON_GetObjectItem(jsonobj, "port");

	if (tunnel_id == NULL) {
		ppo_key->tunid = 0;
	} else if (cJSON_IsString(tunnel_id)) {
		ppo_key->tunid = atoi(tunnel_id->valuestring);
	} else {
		print_err("Error: Network policy tunnel_id is non-string.\n");
		return -EINVAL;
	}

	if (local_ip != NULL && cJSON_IsString(local_ip)) {
		ppo_key->local_ip = parse_ip_address(local_ip);
	} else {
		print_err("Error: Network policy local IP is missing or non-string\n");
		return -EINVAL;
	}

	if (cJSON_IsString(protocol)) {
		ppo_key->proto = atoi(protocol->valuestring);
	} else {
		print_err("Error: Network policy protocol Error\n");
		return -EINVAL;
	}

	if (cJSON_IsString(port)) {
		ppo_key->port = htons(atoi(port->valuestring));
	} else {
		print_err("Error: Network policy port Error\n");
		return -EINVAL;
	}

	return 0;
}

uint32_t parse_ip_address(const cJSON *ipobj)
{
	struct sockaddr_in sa;
	inet_pton(AF_INET, ipobj->valuestring, &(sa.sin_addr));
	return sa.sin_addr.s_addr;
}
