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
import json
from mizar.common.workflow import *
from mizar.dp.mizar.operators.droplets.droplets_operator import *
from mizar.dp.mizar.operators.endpoints.endpoints_operator import *
from mizar.dp.mizar.operators.vpcs.vpcs_operator import *
from mizar.dp.mizar.operators.nets.nets_operator import *
from mizar.common.constants import *
from mizar.networkpolicy.networkpolicy_util import *

logger = logging.getLogger()

droplet_opr = DropletOperator()
endpoint_opr = EndpointOperator()
vpc_opr = VpcOperator()
net_opr = NetOperator()
networkpolicy_util = NetworkPolicyUtil()


class k8sPodCreate(WorkflowTask):

    def requires(self):
        logger.info("Requires {task}".format(task=self.__class__.__name__))
        return []

    def run(self):
        logger.info("Run {task}".format(task=self.__class__.__name__))

        if "hostIP" not in self.param.body['status']:
            self.raise_temporary_error("Pod spec not ready.")

        spec = {
            'hostIP': self.param.body['status']['hostIP'],
            'name': self.param.name,
            'type': COMPUTE_PROVIDER.kubernetes,
            'namespace': self.param.body['metadata'].get('namespace', 'default'),
            'tenant': self.param.body['metadata'].get('tenant', ''),
            'vpc': OBJ_DEFAULTS.default_ep_vpc,
            'subnet': OBJ_DEFAULTS.default_ep_net,
            'phase': self.param.body['status']['phase'],
            'interfaces': [{'name': 'eth0'}]
        }

        spec['vni'] = vpc_opr.store_get(spec['vpc']).vni
        spec['droplet'] = droplet_opr.store_get_by_ip(spec['hostIP'])
        # Preexisting pods triggered when droplet objects are not yet created.
        if not spec['droplet']:
            self.raise_temporary_error("Droplet not yet created.")

        if self.param.extra:
            spec['type'] = COMPUTE_PROVIDER.arktos
            if "arktos_network" in self.param.extra:
                vpc = vpc_opr.store.get_vpc_in_arktosnet(
                    self.param.extra["arktos_network"])
                if self.param.extra["arktos_network"] == "default":
                    vpc = OBJ_DEFAULTS.default_ep_vpc
                spec['vpc'] = vpc
                nets = net_opr.store.get_nets_in_vpc(vpc)
                net = OBJ_DEFAULTS.default_ep_net
                if nets:
                    net = next(iter(nets.values())).name
                spec["subnet"] = net
                logger.info("Putting pod in VPC {} and Net {}".format(
                    spec["vpc"], spec["subnet"]))
            # Example: arktos.futurewei.com/nic: [{"name": "eth0", "ip": "10.10.1.12", "subnet": "net1"}]
            # all three fields are optional. Each item in the list corresponding to an endpoint
            # which represents a network interface for a pod
            if "interfaces" in self.param.extra:
                net_config = self.param.extra["interfaces"]
                configs = json.loads(net_config)
                spec['interfaces'] = configs

        n = net_opr.store.get_net(spec['subnet'])
        ip = n.allocate_ip()
        spec['ip'] = ip

        logger.info("Pod spec {}".format(spec))

        # make sure not to trigger init or create simple endpoint
        # if Arktos network is already marked ready (Needs to confirm with Arktos team)
        # if spec['type'] ==  COMPUTE_PROVIDER.arktos && spec['readiness'] == true:
        #     self.finalize()
        #     return

        (policy_name_list, pod_label_value, namespace_label_value) = networkpolicy_util.retrieve_change_for_networkpolicy(self.param.name, self.param.namespace, self.param.body.metadata.labels, self.param.diff)
        spec['pod_label_value'] = pod_label_value
        spec['namespace_label_value'] = namespace_label_value
        if spec['phase'] != 'Pending':
            networkpolicy_util.handle_networkpolicy_change(policy_name_list)
            self.finalize()
            return

        # Init all interfaces on the host
        interfaces = endpoint_opr.init_simple_endpoint_interfaces(
            spec['hostIP'], spec)
        if not interfaces:
            self.raise_permanent_error(
                "Endpoint {} already exists!".format(spec["name"]))
        # Create the corresponding simple endpoint objects
        endpoint_opr.create_simple_endpoints(interfaces, spec)

        networkpolicy_util.handle_networkpolicy_change(policy_name_list)

        self.finalize()
