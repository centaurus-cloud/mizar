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
import json
from kubernetes import client, config
from mizar.obj.endpoint import Endpoint
from mizar.obj.bouncer import Bouncer
from mizar.obj.networkpolicy import NetworkPolicy
from mizar.common.constants import *
from mizar.common.common import *
from mizar.store.operator_store import OprStore
from mizar.proto.interface_pb2 import *
from mizar.daemon.interface_service import InterfaceServiceClient
import uuid

logger = logging.getLogger()


class NetworkPolicyOperator(object):
    _instance = None

    def __new__(cls, **kwargs):
        if cls._instance is None:
            cls._instance = super(NetworkPolicyOperator, cls).__new__(cls)
            cls._init(cls, **kwargs)
        return cls._instance

    def _init(self, **kwargs):
        logger.info(kwargs)
        self.store = OprStore()
        config.load_incluster_config()
        self.obj_api = client.CustomObjectsApi()
        self.core_api = client.CoreV1Api()

    def get_networkpolicy_from_cluster(self, name):
        splitted = name.split(":")
        return self.obj_api.get_namespaced_custom_object("networking.k8s.io", "v1", splitted[0], "networkpolicies", splitted[1])

    def update_networkpolicy_to_store(self, name, namespace, spec):
        networkpolicy = NetworkPolicy(name, namespace, self.obj_api, self.store, spec)
        self.store.update_networkpolicy(networkpolicy)
