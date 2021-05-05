// SPDX-License-Identifier: GPL-2.0-or-later
/**
 * @file trn_cli_packet_metadata.c
 * @author Hong Chang		(@Hong-Chang)
 *
 * @brief Transit Agent cli subcommands related to packet_metadata
 *
 * @copyright Copyright (c) 2021 The Authors.
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

int trn_cli_update_packet_metadata_subcmd(CLIENT *clnt, int argc, char *argv[])
{
	ketopt_t om = KETOPT_INIT;
	cJSON *json_str = NULL;
	struct cli_conf_data_t conf;

	if (trn_cli_read_conf_str(&om, argc, argv, &conf)) {
		return -EINVAL;
	}

	char *buf = conf.conf_str;
	json_str = trn_cli_parse_json(buf);

	if (json_str == NULL) {
		return -EINVAL;
	}

	int *rc;
	rpc_trn_packet_metadata_t packet_metadata;
	char rpc[] = "update_packet_metadata_1";

	packet_metadata.interface = conf.intf;

	int err = trn_cli_parse_packet_metadata(json_str, &packet_metadata);
	cJSON_Delete(json_str);
	json_str = NULL;

	if (err != 0) {
		print_err("Error: parsing packet metadata config.\n");
		return -EINVAL;
	}

	rc = update_packet_metadata_1(&packet_metadata, clnt);
	if (rc == (int *)NULL) {
		print_err("Error: call failed: update_packet_metadata_1.\n");
		return -EINVAL;
	}

	if (*rc != 0) {
		print_err(
			"Error: %s fatal error, see transitd logs for details.\n",
			rpc);
		return -EINVAL;
	}

	print_msg(
		"update_packet_metadata_1 successfully updated packet metadata %d on interface %s.\n",
		packet_metadata.ip, packet_metadata.interface);
	return 0;
}

int trn_cli_delete_packet_metadata_subcmd(CLIENT *clnt, int argc, char *argv[])
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
	rpc_trn_packet_metadata_key_t key;
	char rpc[] = "delete_packet_metadata_1";
	key.interface = conf.intf;

	int err = trn_cli_parse_packet_metadata_key(json_str, &key);
	cJSON_Delete(json_str);

	if (err != 0) {
		print_err("Error: parsing packet metadata config.\n");
		return -EINVAL;
	}
	rc = delete_packet_metadata_1(&key, clnt);
	if (rc == (int *)NULL) {
		print_err("Error: call failed: delete_packet_metadata_1.\n");
		return -EINVAL;
	}

	if (*rc != 0) {
		print_err(
			"Error: %s fatal error, see transitd logs for details.\n",
			rpc);
		return -EINVAL;
	}

	print_msg(
		"delete_packet_metadata_1 successfully deleted packet metadata %d on interface %s.\n",
		key.ip, key.interface);
	return 0;
}

void dump_packet_metadata(struct rpc_trn_packet_metadata_t *packet_metadata)
{
	print_msg("Interface: %s\n", packet_metadata->interface);
	print_msg("Tunnel ID: %ld\n", packet_metadata->tunid);
	print_msg("IP: 0x%x\n", packet_metadata->ip);
}
