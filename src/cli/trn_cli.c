// SPDX-License-Identifier: GPL-2.0-or-later
/**
 * @file trn_cli.c
 * @author Sherif Abdelwahab (@zasherif)
 *         Phu Tran          (@phudtran)
 *
 * @brief Transit frontend
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

static CLIENT *clnt;

static const struct cmd {
	const char *cmd;
	int (*func)(CLIENT *clnt, int argc, char **argv);
} cmds[] = {
	{ "update-vpc", trn_cli_update_vpc_subcmd },
	{ "update-net", trn_cli_update_net_subcmd },
	{ "update-ep", trn_cli_update_ep_subcmd },
	{ "update-port", trn_cli_update_port_subcmd },
	{ "update-agent-ep", trn_cli_update_agent_ep_subcmd },
	{ "update-packet-metadata", trn_cli_update_packet_metadata_subcmd },
	{ "update-agent-metadata", trn_cli_update_agent_md_subcmd },
	{ "load-transit-xdp", trn_cli_load_transit_subcmd },
	{ "load-agent-xdp", trn_cli_load_agent_subcmd },
	{ "unload-transit-xdp", trn_cli_unload_transit_subcmd },
	{ "unload-agent-xdp", trn_cli_unload_agent_subcmd },
	{ "get-vpc", trn_cli_get_vpc_subcmd },
	{ "get-net", trn_cli_get_net_subcmd },
	{ "get-ep", trn_cli_get_ep_subcmd },
	{ "get-agent-ep", trn_cli_get_agent_ep_subcmd },
	{ "get-agent-metadata", trn_cli_get_agent_md_subcmd },
	{ "delete-vpc", trn_cli_delete_vpc_subcmd },
	{ "delete-net", trn_cli_delete_net_subcmd },
	{ "delete-ep", trn_cli_delete_ep_subcmd },
	{ "delete-agent-ep", trn_cli_delete_agent_ep_subcmd },
	{ "delete-packet-metadata", trn_cli_delete_packet_metadata_subcmd },
	{ "delete-agent-metadata", trn_cli_delete_agent_md_subcmd },
	{ "load-pipeline-stage", trn_cli_load_pipeline_stage_subcmd },
	{ "unload-pipeline-stage", trn_cli_unload_pipeline_stage_subcmd },
	{ "update-net-policy-in", trn_cli_update_transit_network_policy_subcmd },
	{ "update-net-policy-out", trn_cli_update_agent_network_policy_subcmd },
	{ "delete-net-policy-in", trn_cli_delete_transit_network_policy_subcmd },
	{ "delete-net-policy-out", trn_cli_delete_agent_network_policy_subcmd },
	{ "update-net-policy-enforce-in", trn_cli_update_transit_network_policy_enforcement_subcmd },
	{ "update-net-policy-enforce-out", trn_cli_update_agent_network_policy_enforcement_subcmd },
	{ "delete-net-policy-enforce-in", trn_cli_delete_transit_network_policy_enforcement_subcmd },
	{ "delete-net-policy-enforce-out", trn_cli_delete_agent_network_policy_enforcement_subcmd },
	{ "update-net-policy-protocol-port-in", trn_cli_update_transit_network_policy_protocol_port_subcmd },
	{ "update-net-policy-protocol-port-out", trn_cli_update_agent_network_policy_protocol_port_subcmd },
	{ "delete-net-policy-protocol-port-in", trn_cli_delete_transit_network_policy_protocol_port_subcmd },
	{ "delete-net-policy-protocol-port-out", trn_cli_delete_agent_network_policy_protocol_port_subcmd },
	{ 0 },
};

void cleanup(int alloced, void *ptr)
{
	if (alloced) {
		free(ptr);
		ptr = NULL;
	}
}

int main(int argc, char *argv[])
{
	ketopt_t om = KETOPT_INIT, os = KETOPT_INIT;
	int i, o;
	char LOCALHOST[] = "localhost";
	char UDP[] = "udp";
	char *server = NULL;
	char *protocol = NULL;
	int rc = 0;
	const struct cmd *c;
	int malloc_server = 0;
	int malloc_protocol = 0;

	while ((o = ketopt(&om, argc, argv, 0, "s:p:", 0)) >= 0) {
		if (o == 's') {
			server = malloc(sizeof(char) * strlen(om.arg) + 1);
			strcpy(server, om.arg);
			printf("main -s %s\n", server);
			malloc_server = 1;
		}
		if (o == 'p') {
			protocol = malloc(sizeof(char) * strlen(om.arg) + 1);
			strcpy(protocol, om.arg);
			printf("main -s %s\n", server);
			malloc_protocol = 1;
		}
	}

	if (server == NULL) {
		server = LOCALHOST;
	}

	if (protocol == NULL) {
		protocol = UDP;
	}

	printf("Connecting to %s using %s protocol.\n", server, protocol);

	clnt = clnt_create(server, RPC_TRANSIT_REMOTE_PROTOCOL,
			   RPC_TRANSIT_ALFAZERO, protocol);

	if (clnt == NULL) {
		clnt_pcreateerror(server);
		cleanup(malloc_server, server);
		cleanup(malloc_protocol, protocol);
		exit(1);
	}

	if (om.ind == argc) {
		fprintf(stderr, "missing subcommand.\n");
		cleanup(malloc_server, server);
		cleanup(malloc_protocol, protocol);
		exit(1);
	}

	for (c = cmds; c->cmd; ++c) {
		if (strcmp(argv[om.ind], c->cmd) == 0) {
			rc = (c->func(clnt, argc - om.ind, argv + om.ind));
			break;
		}
	}

	if (rc) {
		cleanup(malloc_server, server);
		cleanup(malloc_protocol, protocol);
		exit(1);
	}

	clnt_destroy(clnt);
	cleanup(malloc_server, server);
	cleanup(malloc_protocol, protocol);

	return 0;
}
