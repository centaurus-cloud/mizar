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
from mizar.common.workflow import *
from mizar.dp.mizar.operators.networkpolicies.networkpolicies_operator import *
from mizar.networkpolicy.networkpolicy_util import *

networkpolicy_opr = NetworkPolicyOperator()
networkpolicy_util = NetworkPolicyUtil()
logger = logging.getLogger()

class k8sNetworkPolicyDelete(WorkflowTask):

    def requires(self):
        logger.info("Requires {task}".format(task=self.__class__.__name__))
        return []

    def run(self):
        logger.info("Run {task}".format(task=self.__class__.__name__))

        policy_name = "{}:{}".format(self.param.namespace, self.param.name)
        endpoint_names = networkpolicy_util.update_and_retrieve_endpoint_names(policy_name, self.param.namespace, None, None)

        networkpolicy_util.handle_networkpolicy_update_delete(endpoint_names)
        networkpolicy_opr.store.delete_networkpolicy(policy_name)

        self.finalize()
