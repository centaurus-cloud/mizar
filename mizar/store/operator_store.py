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
import inspect

logger = logging.getLogger()


class OprStore(object):
    _instance = None

    def __new__(cls, **kwargs):
        if cls._instance is None:
            cls._instance = super(OprStore, cls).__new__(cls)
            cls._init(cls, **kwargs)
        return cls._instance

    def _init(self, **kwargs):
        logger.info(kwargs)
        self.droplets_store = {}

        self.vpcs_store = {}
        self.arktosnet_vpc_store = {}
        self.nets_vpc_store = {}
        self.nets_store = {}

        self.eps_store = {}
        self.eps_net_store = {}
        self.eps_pod_store = {}

        self.networkpolicies_store = {}
        self.eps_store_to_be_updated_networkpolicy = set()
        self.pod_names_to_be_ignored_by_networkpolicy = set()
        self.networkpolicies_to_be_updated_store = {}

        # label of pods in networkpolicy.spec.podSelector
        self.label_networkpolicies_store = {}
        # label of pods in networkpolicy ingress rules
        self.label_networkpolicies_ingress_store = {}
        # label of pods in networkpolicy egress rules
        self.label_networkpolicies_egress_store = {}

        # label of namespaces in networkpolicy ingress rules
        self.namespace_label_networkpolicies_ingress_store = {}
        # label of namespaces in networkpolicy egress rules
        self.namespace_label_networkpolicies_egress_store = {}

        self.networkpolicy_endpoints_ingress_store = {}
        self.networkpolicy_endpoints_egress_store = {}

        self.pod_label_value_store = {
            "": 0
        }
        self.namespace_label_value_store = {
            "": 0
        }

        self.dividers_store = {}
        self.dividers_vpc_store = {}

        self.bouncers_store = {}
        self.bouncers_net_store = {}
        self.bouncers_vpc_store = {}

    def update_vpc(self, vpc):
        self.vpcs_store[vpc.name] = vpc

    def delete_vpc(self, name):
        if name in self.vpcs_store:
            del self.vpcs_store[name]

    def get_vpc(self, name):
        if name in self.vpcs_store:
            return self.vpcs_store[name]
        return None

    def update_arktosnet_vpc(self, a, v):
        if v in self.vpcs_store:
            self.arktosnet_vpc_store[a] = v

    def get_vpc_in_arktosnet(self, name):
        if name in self.arktosnet_vpc_store:
            return self.arktosnet_vpc_store[name]
        return None

    def contains_vpc(self, name):
        if name in self.vpcs_store:
            return True
        return False

    def _dump_vpcs(self):
        for v in self.vpcs_store.values():
            logger.info("VPC: {}, Spec: {}".format(v.name, v.get_obj_spec()))

    def update_net(self, net):
        self.nets_store[net.name] = net

        if net.vpc not in self.nets_vpc_store:
            self.nets_vpc_store[net.vpc] = {}
        self.nets_vpc_store[net.vpc][net.name] = net

    def delete_net(self, name):
        if name not in self.nets_store:
            return
        net = self.nets_store.pop(name)

        if name not in self.nets_vpc_store[net.vpc]:
            return net
        self.nets_vpc_store[net.vpc].pop(name)
        l = len(self.nets_vpc_store[net.vpc])
        if l == 0:
            del self.nets_vpc_store[net.vpc]
        return net

    def get_net(self, name):
        if name in self.nets_store:
            return self.nets_store[name]
        return None

    def get_nets_in_vpc(self, vpc):
        if vpc in self.nets_vpc_store:
            return self.nets_vpc_store[vpc]
        return {}

    def contains_net(self, name):
        if name in self.nets_store:
            return True
        return False

    def _dump_nets(self):
        for n in self.nets_store.values():
            logger.info("Net: {}, Spec: {}".format(n.name, n.get_obj_spec()))

    def update_ep(self, ep):
        logger.info("Store update ep {}".format(ep.name))

        old_ep = self.get_ep(ep.name)
        if old_ep is not None:
            if len(old_ep.ingress_networkpolicies) > 0 and len(ep.ingress_networkpolicies) == 0:
                ep.ingress_networkpolicies = old_ep.ingress_networkpolicies
            if len(old_ep.egress_networkpolicies) > 0 and len(ep.egress_networkpolicies) == 0:
                ep.egress_networkpolicies = old_ep.egress_networkpolicies
            if len(old_ep.data_for_networkpolicy) > 0 and len(ep.data_for_networkpolicy) == 0:
                ep.data_for_networkpolicy = old_ep.data_for_networkpolicy

        # logger.info('caller name:{}'.format(inspect.stack()[1][3]))
        self.eps_store[ep.name] = ep
        if ep.net not in self.eps_net_store:
            self.eps_net_store[ep.net] = {}
        self.eps_net_store[ep.net][ep.name] = ep
        if ep.pod not in self.eps_pod_store:
            self.eps_pod_store[ep.pod] = {}
        self.eps_pod_store[ep.pod][ep.name] = ep

    def delete_ep(self, name):
        if name not in self.eps_store:
            return
        ep = self.eps_store.pop(name)

        if name not in self.eps_net_store[ep.net]:
            return ep
        self.eps_net_store[ep.net].pop(name)
        l = len(self.eps_net_store[ep.net])
        if l == 0:
            del self.eps_net_store[ep.net]
        if name not in self.eps_pod_store[ep.pod]:
            return ep
        self.eps_pod_store[ep.pod].pop(name)
        l = len(self.eps_pod_store[ep.pod])
        if l == 0:
            del self.eps_pod_store[ep.pod]
        return ep

    def get_ep(self, name):
        if name in self.eps_store:
            return self.eps_store[name]
        return None

    def get_eps_in_net(self, net):
        if net in self.eps_net_store:
            return self.eps_net_store[net]
        return {}

    def get_eps_in_pod(self, pod):
        if pod in self.eps_pod_store:
            return self.eps_pod_store[pod]
        return {}

    def contains_ep(self, name):
        if name in self.eps_store:
            return True
        return False

    def _dump_eps(self):
        for e in self.eps_store.values():
            logger.debug("EP: {}, Spec: {}".format(e.name, e.get_obj_spec()))

    def get_networkpolicy(self, name):
        if name in self.networkpolicies_store:
            return self.networkpolicies_store[name]
        return None

    def update_networkpolicy(self, networkpolicy):
        logger.info("Store update networkpolicy {}".format(networkpolicy.name))
        self.networkpolicies_store[networkpolicy.name] = networkpolicy

    def delete_networkpolicy(self, name):
        if name in self.networkpolicies_store:
            del self.networkpolicies_store[name]
        self.delete_networkpolicy_in_label_policy_store(self.label_networkpolicies_store, name)
        self.delete_networkpolicy_in_label_policy_store(self.label_networkpolicies_ingress_store, name)
        self.delete_networkpolicy_in_label_policy_store(self.label_networkpolicies_egress_store, name)
        self.delete_networkpolicy_in_label_policy_store(self.namespace_label_networkpolicies_ingress_store, name)
        self.delete_networkpolicy_in_label_policy_store(self.namespace_label_networkpolicies_egress_store, name)

    def delete_networkpolicy_in_label_policy_store(self, label_policy_store, policy_name):
        label_list = set()
        for label in label_policy_store:
            if policy_name in label_policy_store[label]:
                label_policy_store[label].remove(policy_name)
                if len(label_policy_store[label]) == 0:
                    label_list.add(label)

        for label in label_list:
            label_policy_store.pop(label)

    def get_networkpolicies_to_be_updated_by_pod(self, pod_name):
        if pod_name in self.networkpolicies_to_be_updated_store:
            return self.networkpolicies_to_be_updated_store[pod_name]
        return None

    def add_networkpolicies_to_be_updated(self, pod_name, policy_name):
        if pod_name not in self.networkpolicies_to_be_updated_store:
            self.networkpolicies_to_be_updated_store[pod_name] = set()
        self.networkpolicies_to_be_updated_store[pod_name].add(policy_name)

    def get_networkpolicies_by_label(self, label):
        if label in self.label_networkpolicies_store:
            return self.label_networkpolicies_store[label]
        return None

    def add_label_networkpolicy(self, label, policy_name):
        if label not in self.label_networkpolicies_store:
            self.label_networkpolicies_store[label] = set()
        self.label_networkpolicies_store[label].add(policy_name)

    def get_networkpolicies_by_label_ingress(self, label):
        if label in self.label_networkpolicies_ingress_store:
            return self.label_networkpolicies_ingress_store[label]
        return None

    def add_label_networkpolicy_ingress(self, label, policy_name_list):
        if label not in self.label_networkpolicies_ingress_store:
            self.label_networkpolicies_ingress_store[label] = set()
        for policy_name in policy_name_list:
            self.label_networkpolicies_ingress_store[label].add(policy_name)

    def get_networkpolicies_by_label_egress(self, label):
        if label in self.label_networkpolicies_egress_store:
            return self.label_networkpolicies_egress_store[label]
        return None

    def add_label_networkpolicy_egress(self, label, policy_name_list):
        if label not in self.label_networkpolicies_egress_store:
            self.label_networkpolicies_egress_store[label] = set()
        for policy_name in policy_name_list:
            self.label_networkpolicies_egress_store[label].add(policy_name)

    def get_networkpolicies_by_namespace_label_ingress(self, label):
        if label in self.namespace_label_networkpolicies_ingress_store:
            return self.namespace_label_networkpolicies_ingress_store[label]
        return None

    def add_namespace_label_networkpolicy_ingress(self, label, policy_name_list):
        if label not in self.namespace_label_networkpolicies_ingress_store:
            self.namespace_label_networkpolicies_ingress_store[label] = set()
        for policy_name in policy_name_list:
            self.namespace_label_networkpolicies_ingress_store[label].add(policy_name)

    def get_networkpolicies_by_namespace_label_egress(self, label):
        if label in self.namespace_label_networkpolicies_egress_store:
            return self.namespace_label_networkpolicies_egress_store[label]
        return None

    def add_namespace_label_networkpolicy_egress(self, label, policy_name_list):
        if label not in self.namespace_label_networkpolicies_egress_store:
            self.namespace_label_networkpolicies_egress_store[label] = set()
        for policy_name in policy_name_list:
            self.namespace_label_networkpolicies_egress_store[label].add(policy_name)

    def get_endpoints_by_networkpolicy_ingress(self, policy_name):
        if policy_name in self.networkpolicy_endpoints_ingress_store:
            return self.networkpolicy_endpoints_ingress_store[policy_name]
        return None

    def add_networkpolicy_endpoint_ingress(self, policy_name, endpoint_name):
        if policy_name not in self.networkpolicy_endpoints_ingress_store:
            self.networkpolicy_endpoints_ingress_store[policy_name] = set()
        self.networkpolicy_endpoints_ingress_store[policy_name].add(endpoint_name)

    def get_endpoints_by_networkpolicy_egress(self, policy_name):
        if policy_name in self.networkpolicy_endpoints_egress_store:
            return self.networkpolicy_endpoints_egress_store[policy_name]
        return None

    def add_networkpolicy_endpoint_egress(self, policy_name, endpoint_name):
        if policy_name not in self.networkpolicy_endpoints_egress_store:
            self.networkpolicy_endpoints_egress_store[policy_name] = set()
        self.networkpolicy_endpoints_egress_store[policy_name].add(endpoint_name)

    def get_or_add_pod_label_value(self, label_combination):
        if label_combination not in self.pod_label_value_store:
            self.pod_label_value_store[label_combination] = len(self.pod_label_value_store)
        return self.pod_label_value_store[label_combination]

    def get_or_add_namespace_label_value(self, label_combination):
        if label_combination not in self.namespace_label_value_store:
            self.namespace_label_value_store[label_combination] = len(self.namespace_label_value_store)
        return self.namespace_label_value_store[label_combination]
    
    def update_droplet(self, droplet):
        self.droplets_store[droplet.name] = droplet

    def delete_droplet(self, name):
        if name in self.droplets_store:
            del self.droplets_store[name]

    def get_droplet(self, name):
        if name in self.droplets_store:
            return self.droplets_store[name]
        return None

    def get_droplet_by_ip(self, ip):
        for d in self.droplets_store:
            if self.droplets_store[d].ip == ip:
                return self.droplets_store[d]
        return None

    def get_all_droplets(self):
        return self.droplets_store.values()

    def contains_droplet(self, name):
        if name in self.droplets_store:
            return True
        return False

    def _dump_droplets(self):
        for d in self.droplets_store.values():
            logger.info("Droplets: {}, Spec: {}".format(
                d.name, d.get_obj_spec()))

    def update_divider(self, div):
        self.dividers_store[div.name] = div
        if div.vpc not in self.dividers_vpc_store:
            self.dividers_vpc_store[div.vpc] = {}
        self.dividers_vpc_store[div.vpc][div.name] = div

    def delete_divider(self, name):
        if name not in self.dividers_store:
            return
        d = self.dividers_store.pop(name)

        if d.vpc not in self.dividers_vpc_store:
            return
        if name not in self.dividers_vpc_store[d.vpc]:
            return
        self.dividers_vpc_store[d.vpc].pop(name)

        l = len(self.dividers_vpc_store[d.vpc])
        if l == 0:
            del self.dividers_vpc_store[d.vpc]

    def get_divider(self, name):
        if name in self.dividers_store:
            return self.dividers_store[name]
        return None

    def get_dividers_of_vpc(self, vpc):
        if vpc in self.dividers_vpc_store:
            return self.dividers_vpc_store[vpc]
        return {}

    def contains_divider(self, name):
        if name in self.dividers_store:
            return True
        return False

    def _dump_dividers(self):
        for d in self.dividers_store.values():
            logger.info("EP: {}, Spec: {}".format(d.name, d.get_obj_spec()))

    def update_bouncer(self, b):
        self.bouncers_store[b.name] = b

        if b.net not in self.bouncers_net_store:
            self.bouncers_net_store[b.net] = {}
        self.bouncers_net_store[b.net][b.name] = b

        if b.vpc not in self.bouncers_vpc_store:
            self.bouncers_vpc_store[b.vpc] = {}
        self.bouncers_vpc_store[b.vpc][b.name] = b

    def delete_bouncer(self, name):
        if name not in self.bouncers_store:
            return
        b = self.bouncers_store.pop(name)

        self.bouncers_net_store[b.net].pop(name)
        self.bouncers_vpc_store[b.vpc].pop(name)

        l = len(self.bouncers_net_store[b.net])
        if l == 0:
            del self.bouncers_net_store[b.net]

        l = len(self.bouncers_vpc_store[b.vpc])
        if l == 0:
            del self.bouncers_vpc_store[b.vpc]

    def get_bouncer(self, name):
        if name in self.bouncers_store:
            return self.bouncers_store[name]
        return None

    def get_bouncers_of_net(self, net):
        if net in self.bouncers_net_store:
            return self.bouncers_net_store[net]
        return {}

    def update_bouncers_of_net(self, net, bouncers):
        if net in self.bouncers_net_store:
            self.bouncers_net_store[net] = bouncers

    def get_bouncers_of_vpc(self, vpc):
        if vpc in self.bouncers_vpc_store:
            return self.bouncers_vpc_store[vpc]
        return {}

    def contains_bouncer(self, name):
        if name in self.bouncers_store:
            return True
        return False

    def _dump_bouncers(self):
        for b in self.bouncers_store.values():
            logger.info("Bouncer: {}, Spec: {}".format(
                b.name, b.get_obj_spec()))
