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

import random
import uuid
import logging
from kubernetes import client, config
from mizar.common.constants import *
from mizar.common.common import *
from mizar.common.cidr import Cidr
from mizar.obj.vpc import Vpc
from mizar.obj.net import Net
from mizar.obj.divider import Divider
from mizar.store.operator_store import OprStore
logger = logging.getLogger()


class VpcOperator(object):
    _instance = None

    def __new__(cls, **kwargs):
        if cls._instance is None:
            cls._instance = super(VpcOperator, cls).__new__(cls)
            cls._init(cls, **kwargs)
        return cls._instance

    def _init(self, **kwargs):
        logger.info(kwargs)
        self.store = OprStore()
        config.load_incluster_config()
        self.obj_api = client.CustomObjectsApi()

    def query_existing_vpcs(self):
        def list_vpc_obj_fn(name, spec, plurals):
            logger.info("Bootstrapped {}".format(name))
            v = self.get_vpc_stored_obj(name, spec)
            if v.status == OBJ_STATUS.vpc_status_provisioned:
                self.store_update(v)

        kube_list_obj(self.obj_api, RESOURCES.vpcs, list_vpc_obj_fn)
        logger.debug("Bootstrap VPC store: {}".format(self.store._dump_vpcs()))

    def get_vpc_tmp_obj(self, name, spec):
        return Vpc(name, self.obj_api, None, spec)

    def get_vpc_stored_obj(self, name, spec):
        return Vpc(name, self.obj_api, self.store, spec)

    def create_default_vpc(self):
        if self.store.get_vpc(OBJ_DEFAULTS.default_ep_vpc):
            return
        v = Vpc(OBJ_DEFAULTS.default_ep_vpc, self.obj_api, self.store)
        v.create_obj()

    def store_update(self, vpc):
        self.store.update_vpc(vpc)

    def store_get(self, vpc):
        return self.store.get_vpc(vpc)

    def create_vpc_dividers(self, vpc, n):
        logger.info("Create {} dividers for vpc: {}".format(n, vpc.name))
        for i in range(n):
            vpc.create_divider()

    def delete_vpc_dividers(self, vpc, n):
        logger.info("Delete {} dividers for vpc: {}".format(n, vpc.name))
        for i in range(n):
            vpc.delete_divider()

    def process_divider_change(self, vpc, old, new):
        diff = new - old
        if diff > 0:
            logger.info("Scaling out VPCs dividers: {}".format(diff))
            return self.create_vpc_dividers(vpc, abs(diff))

        if diff < 0:
            logger.info("Scaling in VPCs dividers: {}".format(diff))
            return self.delete_vpc_dividers(vpc, abs(diff))

    def set_vpc_provisioned(self, vpc):
        vpc.set_status(OBJ_STATUS.vpc_status_provisioned)
        vpc.update_obj()

    def on_divider_provisioned(self, body, spec, **kwargs):
        name = kwargs['name']
        self._on_divider_provisioned(name, spec)

    def _on_divider_provisioned(self, name, spec):
        logger.info(
            "on_divider_provisioned {} with spec: {}".format(name, spec))
        div = Divider(name, self.obj_api, None, spec)
        v = self.store.get_vpc(div.vpc)
        if v.status != OBJ_STATUS.vpc_status_provisioned:
            v.set_status(OBJ_STATUS.vpc_status_provisioned)
            v.update_obj()

    def on_vpc_delete(self, body, spec, **kwargs):
        logger.info("on_vpc_delete {}".format(spec))

    def allocate_vni(self, vpc):
        # Scrappy allocator for now!!
        # TODO: There is a tiny chance of collision here, not to worry about now
        if vpc.name == OBJ_DEFAULTS.default_ep_vpc:
            return OBJ_DEFAULTS.default_vpc_vni
        vpc.set_vni(str(uuid.uuid4().int & (1 << 24)-1))

    def deallocate_vni(self, vpc):
        # TODO: Keep track of VNI allocation
        pass
