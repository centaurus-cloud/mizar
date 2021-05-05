# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Authors.

# Authors: Sherif Abdelwahab <@zasherif>
#          Phu Tran          <@phudtran>

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:The above copyright
# notice and this permission notice shall be included in all copies or
# substantial portions of the Software.THE SOFTWARE IS PROVIDED "AS IS",
# WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
# TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
# FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
# TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
# THE USE OR OTHER DEALINGS IN THE SOFTWARE.

import logging
from mizar.common.rpc import TrnRpc
from mizar.common.constants import *
from mizar.common.common import *
from mizar.common.cidr import Cidr

logger = logging.getLogger()


class Bouncer(object):
    def __init__(self, name, obj_api, opr_store, spec=None):
        self.name = name
        self.obj_api = obj_api
        self.store = opr_store
        self.droplet = ""
        self.droplet_obj = None
        self.vpc = ""
        self.vni = ""
        self.nip = ""
        self.prefix = ""
        self.net = ""
        self.ip = ""
        self.mac = ""
        self.eps = {}
        self.dividers = {}
        self.known_substrates = {}
        self.status = OBJ_STATUS.bouncer_status_init
        self.scaled_ep_obj = '/trn_xdp/trn_transit_scaled_endpoint_ebpf_debug.o'
        if spec is not None:
            self.set_obj_spec(spec)

    @property
    def rpc(self):
        return TrnRpc(self.ip, self.mac)

    def get_obj_spec(self):
        self.obj = {
            "vpc": self.vpc,
            "net": self.net,
            "ip": self.ip,
            "mac": self.mac,
            "status": self.status,
            "droplet": self.droplet,
            "vni": self.vni,
            "nip": self.nip,
            "prefix": self.prefix
        }

        return self.obj

    def set_obj_spec(self, spec):
        self.status = get_spec_val('status', spec)
        self.vpc = get_spec_val('vpc', spec)
        self.net = get_spec_val('net', spec)
        self.ip = get_spec_val('ip', spec)
        self.mac = get_spec_val('mac', spec)
        self.droplet = get_spec_val('droplet', spec)
        self.vni = get_spec_val('vni', spec)
        self.nip = get_spec_val('nip', spec)
        self.prefix = get_spec_val('prefix', spec)

    # K8s APIs
    def get_name(self):
        return self.name

    def get_plural(self):
        return "bouncers"

    def get_kind(self):
        return "Bouncer"

    def get_divider_ips(self):
        divider_ips = []
        for d in self.dividers.values():
            if d.ip not in divider_ips:
                divider_ips.append(d.ip)
        return divider_ips

    def get_nip(self):
        return self.nip

    def get_prefixlen(self):
        return self.prefix

    def store_update_obj(self):
        if self.store is None:
            return
        self.store.update_bouncer(self)

    def store_delete_obj(self):
        if self.store is None:
            return
        self.store.delete_bouncer(self.name)

    def create_obj(self):
        return kube_create_obj(self)

    def update_obj(self):
        return kube_update_obj(self)

    def delete_obj(self):
        return kube_delete_obj(self)

    def watch_obj(self, watch_callback):
        return kube_watch_obj(self, watch_callback)

    def set_status(self, status):
        self.status = status

    def load_transit_xdp_pipeline_stage(self):
        self.rpc.load_transit_xdp_pipeline_stage(CONSTANTS.ON_XDP_SCALED_EP,
                                                 self.scaled_ep_obj)

    def update_eps(self, eps):
        for ep in eps:
            self.eps[ep.name] = ep
            if ep.type in OBJ_DEFAULTS.droplet_eps:
                self._update_simple_ep(ep)
            if ep.type == OBJ_DEFAULTS.ep_type_scaled:
                self._update_scaled_ep(ep)
            if ep.type == OBJ_DEFAULTS.ep_type_gateway:
                self.update_gw_ep(ep)

    def update_gw_ep(self, ep):
        logger.info("Update gateway endpoint")
        ep.set_backends([self.ip])
        self.droplet_obj.update_ep(self.name, ep)

    def _update_simple_ep(self, ep):
        logger.info(
            "Update Bouncer with Simple EP:{}, type:{}".format(ep.name, ep.type))
        logger.info("self ip {} epfuncip {}, field ip {}".format(
            self.ip, ep.get_droplet_ip(), ep.droplet_obj.ip))
        self.droplet_obj.update_ep(self.name, ep)
        self.droplet_obj.update_substrate(ep)

    def _update_scaled_ep(self, ep):
        logger.info("Bouncer update scaled ep. {}".format(ep.backends))
        if ep.backends:
            self.droplet_obj.update_ep(self.name, ep)

    def update_vpc(self, dividers, add=True):
        for divider in dividers:
            if add:
                logger.info("Divider added: {}".format(divider.name))
                self.dividers[divider.name] = divider
                self.droplet_obj.update_substrate(divider)
            else:
                logger.info("Divider removed: {}".format(divider.name))
                self.dividers[divider.name] = divider
                self.droplet_obj.delete_substrate(divider)
        self.droplet_obj.update_vpc(self)

    def delete_vpc(self):
        for name in list(self.dividers.keys()):
            divider = self.dividers.pop(name)
            self.droplet_obj.delete_substrate(divider)
        self.droplet_obj.delete_vpc(self)

    def set_vpc(self, vpc):
        self.vpc = vpc

    def set_net(self, net):
        self.net = net

    def set_vni(self, vni):
        self.vni = vni

    def set_cidr(self, cidr):
        self.nip = str(cidr.ip)
        self.prefix = str(cidr.prefixlen)

    def set_droplet(self, droplet):
        self.droplet_obj = droplet
        self.droplet = droplet.name
        self.ip = droplet.ip
        self.mac = droplet.mac

    def delete_eps(self, eps):
        for ep in eps:
            if ep.name in self.eps.keys():
                self.eps.pop(ep.name)
                if ep.type == OBJ_DEFAULTS.ep_type_simple:
                    self._delete_ep(ep)
                if ep.type == OBJ_DEFAULTS.ep_type_scaled:
                    self._delete_scaled_ep(ep)

    def _delete_ep(self, ep):
        self.droplet_obj.delete_ep(self.name, ep)
        self.droplet_obj.delete_substrate(ep)
        ep.droplet_obj.delete_ep(ep.name, ep)

    def _delete_scaled_ep(self, ep):
        self.droplet_obj.delete_ep(self.name, ep)
