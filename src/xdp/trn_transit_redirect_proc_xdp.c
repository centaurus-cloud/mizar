// SPDX-License-Identifier: GPL-2.0-or-later
/**
 * @file trn_transit_xdp.c
 * @author Sherif Abdelwahab (@zasherif)
 *
 * @brief Implements the Transit XDP program (switching and routing logic)
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

#include <linux/bpf.h>
#include <linux/if_arp.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <linux/if_vlan.h>
#include <linux/in.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/pkt_cls.h>
#include <linux/socket.h>
#include <linux/tcp.h>
#include <linux/types.h>
#include <linux/udp.h>
#include <stddef.h>
#include <string.h>

#include "extern/bpf_endian.h"
#include "extern/bpf_helpers.h"

#include "trn_datamodel.h"
#include "trn_kern.h"
#include "trn_transit_xdp_stages_maps.h"

int _version SEC("version") = 1;

SEC("transit_redirect_proc")
int _transit_redirect_proc(struct xdp_md *ctx)
{
	/* Simple example program that gets share same maps with transit XDP
		can be invoked on redirect */
	bpf_debug("[Transit:%d:] redirect processing\n", __LINE__);
	int map_idx = 0;

	void *networks_map, *vpc_map, *endpoints_map, *port_map,
		*hosted_endpoints_iface_map, *interface_config_map,
		*interfaces_map;

	networks_map = bpf_map_lookup_elem(&networks_map_ref, &map_idx);
	if (!networks_map) {
		bpf_debug("[Transit:%d:] failed to find networks_map\n",
			  __LINE__);
		return XDP_ABORTED;
	}
	bpf_debug("[Transit:%d:] found networks_map\n", __LINE__);

	vpc_map = bpf_map_lookup_elem(&vpc_map_ref, &map_idx);
	if (!vpc_map) {
		bpf_debug("[Transit:%d:] failed to find vpc_map\n", __LINE__);
		return XDP_ABORTED;
	}
	bpf_debug("[Transit:%d:] found vpc_map\n", __LINE__);

	endpoints_map = bpf_map_lookup_elem(&endpoints_map_ref, &map_idx);
	if (!endpoints_map) {
		bpf_debug("[Transit:%d:] failed to find endpoints_map\n",
			  __LINE__);
		return XDP_ABORTED;
	}
	bpf_debug("[Transit:%d:] found endpoints_map\n", __LINE__);

	port_map = bpf_map_lookup_elem(&port_map_ref, &map_idx);
	if (!port_map) {
		bpf_debug("[Transit:%d:] failed to find port_map\n", __LINE__);
		return XDP_ABORTED;
	}
	bpf_debug("[Transit:%d:] found port_map\n", __LINE__);

	hosted_endpoints_iface_map =
		bpf_map_lookup_elem(&hosted_endpoints_iface_map_ref, &map_idx);
	if (!hosted_endpoints_iface_map) {
		bpf_debug(
			"[Transit:%d:] failed to find hosted_endpoints_iface_map\n",
			__LINE__);
		return XDP_ABORTED;
	}
	bpf_debug("[Transit:%d:] found hosted_endpoints_iface_map\n", __LINE__);

	interface_config_map =
		bpf_map_lookup_elem(&interface_config_map_ref, &map_idx);
	if (!interface_config_map) {
		bpf_debug("[Transit:%d:] failed to find interface_config_map\n",
			  __LINE__);
		return XDP_ABORTED;
	}
	bpf_debug("[Transit:%d:] found interface_config_map\n", __LINE__);

	interfaces_map = bpf_map_lookup_elem(&interfaces_map_ref, &map_idx);
	if (!interfaces_map) {
		bpf_debug("[Transit:%d:] failed to find interfaces_map\n",
			  __LINE__);
		return XDP_ABORTED;
	}

	bpf_debug("[Transit:%d:] found all inner maps!\n", __LINE__);

	return XDP_REDIRECT;
}

char _license[] SEC("license") = "GPL";
