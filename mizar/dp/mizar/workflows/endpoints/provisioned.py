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
import time
from mizar.common.workflow import *
from mizar.dp.mizar.operators.endpoints.endpoints_operator import *
from mizar.dp.mizar.operators.droplets.droplets_operator import *
from mizar.networkpolicy.networkpolicy_util import *

logger = logging.getLogger()

endpoints_opr = EndpointOperator()
droplets_opr = DropletOperator()
networkpolicy_util = NetworkPolicyUtil()


class EndpointProvisioned(WorkflowTask):

    def requires(self):
        logger.info("Requires {task}".format(task=self.__class__.__name__))
        return []

    def run(self):
        logger.info("Run {task}".format(task=self.__class__.__name__))
        endpoint = endpoints_opr.get_endpoint_stored_obj(
            self.param.name, self.param.spec)
        endpoint.droplet_obj = droplets_opr.store.get_droplet(endpoint.droplet)
        endpoints_opr.store_update(endpoint)

        if self.param.name in endpoints_opr.store.eps_store_to_be_updated_networkpolicy:
            endpoints_opr.store.eps_store_to_be_updated_networkpolicy.remove(self.param.name)
            time.sleep(1) # Wait a little time for newly created endpoint network.
            if len(endpoint.ingress_networkpolicies) == 1:
                endpoint.update_network_policy_enforcement_map_ingress()
            if len(endpoint.egress_networkpolicies) == 1:
                endpoint.update_network_policy_enforcement_map_egress()
            networkpolicy_util.handle_endpoint_for_networkpolicy(endpoint)

        if endpoint.pod in endpoints_opr.store.networkpolicies_to_be_updated_store:
            networkpolicy_util.wait_until_pod_has_ip(endpoint.pod)
            networkpolicy_util.handle_networkpolicy_change(endpoints_opr.store.networkpolicies_to_be_updated_store[endpoint.pod])
            endpoints_opr.store.networkpolicies_to_be_updated_store.pop(endpoint.pod)

        self.finalize()
