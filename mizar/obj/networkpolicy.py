# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Authors.

# Authors: Hong Chang   <@Hong-Chang>

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
import ipaddress
from mizar.common.common import *

logger = logging.getLogger()


class NetworkPolicy:
    def __init__(self, name, namespace, obj_api, opr_store, spec=None):
        self.name = name
        self.namespace = namespace
        self.obj_api = obj_api
        self.store = opr_store
        self.pod_label_dict = {}
        self.policy_types = []
        if spec is not None:
            self.set_obj_spec(spec)

    def set_obj_spec(self, spec):
        self.pod_label_dict = spec["podSelector"]["matchLabels"] if "matchLabels" in spec["podSelector"] else {}
        self.policy_types = spec["policyTypes"]

    @property
    def get_pod_label_dict(self):
        return self.pod_label_dict

    def get_policy_types(self):
        return self.policy_types

    def get_name(self):
        return self.name

    def get_namespace(self):
        return self.namespace

    def get_plural(self):
        return "networkpolicies"

    def get_kind(self):
        return "NetworkPolicy"

    def store_update_obj(self):
        if self.store is None:
            return
        self.store.update_networkpolicy(self)

    def store_delete_obj(self):
        if self.store is None:
            return
        self.store.delete_networkpolicy(self.name)
