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
from mizar.common.cidr import Cidr
from mizar.common.constants import *
from mizar.common.common import *
from mizar.obj.net import Net
from mizar.obj.endpoint import Endpoint
from mizar.obj.bouncer import Bouncer
from mizar.store.operator_store import OprStore
logger = logging.getLogger()


class NetOperator(object):
    _instance = None

    def __new__(cls, **kwargs):
        if cls._instance is None:
            cls._instance = super(NetOperator, cls).__new__(cls)
            cls._init(cls, **kwargs)
        return cls._instance

    def _init(self, **kwargs):
        logger.info(kwargs)
        self.store = OprStore()
        config.load_incluster_config()
        self.obj_api = client.CustomObjectsApi()

    def query_existing_nets(self):
        def list_net_obj_fn(name, spec, plurals):
            logger.info("Bootstrapped {}".format(name))
            n = Net(name, self.obj_api, self.store, spec)
            self.store_update(n)

        kube_list_obj(self.obj_api, RESOURCES.nets, list_net_obj_fn)
        self.create_default_net()
        logger.debug("Bootstrap Net store: {}".format(self.store._dump_nets()))

    def get_net_tmp_obj(self, name, spec):
        return Net(name, self.obj_api, None, spec)

    def get_net_stored_obj(self, name, spec):
        return Net(name, self.obj_api, self.store, spec)

    def store_update(self, net):
        self.store.update_net(net)

    def create_default_net(self):
        if self.store.get_net(OBJ_DEFAULTS.default_ep_net):
            return
        n = Net(OBJ_DEFAULTS.default_ep_net, self.obj_api, self.store)
        n.create_obj()

    def set_net_provisioned(self, net):
        net.set_status(OBJ_STATUS.net_status_provisioned)
        net.update_obj()

    def on_net_init(self, body, spec, **kwargs):
        name = kwargs['name']
        logger.info("Net on_net_init {}".format(spec))
        n = Net(name, self.obj_api, self.store, spec)
        for i in range(n.n_bouncers):
            n.create_bouncer()
        n.set_status(OBJ_STATUS.net_status_allocated)
        n.update_obj()

    def create_net_bouncers(self, net, n):
        logger.info("Create {} Bouncers for net: {}".format(n, net.name))
        for i in range(n):
            net.create_bouncer()
        return net

    def delete_net_bouncers(self, net, n):
        logger.info("Deleting all Bouncers for net: {}".format(net.name))
        for i in range(n):
            net.delete_bouncer()

    def process_bouncer_change(self, net, old, new):
        diff = new - old
        if diff > 0:
            logger.info("Scaling out Nets bouncers: {}".format(diff))
            return self.create_net_bouncers(net, abs(diff))

        if diff < 0:
            logger.info("Scaling in Nets bouncers: {}".format(diff))
            pass

        return

    def allocate_endpoint(self, ep):
        n = self.store.get_net(ep.net)
        logger.info("IP {} for net {}".format(ep.ip, n.name))
        if ep.type == OBJ_DEFAULTS.ep_type_host:
            ip = ep.get_droplet_ip()
            ep.set_ip(ip)
        if ep.ip == "":
            ip = n.allocate_ip()
            ep.set_ip(ip)
        gw = n.get_gw_ip()
        if ep.get_prefix() == "":
            ep.set_prefix(n.get_prefixlen())
        ep.set_gw(gw)

        # TODO: Most of the time is spent in loading the transit agent
        # if ep.type == OBJ_DEFAULTS.ep_type_simple:
        #	ep.load_transit_agent()

        logger.info("After Allocate IP {} for net {}".format(ep.ip, n.name))
        n.mark_ip_as_allocated(ep.ip)
        self.store.update_net(n)
        ep.set_vni(n.vni)

    def deallocate_endpoint(self, ep):
        n = self.store.get_net(ep.net)
        n.deallocate_ip(ep.ip)
