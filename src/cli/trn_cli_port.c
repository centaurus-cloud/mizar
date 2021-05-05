// SPDX-License-Identifier: GPL-2.0-or-later
/**
 * @file trn_cli_port.c
 * @author Phu Tran          (@phudtran)
 *
 *
 * @brief CLI subcommands related to endpoints
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

int trn_cli_update_port_subcmd(CLIENT *clnt, int argc, char *argv[])
{
	ketopt_t om = KETOPT_INIT;
	struct cli_conf_data_t conf;
	cJSON *json_str = NULL;

	if (trn_cli_read_conf_str(&om, argc, argv, &conf)) {
		return -EINVAL;
	}

	char *buf = conf.conf_str;
	json_str = trn_cli_parse_json(buf);

	if (json_str == NULL) {
		return -EINVAL;
	}

	int *rc;
	rpc_trn_port_t port;
	char rpc[] = "update_port_1";

	port.interface = conf.intf;

	int err = trn_cli_parse_port(json_str, &port);
	cJSON_Delete(json_str);

	if (err != 0) {
		print_err("Error: parsing port config.\n");
		return -EINVAL;
	}

	rc = update_port_1(&port, clnt);
	if (rc == (int *)NULL) {
		print_err("RPC Error: client call failed: update_port_1.\n");
		return -EINVAL;
	}

	if (*rc != 0) {
		print_err(
			"Error: %s fatal daemon error, see transitd logs for details.\n",
			rpc);
		return -EINVAL;
	}

	dump_port(&port);
	print_msg(
		"update_port_1 successfully updated port %u with protocol %u.\n",
		ntohs(port.target_port), port.protocol);
	return 0;
}

void dump_port(struct rpc_trn_port_t *port)
{
	print_msg("Interface: %s\n", port->interface);
	print_msg("Tunnel ID: %ld\n", port->tunid);
	print_msg("IP: 0x%x\n", port->ip);
	print_msg("Frontend Port: %u\n", ntohs(port->port));
	print_msg("Target Port: %u\n", ntohs(port->target_port));
	print_msg("Target Protocol: %u\n", port->protocol);
}
