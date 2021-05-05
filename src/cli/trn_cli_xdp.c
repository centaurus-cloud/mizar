// SPDX-License-Identifier: GPL-2.0-or-later
/**
 * @file trn_cli_xdp.c
 * @author Sherif Abdelwahab (@zasherif)
 *
 * @brief CLI subcommands to loading/unloading manage XDP programs
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

int trn_cli_unload_pipeline_stage_subcmd(CLIENT *clnt, int argc, char *argv[])
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
	rpc_trn_ebpf_prog_stage_t stage;
	char rpc[] = "unload_transit_xdp_pipeline_stage_1";
	stage.interface = conf.intf;

	int err = trn_cli_parse_ebpf_prog_stage(json_str, &stage);
	cJSON_Delete(json_str);

	if (err != 0) {
		print_err("Error: parsing ebpf program stage.\n");
		return -EINVAL;
	}

	rc = unload_transit_xdp_pipeline_stage_1(&stage, clnt);
	if (rc == (int *)NULL) {
		print_err(
			"Error: call failed: unload_transit_xdp_pipeline_stage_1.\n");
		return -EINVAL;
	}

	if (*rc != 0) {
		print_err(
			"Error: %s fatal error, see transitd logs for details.\n",
			rpc);
		return -EINVAL;
	}

	print_msg(
		"unload_transit_xdp_pipeline_stage_1 successfully removed program on interface %s at stage %d.\n",
		stage.interface, stage.stage);

	return 0;
}

int trn_cli_load_pipeline_stage_subcmd(CLIENT *clnt, int argc, char *argv[])
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
	rpc_trn_ebpf_prog_t ebpf_prog;
	char rpc[] = "load_transit_xdp_pipeline_stage_1";
	char xdp_path[1024];
	ebpf_prog.xdp_path = xdp_path;
	ebpf_prog.interface = conf.intf;

	int err = trn_cli_parse_ebpf_prog(json_str, &ebpf_prog);
	cJSON_Delete(json_str);

	if (err != 0) {
		print_err("Error: parsing ebpf program config.\n");
		return -EINVAL;
	}

	rc = load_transit_xdp_pipeline_stage_1(&ebpf_prog, clnt);
	if (rc == (int *)NULL) {
		print_err(
			"Error: call failed: load_transit_xdp_pipeline_stage_1.\n");
		return -EINVAL;
	}

	if (*rc != 0) {
		print_err(
			"Error: %s fatal error, see transitd logs for details.\n",
			rpc);
		return -EINVAL;
	}

	print_msg(
		"load_transit_xdp_pipeline_stage_1 successfully added program on interface %s at stage %d.\n",
		ebpf_prog.interface, ebpf_prog.stage);

	return 0;
}

int trn_cli_load_transit_subcmd(CLIENT *clnt, int argc, char *argv[])
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
	rpc_trn_xdp_intf_t xdp_intf;
	char rpc[] = "load_transit_xdp_1";
	char xdp_path[1024];
	char pcapfile[1024];
	int xdp_flag = XDP_FLAGS_SKB_MODE;
	xdp_intf.xdp_path = xdp_path;
	xdp_intf.pcapfile = pcapfile;
	xdp_intf.xdp_flag = xdp_flag;
	xdp_intf.interface = conf.intf;

	int err = trn_cli_parse_xdp(json_str, &xdp_intf);
	cJSON_Delete(json_str);

	if (err != 0) {
		print_err("Error: parsing XDP path config.\n");
		return -EINVAL;
	}

	rc = load_transit_xdp_1(&xdp_intf, clnt);
	if (rc == (int *)NULL) {
		print_err("Error: call failed: load_transit_xdp_1.\n");
		return -EINVAL;
	}

	if (*rc != 0) {
		print_err(
			"Error: %s fatal error, see transitd logs for details.\n",
			rpc);
		return -EINVAL;
	}

	print_msg(
		"load_transit_xdp_1 successfully loaded transit xdp on interface %s.\n",
		xdp_intf.interface);

	return 0;
}

int trn_cli_unload_transit_subcmd(CLIENT *clnt, int argc, char *argv[])
{
	ketopt_t om = KETOPT_INIT;
	struct cli_conf_data_t conf;

	if (trn_cli_read_conf_str(&om, argc, argv, &conf)) {
		return -EINVAL;
	}

	int *rc;
	char rpc[] = "unload_transit_xdp_1";
	rpc_intf_t intf;
	intf.interface = conf.intf;

	rc = unload_transit_xdp_1(&intf, clnt);
	if (rc == (int *)NULL) {
		print_err("Error: call failed: unload_transit_xdp_1.\n");
		return -EINVAL;
	}

	if (*rc != 0) {
		printf("%s fatal error, see transitd logs for details.\n", rpc);
		return -EINVAL;
	}

	printf("unload_transit_xdp_1 successfully unloaded transit xdp on interface %s.\n",
	       intf.interface);
	return 0;
}

int trn_cli_load_agent_subcmd(CLIENT *clnt, int argc, char *argv[])
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
	rpc_trn_xdp_intf_t xdp_intf;
	char rpc[] = "load_transit_agent_xdp_1";
	char xdp_path[1024];
	char pcapfile[1024];
	xdp_intf.xdp_path = xdp_path;
	xdp_intf.pcapfile = pcapfile;

	xdp_intf.interface = conf.intf;

	int err = trn_cli_parse_xdp(json_str, &xdp_intf);
	cJSON_Delete(json_str);

	if (err != 0) {
		print_err("Error: parsing VPC config.\n");
		return -EINVAL;
	}

	rc = load_transit_agent_xdp_1(&xdp_intf, clnt);

	if (rc == (int *)NULL) {
		print_err("Error: call failed: load_transit_agent_xdp_1.\n");
		return -EINVAL;
	}

	if (*rc != 0) {
		print_err(
			"Error: %s fatal error, see transitd logs for details.\n",
			rpc);
		return -EINVAL;
	}

	print_msg(
		"load_transit_agent_xdp_1 successfully loaded transit agent xdp on interface %s.\n",
		xdp_intf.interface);
	return 0;
}

int trn_cli_unload_agent_subcmd(CLIENT *clnt, int argc, char *argv[])
{
	ketopt_t om = KETOPT_INIT;
	struct cli_conf_data_t conf;

	if (trn_cli_read_conf_str(&om, argc, argv, &conf)) {
		return -EINVAL;
	}

	int *rc;
	char rpc[] = "unload_transit_agent_xdp_1";
	rpc_intf_t intf;
	intf.interface = conf.intf;

	rc = unload_transit_agent_xdp_1(&intf, clnt);
	if (rc == (int *)NULL) {
		print_err("Error: call failed: unload_transit_agent_xdp_1.\n");
		return -EINVAL;
	}

	if (*rc != 0) {
		print_err(
			"Error: %s fatal error, see transitd logs for details.\n",
			rpc);
		return -EINVAL;
	}

	print_msg(
		"unload_transit_agent_xdp_1 successfully unloaded transit xdp on interface %s.\n",
		intf.interface);
	return 0;
}
