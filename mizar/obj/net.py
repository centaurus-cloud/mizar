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

import uuid
import logging
import random
from mizar.common.rpc import TrnRpc
from mizar.common.constants import *
from mizar.common.common import *
from mizar.obj.bouncer import Bouncer
from mizar.common.cidr import Cidr
from kubernetes.client.rest import ApiException

logger = logging.getLogger()


class Net(object):
    def __init__(self, name, obj_api, opr_store, spec=None):
        self.name = name
        self.vpc = OBJ_DEFAULTS.default_ep_vpc
        self.vni = OBJ_DEFAULTS.default_vpc_vni
        self.n_bouncers = OBJ_DEFAULTS.default_n_bouncers
        self.n_allocated_bouncers = 0
        self.bouncers = {}
        self.endpoints = {}
        self.obj_api = obj_api
        self.store = opr_store
        self.status = OBJ_STATUS.net_status_init
        self.ip = OBJ_DEFAULTS.default_net_ip
        self.prefix = OBJ_DEFAULTS.default_net_prefix
        if spec is not None:
            self.set_obj_spec(spec)
        if self.prefix == "":
            logger.info("Prefix is empty string!")
            self.prefix = OBJ_DEFAULTS.default_net_prefix
        self.cidr = Cidr(self.prefix,
                         self.ip)
        self.ip = self.cidr.ip
        self.prefix = self.cidr.prefixlen
        self.gw = self.cidr.gw

    def get_obj_spec(self):
        self.obj = {
            "ip": self.cidr.ip,
            "prefix": self.cidr.prefixlen,
            "vni": self.vni,
            "vpc": self.vpc,
            "bouncers": self.n_bouncers,
            "status": self.status
        }

        return self.obj

    def set_obj_spec(self, spec):
        self.status = get_spec_val('type', spec)
        self.vpc = get_spec_val('vpc', spec)
        self.vni = get_spec_val('vni', spec)
        self.n_bouncers = int(get_spec_val(
            'bouncers', spec, OBJ_DEFAULTS.default_n_bouncers))
        self.ip = get_spec_val('ip', spec, OBJ_DEFAULTS.default_net_ip)
        self.prefix = get_spec_val(
            'prefix', spec, OBJ_DEFAULTS.default_net_prefix)

    # K8s APIs
    def get_name(self):
        return self.name

    def get_plural(self):
        return "subnets"

    def get_kind(self):
        return "Subnet"

    def store_update_obj(self):
        if self.store is None:
            return
        self.store.update_net(self)

    def store_delete_obj(self):
        if self.store is None:
            return
        self.store.delete_net(self.name)

    def create_obj(self):
        return kube_create_obj(self)

    def update_obj(self):
        return kube_update_obj(self)

    def delete_obj(self):
        return kube_delete_obj(self)

    def watch_obj(self, watch_callback):
        return kube_watch_obj(self, watch_callback)

    def set_vni(self, vni):
        self.vni = vni

    def set_status(self, status):
        self.status = status

    def get_gw_ip(self):
        return str(self.cidr.get_ip(1))

    def get_tunnel_id(self):
        return str(self.vni)

    def get_nip(self):
        return str(self.cidr.ip)

    def get_prefixlen(self):
        return str(self.cidr.prefixlen)

    def get_bouncers_ips(self):
        bouncer_ips = []
        for b in self.bouncers.values():
            if b.ip not in bouncer_ips:
                bouncer_ips.append(b.ip)
        return bouncer_ips

    def create_bouncer(self):
        u = str(uuid.uuid4())
        bouncer_name = self.name + '-b-' + u
        logger.info("Create bouncer {} for net {}".format(
            bouncer_name, self.name))
        b = Bouncer(bouncer_name, self.obj_api, None)
        b.set_vpc(self.vpc)
        b.set_cidr(self.cidr)
        b.set_vni(self.vni)
        b.set_net(self.name)
        b.create_obj()

    def delete_bouncer(self):
        b = self.bouncers.pop(random.choice(list(self.bouncers.keys())))
        logger.info("Deleted bouncer {} from net {}".format(b.name, self.name))
        b.delete_obj()

    def allocate_ip(self):
        return self.cidr.allocate_ip()

    def deallocate_ip(self, ip):
        return self.cidr.deallocate_ip(ip)

    def mark_ip_as_allocated(self, ip):
        self.cidr.mark_ip_as_allocated(ip)

    def update_gw_endpoint(self):
        pass

    def delete_gw_endpoint(self):
        pass

    def update_host_endpoint(self):
        pass

    def delete_host_endpoint(self):
        pass

    def update_scaled_endpoint(self):
        pass

    def delete_scaled_endpoint(self):
        pass
