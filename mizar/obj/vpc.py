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
import random
import uuid
from mizar.common.rpc import TrnRpc
from mizar.common.constants import *
from mizar.common.common import *
from mizar.common.cidr import Cidr
from mizar.obj.divider import Divider
from kubernetes.client.rest import ApiException

logger = logging.getLogger()


class Vpc(object):
    def __init__(self, name, obj_api, opr_store, spec=None):
        self.name = name
        self.vni = OBJ_DEFAULTS.default_vpc_vni
        self.n_dividers = OBJ_DEFAULTS.default_n_dividers
        self.n_allocated_dividers = 0
        self.obj_api = obj_api
        self.status = OBJ_STATUS.vpc_status_init
        self.dividers = {}
        self.networks = {}
        self.store = opr_store
        self.ip = OBJ_DEFAULTS.default_vpc_ip
        self.prefix = OBJ_DEFAULTS.default_vpc_prefix
        if spec is not None:
            self.set_obj_spec(spec)
        if self.prefix == "":
            logger.info("Prefix is empty string!")
            self.prefix = OBJ_DEFAULTS.default_vpc_prefix
        self.cidr = Cidr(self.prefix, self.ip)
        self.ip = self.cidr.ip
        self.prefix = self.cidr.prefixlen

    def get_obj_spec(self):
        self.obj = {
            "ip": self.cidr.ip,
            "prefix": self.cidr.prefixlen,
            "vni": self.vni,
            "dividers": self.n_dividers,
            "status": self.status
        }

        return self.obj

    def set_obj_spec(self, spec):
        self.status = get_spec_val('status', spec, OBJ_STATUS.vpc_status_init)
        self.ip = get_spec_val('ip', spec, OBJ_DEFAULTS.default_vpc_ip)
        self.prefix = get_spec_val(
            "prefix", spec, OBJ_DEFAULTS.default_vpc_prefix)
        self.vni = get_spec_val('vni', spec, None)
        self.n_dividers = int(get_spec_val(
            'dividers', spec, OBJ_DEFAULTS.default_n_dividers))

    # K8s APIs
    def get_name(self):
        return self.name

    def get_plural(self):
        return "vpcs"

    def get_kind(self):
        return "Vpc"

    def store_update_obj(self):
        if self.store is None:
            return
        self.store.update_vpc(self)

    def store_delete_obj(self):
        if self.store is None:
            return
        self.store.delete_vpc(self.name)

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

    def create_divider(self):
        u = str(uuid.uuid4())
        divider_name = self.name + '-d-' + u
        logger.info("Create divider {} for vpc {}".format(
            divider_name, self.name))
        d = Divider(divider_name, self.obj_api, None)
        d.name = divider_name
        d.set_vpc(self.name)
        d.set_vni(self.vni)
        self.dividers[divider_name] = d
        d.create_obj()

    def delete_divider(self):
        logger.info("Delete divider for net {}".format(self.name))
        d = self.dividers.pop(random.choice(list(self.dividers.keys())))
        d.delete_obj()
