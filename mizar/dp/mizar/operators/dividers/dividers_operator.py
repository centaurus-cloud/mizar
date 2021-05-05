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
from kubernetes import client, config
from mizar.common.constants import *
from mizar.common.common import *
from mizar.obj.bouncer import Bouncer
from mizar.obj.divider import Divider
from mizar.store.operator_store import OprStore
import logging

logger = logging.getLogger()


class DividerOperator(object):
    _instance = None

    def __new__(cls, **kwargs):
        if cls._instance is None:
            cls._instance = super(DividerOperator, cls).__new__(cls)
            cls._init(cls, **kwargs)
        return cls._instance

    def _init(self, **kwargs):
        logger.info(kwargs)
        self.store = OprStore()
        config.load_incluster_config()
        self.obj_api = client.CustomObjectsApi()

    def query_existing_dividers(self):
        logger.info("divider on_startup")

        def list_dividers_obj_fn(name, spec, plurals):
            logger.info("Bootstrapped Divider {}".format(name))
            d = Divider(name, self.obj_api, self.store, spec)
            if d.status == OBJ_STATUS.divider_status_provisioned:
                self.store_update(d)

        kube_list_obj(self.obj_api, RESOURCES.dividers, list_dividers_obj_fn)

    def get_divider_tmp_obj(self, name, spec):
        return Divider(name, self.obj_api, None, spec)

    def get_divider_stored_obj(self, name, spec):
        return Divider(name, self.obj_api, self.store, spec)

    def store_update(self, divider):
        self.store.update_divider(divider)

    def set_divider_provisioned(self, div):
        div.set_status(OBJ_STATUS.divider_status_provisioned)
        div.update_obj()

    def update_divider_with_bouncers(self, bouncer, net):
        dividers = self.store.get_dividers_of_vpc(bouncer.vpc).values()
        for d in dividers:
            d.update_net(net)

    def delete_bouncer_from_dividers(self, bouncer, net):
        dividers = self.store.get_dividers_of_vpc(bouncer.vpc).values()
        for d in dividers:
            d.update_net(net, False)

    def update_net(self, net, dividers=None):
        if not dividers:
            dividers = self.store.get_dividers_of_vpc(net.vpc).values()
        for d in dividers:
            d.update_net(net)

    def delete_net(self, net):
        dividers = self.store.get_dividers_of_vpc(net.vpc).values()
        for d in dividers:
            d.delete_net(net)

    def delete_nets_from_divider(self, nets, divider):
        for net in nets:
            divider.delete_net(net)

    def update_vpc(self, bouncer):
        dividers = self.store.get_dividers_of_vpc(bouncer.vpc).values()
        bouncer.update_vpc(dividers)
