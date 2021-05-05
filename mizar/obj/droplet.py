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
import json
from mizar.common.rpc import TrnRpc
from mizar.common.constants import *
from mizar.common.common import *
from mizar.common.common import run_cmd
from mizar.get_interface import get_itf

net_itf = get_itf()
logger = logging.getLogger()


class Droplet(object):

    def __init__(self, name, obj_api, opr_store, spec=None):
        self.name = name
        self.obj_api = obj_api
        self.store = opr_store
        self.ip = ""
        self.mac = ""
        self.phy_itf = net_itf
        self.status = OBJ_STATUS.droplet_status_init
        self.known_substrates = {}
        self.known_bouncers = {}
        self.known_nets = {}
        self.known_eps = {}
        if spec is not None:
            self.set_obj_spec(spec)

    @property
    def rpc(self):
        return TrnRpc(self.ip, self.mac)

    def get_obj_spec(self):
        self.obj = {
            "mac": self.mac,
            "ip": self.ip,
            "status": self.status,
            "itf": self.phy_itf
        }

        return self.obj

    def set_obj_spec(self, spec):
        self.status = get_spec_val('status', spec)
        self.mac = get_spec_val('mac', spec)
        self.ip = get_spec_val('ip', spec)
        self.phy_itf = get_spec_val('itf', spec)

    # K8s APIs
    def get_name(self):
        return self.name

    def get_plural(self):
        return "droplets"

    def get_kind(self):
        return "Droplet"

    def store_update_obj(self):
        if self.store is None:
            return
        self.store.update_droplet(self)

    def store_delete_obj(self):
        if self.store is None:
            return
        self.store.delete_droplet(self.name)

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

    def update_substrate(self, obj):
        if obj.name not in self.known_substrates.keys():
            logger.info("DROPLET_SUBSTRATE: Updated")
            self.known_substrates[obj.name] = obj.droplet_obj.ip
        self.rpc.update_substrate_ep(obj.droplet_obj.ip, obj.droplet_obj.mac)

    def delete_substrate(self, obj):
        if obj.name in self.known_substrates.keys():
            self.known_substrates.pop(obj.name)
            if obj.droplet_obj.ip not in self.known_substrates.values():
                logger.info("DROPLET_SUBSTRATE: Deleted")
                self.rpc.delete_substrate_ep(obj.droplet_obj.ip)

    def update_vpc(self, bouncer):
        if bouncer.name not in self.known_bouncers.keys():
            self.known_bouncers[bouncer.name] = bouncer.vpc
        self.rpc.update_vpc(bouncer)

    def delete_vpc(self, bouncer):
        if bouncer.name in self.known_bouncers.keys():
            self.known_bouncers.pop(bouncer.name)
            if bouncer.vpc not in self.known_bouncers.values():
                self.rpc.delete_vpc(bouncer)

    def update_net(self, net):
        nip = net.get_nip()
        if net.name not in self.known_nets.keys():
            self.known_nets[net.name] = nip
        self.rpc.update_net(net)

    def delete_net(self, net):
        nip = net.get_nip()
        if net.name in self.known_nets.keys():
            self.known_nets.pop(net.name)
            if nip not in self.known_nets.values():
                self.rpc.delete_net(net)

    def update_ep(self, name, ep):
        name = name + ep.name
        if name not in self.known_eps.keys():
            self.known_eps[name] = ep.ip
        self.rpc.update_ep(ep)

    def delete_ep(self, name, ep):
        name = name + ep.name
        if name in self.known_eps.keys():
            self.known_eps.pop(name)
            if ep.ip not in self.known_eps.values():
                self.rpc.delete_ep(ep)
