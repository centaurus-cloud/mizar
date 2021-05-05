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

import copy
import logging
import time
from mizar.common.ipv4_trie import IPv4Trie
from mizar.dp.mizar.operators.endpoints.endpoints_operator import *
from mizar.dp.mizar.operators.networkpolicies.networkpolicies_operator import *

endpoint_opr = EndpointOperator()
networkpolicy_opr = NetworkPolicyOperator()

logger = logging.getLogger()


class NetworkPolicyUtil:
    def update_and_retrieve_endpoint_names(self, policy_name, policy_namespace, pod_label_dict, policy_types):
        self.update_pod_label_networkpolicy_mapping_in_store(policy_name, pod_label_dict)
        
        endpoint_name_list = self.retrieve_endpoints_for_networkpolicy(policy_name, policy_namespace, pod_label_dict)
        endpoint_names_to_be_handled = set()

        if policy_types is None:
            if policy_name in networkpolicy_opr.store.networkpolicy_endpoints_ingress_store:
                for endpoint_name in networkpolicy_opr.store.networkpolicy_endpoints_ingress_store[policy_name]:                    
                    ep = networkpolicy_opr.store.get_ep(endpoint_name)
                    if ep is None:
                        logger.warn("In operator store, found endpoint {} in networkpolicy_endpoints_ingress_store but cannot retrieve it from eps_store.".format(endpoint_name))
                    else:
                        ep.remove_ingress_networkpolicy(policy_name)
                    endpoint_names_to_be_handled.add(endpoint_name)
                networkpolicy_opr.store.networkpolicy_endpoints_ingress_store.pop(policy_name)
            if policy_name in networkpolicy_opr.store.networkpolicy_endpoints_egress_store:
                for endpoint_name in networkpolicy_opr.store.networkpolicy_endpoints_egress_store[policy_name]:
                    ep = networkpolicy_opr.store.get_ep(endpoint_name)
                    if ep is None:
                        logger.warn("In operator store, found endpoint {} in networkpolicy_endpoints_egress_store but cannot retrieve it from eps_store.".format(endpoint_name))
                    else:
                        ep.remove_egress_networkpolicy(policy_name)
                    endpoint_names_to_be_handled.add(endpoint_name)
                networkpolicy_opr.store.networkpolicy_endpoints_egress_store.pop(policy_name)
        else:
            if "Ingress" in policy_types:
                if policy_name not in networkpolicy_opr.store.networkpolicy_endpoints_ingress_store:
                    networkpolicy_opr.store.networkpolicy_endpoints_ingress_store[policy_name] = set()

                # Find newly added endpoint and add it into store.networkpolicy_endpoints_ingress_store
                for endpoint_name in endpoint_name_list:
                    endpoint_names_to_be_handled.add(endpoint_name) 
                    if endpoint_name not in networkpolicy_opr.store.networkpolicy_endpoints_ingress_store[policy_name]:                
                        networkpolicy_opr.store.add_networkpolicy_endpoint_ingress(policy_name, endpoint_name)                       

                # Find deleted endpoint and remove it from store.networkpolicy_endpoints_ingress_store
                endpoint_names_to_be_removed_ingress = set()
                for endpoint_name in networkpolicy_opr.store.networkpolicy_endpoints_ingress_store[policy_name]:
                    if endpoint_name not in endpoint_name_list:
                        endpoint_names_to_be_handled.add(endpoint_name)
                        endpoint_names_to_be_removed_ingress.add(endpoint_name)
                for endpoint_name in endpoint_names_to_be_removed_ingress:            
                    networkpolicy_opr.store.networkpolicy_endpoints_ingress_store[policy_name].remove(endpoint_name)
                    ep = networkpolicy_opr.store.get_ep(endpoint_name)
                    if ep is None:
                        logger.warn("In operator store, found endpoint {} in networkpolicy_endpoints_ingress_store but cannot retrieve it from eps_store.".format(endpoint_name))
                    else:
                        ep.remove_ingress_networkpolicy(policy_name)
                if len(networkpolicy_opr.store.networkpolicy_endpoints_ingress_store[policy_name]) == 0:
                    networkpolicy_opr.store.networkpolicy_endpoints_ingress_store.pop(policy_name)

            if "Egress" in policy_types:
                if policy_name not in networkpolicy_opr.store.networkpolicy_endpoints_egress_store:
                    networkpolicy_opr.store.networkpolicy_endpoints_egress_store[policy_name] = set()

                # Find newly added endpoint and add it into store.networkpolicy_endpoints_egress_store
                for endpoint_name in endpoint_name_list:
                    endpoint_names_to_be_handled.add(endpoint_name) 
                    if endpoint_name not in networkpolicy_opr.store.networkpolicy_endpoints_egress_store[policy_name]:                
                        networkpolicy_opr.store.add_networkpolicy_endpoint_egress(policy_name, endpoint_name)                       

                # Find deleted endpoint and remove it from store.networkpolicy_endpoints_egress_store
                endpoint_names_to_be_removed_egress = set()
                for endpoint_name in networkpolicy_opr.store.networkpolicy_endpoints_egress_store[policy_name]:
                    if endpoint_name not in endpoint_name_list:
                        endpoint_names_to_be_handled.add(endpoint_name)
                        endpoint_names_to_be_removed_egress.add(endpoint_name)
                for endpoint_name in endpoint_names_to_be_removed_egress:            
                    networkpolicy_opr.store.networkpolicy_endpoints_egress_store[policy_name].remove(endpoint_name)
                    ep = networkpolicy_opr.store.get_ep(endpoint_name)
                    if ep is None:
                        logger.warn("In operator store, found endpoint {} in networkpolicy_endpoints_egress_store but cannot retrieve it from eps_store.".format(endpoint_name))
                    else:
                        ep.remove_egress_networkpolicy(policy_name)
                if len(networkpolicy_opr.store.networkpolicy_endpoints_egress_store[policy_name]) == 0:
                    networkpolicy_opr.store.networkpolicy_endpoints_egress_store.pop(policy_name)

        self.set_networkpolicy_for_endpoint(policy_name, policy_types)

        return endpoint_names_to_be_handled

    def update_pod_label_networkpolicy_mapping_in_store(self, policy_name, pod_label_dict):
        if pod_label_dict is None:
            return
            
        for key in pod_label_dict:
            label = "{}={}".format(key, pod_label_dict[key])
            networkpolicy_opr.store.add_label_networkpolicy(label, policy_name)

    def retrieve_endpoints_for_networkpolicy(self, policy_name, policy_namespace, pod_label_dict):
        endpoint_name_list = set()

        if pod_label_dict is None:
            return endpoint_name_list

        pods = kube_list_pods_by_labels(networkpolicy_opr.core_api, pod_label_dict, policy_namespace)
        if pods is None:
            return endpoint_name_list
        
        for pod in pods.items:
            pod_name = pod.metadata.name
            eps = endpoint_opr.store.get_eps_in_pod(pod_name)
            for ep in eps.values():
                endpoint_name_list.add(ep.name)

        return endpoint_name_list

    def set_networkpolicy_for_endpoint(self, policy_name, policy_types):
        if policy_types is not None and "Ingress" in policy_types:
            if policy_name in networkpolicy_opr.store.networkpolicy_endpoints_ingress_store:
                for endpoint_name in networkpolicy_opr.store.networkpolicy_endpoints_ingress_store[policy_name]:
                    ep = networkpolicy_opr.store.get_ep(endpoint_name)
                    if ep is None:
                        logger.warn("In operator store, found endpoint {} in networkpolicy_endpoints_ingress_store but cannot retrieve it from eps_store.".format(endpoint_name))
                    else:
                        ep.add_ingress_networkpolicy(policy_name)
        else:
            self.remove_networkpolicy_from_endpoint_ingress(policy_name)
            
        if policy_types is not None and "Egress" in policy_types:
            if policy_name in networkpolicy_opr.store.networkpolicy_endpoints_egress_store:
                for endpoint_name in networkpolicy_opr.store.networkpolicy_endpoints_egress_store[policy_name]:
                    ep = networkpolicy_opr.store.get_ep(endpoint_name)
                    if ep is None:
                        logger.warn("In operator store, found endpoint {} in networkpolicy_endpoints_egress_store but cannot retrieve it from eps_store.".format(endpoint_name))
                    else:
                        ep.add_egress_networkpolicy(policy_name)
        else:
            self.remove_networkpolicy_from_endpoint_egress(policy_name)

    def remove_networkpolicy_from_endpoint_ingress(self, policy_name):
        if policy_name in networkpolicy_opr.store.networkpolicy_endpoints_ingress_store:
            for endpoint_name in networkpolicy_opr.store.networkpolicy_endpoints_ingress_store[policy_name]:
                ep = networkpolicy_opr.store.get_ep(endpoint_name)
                if ep is None:
                    logger.warn("In operator store, found endpoint {} in networkpolicy_endpoints_ingress_store but cannot retrieve it from eps_store.".format(endpoint_name))
                else:
                    ep.remove_ingress_networkpolicy(policy_name)

    def remove_networkpolicy_from_endpoint_egress(self, policy_name):
        if policy_name in networkpolicy_opr.store.networkpolicy_endpoints_egress_store:
            for endpoint_name in networkpolicy_opr.store.networkpolicy_endpoints_egress_store[policy_name]:
                ep = networkpolicy_opr.store.get_ep(endpoint_name)
                if ep is None:
                    logger.warn("In operator store, found endpoint {} in networkpolicy_endpoints_egress_store but cannot retrieve it from eps_store.".format(endpoint_name))
                else:
                    ep.remove_egress_networkpolicy(policy_name)

    def handle_networkpolicy_update_delete(self, endpoint_name_list):
        for endpoint_name in endpoint_name_list:
            ep = networkpolicy_opr.store.get_ep(endpoint_name)
            if ep is None:
                logger.warn("In operator store, found endpoint {} to be update network policy data but cannot retrieve it from eps_store.".format(endpoint_name))
            else:
                self.handle_endpoint_for_networkpolicy(ep)

    def handle_endpoint_for_networkpolicy(self, ep):
        data_for_networkpolicy_ingress = self.generate_data_for_networkpolicy_ingress(ep)
        data_for_networkpolicy_egress = self.generate_data_for_networkpolicy_egress(ep)
        data_for_networkpolicy = {
            "old": {},
            "ingress": data_for_networkpolicy_ingress,
            "egress": data_for_networkpolicy_egress,
        }
        
        old_data_for_networkpolicy = ep.get_data_for_networkpolicy()
        if len(old_data_for_networkpolicy) > 0:
            if old_data_for_networkpolicy["ingress"] == data_for_networkpolicy_ingress and old_data_for_networkpolicy["egress"] == data_for_networkpolicy_egress:
                return

            old_data_for_networkpolicy["old"] = {}
            data_for_networkpolicy["old"] = old_data_for_networkpolicy

        logger.info("ep: {}, data_for_networkpolicy: {}".format(ep.name, data_for_networkpolicy))
        ep.set_data_for_networkpolicy(data_for_networkpolicy)

        copied_data = copy.deepcopy(data_for_networkpolicy)
        self.remove_dup_data_from_old(copied_data)
        ep.update_networkpolicy_per_endpoint(copied_data)

        for label in data_for_networkpolicy["ingress"]["label_networkpolicies_map"]:
            networkpolicy_opr.store.add_label_networkpolicy_ingress(label, data_for_networkpolicy["ingress"]["label_networkpolicies_map"][label])
        for label in data_for_networkpolicy["egress"]["label_networkpolicies_map"]:
            networkpolicy_opr.store.add_label_networkpolicy_egress(label, data_for_networkpolicy["egress"]["label_networkpolicies_map"][label])
        for label in data_for_networkpolicy["ingress"]["namespace_label_networkpolicies_map"]:
            networkpolicy_opr.store.add_namespace_label_networkpolicy_ingress(label, data_for_networkpolicy["ingress"]["namespace_label_networkpolicies_map"][label])
        for label in data_for_networkpolicy["egress"]["namespace_label_networkpolicies_map"]:
            networkpolicy_opr.store.add_namespace_label_networkpolicy_egress(label, data_for_networkpolicy["egress"]["namespace_label_networkpolicies_map"][label])

    def remove_dup_data_from_old(self, data):
        if "old" not in data:
            return

        old_data = data["old"]
        if "ingress" in data and len(data["ingress"]) > 0 and "ingress" in old_data and len(old_data["ingress"]) > 0:
            self.remove_dup_directional_data_from_old(data["ingress"], old_data["ingress"])
        if "egress" in data and len(data["egress"]) > 0 and "egress" in old_data and len(old_data["egress"]) > 0:
            self.remove_dup_directional_data_from_old(data["egress"], old_data["egress"])

    def remove_dup_directional_data_from_old(self, data, old_data):
        self.remove_items_from_old(data["cidr_table_no_except"], old_data["cidr_table_no_except"])
        self.remove_items_from_old(data["cidr_table_with_except"], old_data["cidr_table_with_except"])
        self.remove_items_from_old(data["cidr_table_except"], old_data["cidr_table_except"])
        self.remove_items_from_old(data["port_table"], old_data["port_table"])

    def remove_items_from_old(self, items, old_items):
        to_be_removed = []
        for item in items:
            if item in old_items:
                to_be_removed.append(item)
        for item in to_be_removed:
            items.remove(item)
            old_items.remove(item)

    def generate_data_for_networkpolicy_ingress(self, ep):
        data = self.init_data_for_networkpolicy()
        direction = "ingress"

        if len(ep.ingress_networkpolicies) == 0:
            return data

        for networkpolicy_name in ep.ingress_networkpolicies:
            networkpolicy_spec = networkpolicy_opr.get_networkpolicy_from_cluster(networkpolicy_name)
            self.fill_data_from_directional_traffic_rules(data, direction, networkpolicy_spec)
        self.build_access_rules(data, ep)
        return data

    def generate_data_for_networkpolicy_egress(self, ep):
        data = self.init_data_for_networkpolicy()
        direction = "egress"

        if len(ep.egress_networkpolicies) == 0:
            return data

        for networkpolicy_name in ep.egress_networkpolicies:
            networkpolicy_spec = networkpolicy_opr.get_networkpolicy_from_cluster(networkpolicy_name)
            self.fill_data_from_directional_traffic_rules(data, direction, networkpolicy_spec)
        self.build_access_rules(data, ep)
        return data

    def init_data_for_networkpolicy(self):
        data = {
            "indexed_policy_count": 0,
            "networkpolicy_map": {},
            "cidrs_map_no_except": {},
            "cidrs_map_with_except": {},
            "cidrs_map_except": {},
            "ports_map": {},
            "label_networkpolicies_map": {},
            "namespace_label_networkpolicies_map": {},
            "cidr_and_policies_map_no_except": {},
            "cidr_and_policies_map_with_except": {},
            "cidr_and_policies_map_except": {},
            "port_and_policies_map": {},
            "indexed_policy_map": {},
            "cidr_table_no_except": [],
            "cidr_table_with_except": [],
            "cidr_table_except": [],
            "port_table": [],
        }
        return data

    def fill_data_from_directional_traffic_rules(self, data, direction, networkpolicy_spec):
        policy_name = "{}:{}".format(networkpolicy_spec["metadata"]["namespace"], networkpolicy_spec["metadata"]["name"])
        if direction in networkpolicy_spec["spec"]:
            for index, directional_traffic_rules in enumerate(networkpolicy_spec["spec"][direction]):
                indexed_policy_name = "{}_{}_{}".format(policy_name, direction, index)
                if policy_name not in data["networkpolicy_map"]:
                    data["networkpolicy_map"][policy_name] = set()
                if indexed_policy_name not in data["networkpolicy_map"][policy_name]:
                    data["networkpolicy_map"][policy_name].add(indexed_policy_name)
                    data["indexed_policy_count"] += 1

                self.fill_cidrs_from_directional_traffic_rules(data, policy_name, indexed_policy_name, direction, directional_traffic_rules)

    def fill_cidrs_from_directional_traffic_rules(self, data, policy_name, indexed_policy_name, direction, directional_traffic_rules):
        if indexed_policy_name not in data["cidrs_map_no_except"]:
            data["cidrs_map_no_except"][indexed_policy_name] = []
        if indexed_policy_name not in data["ports_map"]:
                data["ports_map"][indexed_policy_name] = []

        if len(directional_traffic_rules) == 0: # In such case, it means "Default to allow all triaffic"
            data["cidrs_map_no_except"][indexed_policy_name].append("0.0.0.0/0")
            data["ports_map"][indexed_policy_name].append("any:0")
        else:
            if "ports" in directional_traffic_rules:
                for port in directional_traffic_rules["ports"]:
                    data["ports_map"][indexed_policy_name].append("{}:{}".format(port["protocol"], port["port"]))
            else:
                data["ports_map"][indexed_policy_name].append("any:0")
            
            if indexed_policy_name not in data["cidrs_map_with_except"]:
                data["cidrs_map_with_except"][indexed_policy_name] = []
            if indexed_policy_name not in data["cidrs_map_except"]:
                data["cidrs_map_except"][indexed_policy_name] = []
            for rule_item in directional_traffic_rules["from" if direction == "ingress" else "to"]:
                if "ipBlock" in rule_item:
                    if "except" in rule_item["ipBlock"]:
                        data["cidrs_map_with_except"][indexed_policy_name].append(rule_item["ipBlock"]["cidr"])
                        for except_cidr in rule_item["ipBlock"]["except"]:
                            data["cidrs_map_except"][indexed_policy_name].append(except_cidr)
                    else:
                        data["cidrs_map_no_except"][indexed_policy_name].append(rule_item["ipBlock"]["cidr"])
                elif "namespaceSelector" in rule_item and "podSelector" in rule_item:
                    self.add_label_networkpolicy(data, rule_item["podSelector"]["matchLabels"], policy_name)
                    self.add_namespace_label_networkpolicy(data, rule_item["namespaceSelector"]["matchLabels"], policy_name)
                    namespaces = kube_list_namespaces_by_labels(networkpolicy_opr.core_api, rule_item["namespaceSelector"]["matchLabels"])
                    if namespaces is not None:
                        namespace_set = set()
                        for namespace in namespaces.items:
                            namespace_set.add(namespace.metadata.name)

                        pods = kube_list_pods_by_labels(networkpolicy_opr.core_api, rule_item["podSelector"]["matchLabels"])
                        if pods is not None:
                            for pod in pods.items:
                                if pod.metadata.namespace in namespace_set:
                                    self.add_pod_ip_into_cidrs_map(pod, policy_name, data["cidrs_map_no_except"][indexed_policy_name])
                elif "namespaceSelector" in rule_item:
                    self.add_namespace_label_networkpolicy(data, rule_item["namespaceSelector"]["matchLabels"], policy_name)
                    namespaces = kube_list_namespaces_by_labels(networkpolicy_opr.core_api, rule_item["namespaceSelector"]["matchLabels"])
                    if namespaces is not None:
                        for namespace in namespaces.items:
                            pods = kube_list_pods_by_namespace(networkpolicy_opr.core_api, namespace.metadata.name)
                            if pods is not None:
                                for pod in pods.items:
                                    self.add_pod_ip_into_cidrs_map(pod, policy_name, data["cidrs_map_no_except"][indexed_policy_name])
                elif "podSelector" in rule_item:
                    self.add_label_networkpolicy(data, rule_item["podSelector"]["matchLabels"], policy_name)
                    pods = kube_list_pods_by_labels(networkpolicy_opr.core_api, rule_item["podSelector"]["matchLabels"])
                    if pods is not None:
                        for pod in pods.items:                            
                            self.add_pod_ip_into_cidrs_map(pod, policy_name, data["cidrs_map_no_except"][indexed_policy_name])
                else:
                    raise NotImplementedError("Not implemented for {}".format(rule_item))

    def add_pod_ip_into_cidrs_map(self, pod, policy_name, pod_ip_set):
        if pod.status.pod_ip is None:
            logger.info("pod {} hasn't been assigned ip yet. Will update networkpolicy data for the pod later.".format(pod.metadata.name))
            networkpolicy_opr.store.add_networkpolicies_to_be_updated(pod.metadata.name, policy_name)
        else:
            if pod.metadata.name not in networkpolicy_opr.store.pod_names_to_be_ignored_by_networkpolicy:
                pod_ip_set.append("{}/32".format(pod.status.pod_ip))

    def add_label_networkpolicy(self, data, label_dict, policy_name):
        for key in label_dict:
            label = "{}={}".format(key, label_dict[key])
            if label not in data["label_networkpolicies_map"]:
                data["label_networkpolicies_map"][label] = set()
            data["label_networkpolicies_map"][label].add(policy_name)

    def add_namespace_label_networkpolicy(self, data, label_dict, policy_name):
        for key in label_dict:
            label = "{}={}".format(key, label_dict[key])
            if label not in data["namespace_label_networkpolicies_map"]:
                data["namespace_label_networkpolicies_map"][label] = set()
            data["namespace_label_networkpolicies_map"][label].add(policy_name)

    def wait_until_pod_has_ip(self, pod_name):
        timeout = 60
        while timeout > 0:
            pod = kube_get_pod(networkpolicy_opr.core_api, pod_name)
            if pod is not None and pod.status.pod_ip is not None:
                return
            time.sleep(1)
            timeout -= 1

    def build_access_rules(self, access_rules, ep):
        self.build_cidr_and_policies_map(access_rules, "no_except")
        self.build_cidr_and_policies_map(access_rules, "with_except")
        self.build_cidr_and_policies_map(access_rules, "except")
        self.build_port_and_policies_map(access_rules)
        self.build_indexed_policy_map(access_rules)
        self.build_cidr_table(access_rules, ep, "no_except")
        self.build_cidr_table(access_rules, ep, "with_except")
        self.build_cidr_table(access_rules, ep, "except")
        self.build_port_table(access_rules, ep)

    def build_cidr_and_policies_map(self, access_rules, cidr_type):
        cidr_map_name = "cidrs_map_" + cidr_type
        cidr_and_policies_map_name = "cidr_and_policies_map_" + cidr_type
        trie = IPv4Trie()
        for indexed_policy_name, cidrs in access_rules[cidr_map_name].items():
            for cidr in cidrs:
                if cidr not in access_rules[cidr_and_policies_map_name]:
                    access_rules[cidr_and_policies_map_name][cidr] = set()
                access_rules[cidr_and_policies_map_name][cidr].add(indexed_policy_name)
                trie.insert(cidr, access_rules[cidr_and_policies_map_name][cidr])
        for cidr, indexed_policy_names in access_rules[cidr_and_policies_map_name].items():
            found_cidr_map = trie.find_all(cidr)
            for found_cidr_tuple in found_cidr_map:
                if indexed_policy_names != found_cidr_tuple[1]:
                    for foundPolicyName in found_cidr_tuple[1]:
                        indexed_policy_names.add(foundPolicyName)

    def build_port_and_policies_map(self, access_rules):
        for indexed_policy_name, ports in access_rules["ports_map"].items():
            for port in ports:
                if port not in access_rules["port_and_policies_map"]:
                    access_rules["port_and_policies_map"][port] = set()
                access_rules["port_and_policies_map"][port].add(indexed_policy_name)

    def build_indexed_policy_map(self, access_rules):
        bit = 1
        for _, indexed_policy_names in access_rules["networkpolicy_map"].items():
            for indexed_policy_name in indexed_policy_names:
                access_rules["indexed_policy_map"][indexed_policy_name] = bit
                bit <<= 1

    def build_cidr_table(self, access_rules, ep, cidr_type):
        cidr_table_name = "cidr_table_" + cidr_type
        cidr_and_policies_map_name = "cidr_and_policies_map_" + cidr_type
        for cidr, indexed_policy_names in access_rules[cidr_and_policies_map_name].items():
            splitted_cidr = cidr.split("/")
            access_rules[cidr_table_name].append({
                "vni": ep.vni,
                "local_ip": ep.ip,
                "cidr": splitted_cidr[0],
                "cidr_length": int(splitted_cidr[1]),
                "bit_value": self.calculate_policy_bit_value(access_rules, indexed_policy_names),
            })

    def calculate_policy_bit_value(self, access_rules, indexed_policy_names):
        policy_bit_value = 0
        for indexed_policy_name in indexed_policy_names:
            policy_bit_value += access_rules["indexed_policy_map"][indexed_policy_name]
        return policy_bit_value

    def build_port_table(self, access_rules, ep):
        for port, indexed_policy_names in access_rules["port_and_policies_map"].items():
            splitted = port.split(":")
            access_rules["port_table"].append({
                "vni": ep.vni,
                "local_ip": ep.ip,
                "protocol": splitted[0],
                "port": splitted[1],
                "bit_value": self.calculate_policy_bit_value(access_rules, indexed_policy_names),
            })

    def retrieve_change_for_networkpolicy(self, pod_name, namespace, pod_labels, diff):
        data = self.extract_label_change(diff)

        # Followed code piece is to calculate how many policy are affected by the label change, and form data of policy_name_list.
        # Either label is added or removed, policy is affected, so for both data["add"] and data["remove"] we call same function add_affected_networkpolicy_by_pod_label.
        # Here label is defined in policy's podSelector of ingress rule or egress rule.
        policy_name_list = set()
        for label in data["add"]:
            self.add_affected_networkpolicy_by_pod_label(policy_name_list, label)
        for label in data["remove"]:
            self.add_affected_networkpolicy_by_pod_label(policy_name_list, label)

        endpoint_affected_policy_name_list = set()
        # Followed code piece is to calculate how many policy are affected by the label change, and form data of endpoint_affected_policy_name_list.
        # Either label is added or removed, policy is affected, so for both data["add"] and data["remove"] we call same function add_endpoint_affected_networkpolicy_by_pod_label.
        # Here label is defined in policy's podSelector which affecting policy's mapping to endpoints.
        for label in data["add"]:            
            self.add_endpoint_affected_networkpolicy_by_pod_label(endpoint_affected_policy_name_list, label)
        for label in data["remove"]:
            self.add_endpoint_affected_networkpolicy_by_pod_label(endpoint_affected_policy_name_list, label)

        for policy_name in endpoint_affected_policy_name_list:
            networkpolicy = networkpolicy_opr.store.get_networkpolicy(policy_name)
            if networkpolicy is None:
                logger.warn("In operator store, found affected networkpolicy {} by pod label change but cannot retrieve it from networkpolicies_store.".format(policy_name))
            else:
                self.update_and_retrieve_endpoint_names(policy_name, networkpolicy.namespace, networkpolicy.pod_label_dict, networkpolicy.policy_types)
                policy_name_list.add(policy_name)

        # Followed code piece is to detect how many policies are affect by a pod change through namespace.
        # Policy rules have namespaceSelector. So if there is pod created in the namespace, it should affect policy map data.
        namespace_obj = kube_get_namespace(networkpolicy_opr.core_api, namespace)
        if namespace_obj is not None and namespace_obj.metadata.labels is not None:
            self.add_affected_networkpolicy_by_namespace_labels(policy_name_list, namespace_obj.metadata.labels)

        pod_label_combination = self.get_label_combination(pod_labels)
        pod_label_value = networkpolicy_opr.store.get_or_add_pod_label_value(pod_label_combination)

        namespace_label_combination = self.get_label_combination(namespace_obj.metadata.labels)
        namespace_label_value = networkpolicy_opr.store.get_or_add_namespace_label_value(namespace_label_combination)

        return (policy_name_list, pod_label_value, namespace_label_value)

    def get_label_combination(self, labels):
        if not labels:
            return ""

        label_list = []
        for key in sorted(labels):
            label_list.append("{}={}".format(key, labels[key]))
        return ','.join(label_list)
    
    def handle_pod_delete_for_networkpolicy(self, pod_name, namespace, diff, eps):
        data = self.extract_label_change(diff)

        policy_name_list = set()
        for label in data["remove"]:
            self.add_affected_networkpolicy_by_pod_label(policy_name_list, label)

        endpoint_affected_policy_name_list = set()
        for label in data["remove"]:
            self.add_endpoint_affected_networkpolicy_by_pod_label(endpoint_affected_policy_name_list, label)

        for policy_name in endpoint_affected_policy_name_list:
            networkpolicy = networkpolicy_opr.store.get_networkpolicy(policy_name)
            if networkpolicy is None:
                logger.warn("In operator store, found affected networkpolicy {} by pod label change but cannot retrieve it from networkpolicies_store.".format(policy_name))
            else:
                endpoint_names = self.update_and_retrieve_endpoint_names(policy_name, networkpolicy.namespace, None, None)
                self.handle_networkpolicy_update_delete(endpoint_names)

        namespace_obj = kube_get_namespace(networkpolicy_opr.core_api, namespace)
        if namespace_obj is not None and namespace_obj.metadata.labels is not None:
            self.add_affected_networkpolicy_by_namespace_labels(policy_name_list, namespace_obj.metadata.labels)

        for policy_name in policy_name_list:
            for endpoint_name in eps:
                if policy_name in networkpolicy_opr.store.networkpolicy_endpoints_ingress_store and endpoint_name in networkpolicy_opr.store.networkpolicy_endpoints_ingress_store[policy_name]:
                    networkpolicy_opr.store.networkpolicy_endpoints_ingress_store[policy_name].remove(endpoint_name)
                if policy_name in networkpolicy_opr.store.networkpolicy_endpoints_egress_store and endpoint_name in networkpolicy_opr.store.networkpolicy_endpoints_egress_store[policy_name]:
                    networkpolicy_opr.store.networkpolicy_endpoints_egress_store[policy_name].remove(endpoint_name)

        self.handle_networkpolicy_change(policy_name_list)

    def add_affected_networkpolicy_by_namespace_labels(self, policy_name_list, label_dict):
        for key in label_dict:
            label = "{}={}".format(key, label_dict[key])
            if label in networkpolicy_opr.store.namespace_label_networkpolicies_ingress_store:
                for policy_name in networkpolicy_opr.store.namespace_label_networkpolicies_ingress_store[label]:
                    policy_name_list.add(policy_name)
            if label in networkpolicy_opr.store.namespace_label_networkpolicies_egress_store:
                for policy_name in networkpolicy_opr.store.namespace_label_networkpolicies_egress_store[label]:
                    policy_name_list.add(policy_name)

    def add_affected_networkpolicy_by_pod_label(self, policy_name_list, label):
        if label in networkpolicy_opr.store.label_networkpolicies_ingress_store:
            for policy_name in networkpolicy_opr.store.label_networkpolicies_ingress_store[label]:
                policy_name_list.add(policy_name)
        if label in networkpolicy_opr.store.label_networkpolicies_egress_store:
            for policy_name in networkpolicy_opr.store.label_networkpolicies_egress_store[label]:
                policy_name_list.add(policy_name)

    # Here endpoint is not a noun. It's adjectives to describe networkpolicy. It's "endpoint_affected_networkpolicy". 
    # In a networkpolicy definition, the podSelector is a list of labels. And the podSelector can appear two places: 
    # one is in policy ingress/egress rules, the second one is in the beginning of the policy definition which means the policy's power on the pods/endpoints. 
    # Accordingly, when a pod changed label, it may affect networkpolicy ingress/egress rules, or affect networkpolicy-endpoints mapping. 
    # If a policy is affected by pod label change, I call it "endpoint affected networkpolicy".
    def add_endpoint_affected_networkpolicy_by_pod_label(self, policy_name_list, label):
        if label in networkpolicy_opr.store.label_networkpolicies_store:
            for policy_name in networkpolicy_opr.store.label_networkpolicies_store[label]:
                policy_name_list.add(policy_name)

    def handle_namespace_change_for_networkpolicy(self, diff):
        data = self.extract_label_change(diff)
        if len(data["add"]) == 0 and len(data["remove"]) == 0:
            return

        policy_name_list = set()
        for label in data["add"]:
            self.add_affected_networkpolicy_by_namespace_label(policy_name_list, label)

        for label in data["remove"]:
            self.add_affected_networkpolicy_by_namespace_label(policy_name_list, label)

        self.handle_networkpolicy_change(policy_name_list)

    def handle_networkpolicy_change(self, policy_name_list):
        if len(policy_name_list) == 0:
            return

        endpoint_name_list = set()
        for policy_name in policy_name_list:
            if policy_name in networkpolicy_opr.store.networkpolicy_endpoints_ingress_store:
                for endpoint_name in networkpolicy_opr.store.networkpolicy_endpoints_ingress_store[policy_name]:
                    endpoint_name_list.add(endpoint_name)
            if policy_name in networkpolicy_opr.store.networkpolicy_endpoints_egress_store:
                for endpoint_name in networkpolicy_opr.store.networkpolicy_endpoints_egress_store[policy_name]:
                    endpoint_name_list.add(endpoint_name)
        
        for endpoint_name in endpoint_name_list:
            ep = endpoint_opr.store_get(endpoint_name)
            if ep is None:
                logger.warn("In operator store, found endpoint {} in networkpolicy_endpoints_(ingress/egress)_store but cannot retrieve it from eps_store.".format(endpoint_name))
            else:
                if ep.ip == "":
                    logger.info("Endpoint {} hasn't been assigned ip yet. Will update networkpolicy data for the endpoint later.".format(endpoint_name))
                    networkpolicy_opr.store.eps_store_to_be_updated_networkpolicy.add(endpoint_name)
                else:
                    logger.info("Update networkpolicy data for endpoint {}".format(ep.name))
                    self.handle_endpoint_for_networkpolicy(ep)

    def add_affected_networkpolicy_by_namespace_label(self, policy_name_list, label):
        if label in networkpolicy_opr.store.namespace_label_networkpolicies_ingress_store:
            for policy_name in networkpolicy_opr.store.namespace_label_networkpolicies_ingress_store[label]:
                policy_name_list.add(policy_name)
        if label in networkpolicy_opr.store.namespace_label_networkpolicies_egress_store:
            for policy_name in networkpolicy_opr.store.namespace_label_networkpolicies_egress_store[label]:
                policy_name_list.add(policy_name)

    def extract_label_change(self, diff):
        data = {
            "add": set(),
            "remove": set()
        }
        for item in diff:
            self.process_label_change(data, item[0], item[1], item[2], item[3])

        return data        

    def process_label_change(self, data, change_type, field, old, new):
        if field is not None and len(field) == 3 and field[0] == "metadata" and field[1] == "labels":
            if change_type == "add":
                data["add"].add("{}={}".format(field[2], new))
            elif change_type == "remove":
                data["remove"].add("{}={}".format(field[2], old))
            elif change_type == "change":
                data["add"].add("{}={}".format(field[2], new))
                data["remove"].add("{}={}".format(field[2], old))
            else:
                raise NotImplementedError("Not implemented for label change type of {}".format(change_type))
        elif field is not None and len(field) == 0:
            if change_type == "add" and "metadata" in new and new["metadata"] is not None and new["metadata"]["labels"] is not None:
                labels = new["metadata"]["labels"]
                for key in labels:
                    data["add"].add("{}={}".format(key, labels[key]))
            elif change_type == "remove" and "metadata" in old and old["metadata"] is not None and old["metadata"]["labels"] is not None:
                labels = old["metadata"]["labels"]
                for key in labels:
                    data["remove"].add("{}={}".format(key, labels[key]))
