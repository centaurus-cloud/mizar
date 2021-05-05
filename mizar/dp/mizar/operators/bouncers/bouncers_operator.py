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
import logging
from kubernetes import client, config
from mizar.obj.bouncer import Bouncer
from mizar.obj.divider import Divider
from mizar.obj.endpoint import Endpoint
from mizar.common.constants import *
from mizar.common.common import *
from mizar.obj.net import Net
from mizar.store.operator_store import OprStore

logger = logging.getLogger()


class BouncerOperator(object):
    _instance = None

    def __new__(cls, **kwargs):
        if cls._instance is None:
            cls._instance = super(BouncerOperator, cls).__new__(cls)
            cls._init(cls, **kwargs)
        return cls._instance

    def _init(self, **kwargs):
        logger.info(kwargs)
        self.store = OprStore()
        config.load_incluster_config()
        self.obj_api = client.CustomObjectsApi()

    def query_existing_bouncers(self):
        logger.info("bouncer on_startup")

        def list_bouncers_obj_fn(name, spec, plurals):
            logger.info("Bootstrapped Bouncer {}".format(name))
            b = Bouncer(name, self.obj_api, self.store, spec)
            self.store_update(b)

        kube_list_obj(self.obj_api, RESOURCES.droplets, list_bouncers_obj_fn)

    def get_bouncer_tmp_obj(self, name, spec):
        return Bouncer(name, self.obj_api, None, spec)

    def get_bouncer_stored_obj(self, name, spec):
        return Bouncer(name, self.obj_api, self.store, spec)

    def store_update(self, b):
        self.store.update_bouncer(b)

    def set_bouncer_provisioned(self, bouncer):
        bouncer.set_status(OBJ_STATUS.bouncer_status_provisioned)
        bouncer.update_obj()

    def update_bouncers_with_divider(self, div):
        bouncers = self.store.get_bouncers_of_vpc(div.vpc)
        for b in bouncers.values():
            b.update_vpc(set([div]))

    def delete_divider_from_bouncers(self, div):
        bouncers = self.store.get_bouncers_of_vpc(div.vpc)
        for b in bouncers.values():
            b.update_vpc(set([div]), False)

    def update_endpoint_with_bouncers(self, ep):
        bouncers = self.store.get_bouncers_of_net(ep.net)
        eps = set([ep])
        for key in bouncers:
            bouncers[key].update_eps(eps)

        if ep.type == OBJ_DEFAULTS.ep_type_simple or ep.type == OBJ_DEFAULTS.ep_type_host:
            ep.update_bouncers_list(bouncers)

    def delete_endpoint_from_bouncers(self, ep):
        bouncers = self.store.get_bouncers_of_net(ep.net)
        eps = set([ep])
        for key in bouncers:
            bouncers[key].delete_eps(eps)
        self.store.update_bouncers_of_net(ep.net, bouncers)
        if ep.type == OBJ_DEFAULTS.ep_type_simple:
            ep.unload_transit_agent_xdp()

    def delete_vpc(self, bouncer):
        bouncer.delete_vpc()
