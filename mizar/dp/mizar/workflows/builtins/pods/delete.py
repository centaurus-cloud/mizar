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
from mizar.common.workflow import *
from mizar.dp.mizar.operators.endpoints.endpoints_operator import *
from mizar.networkpolicy.networkpolicy_util import *
logger = logging.getLogger()

endpoints_opr = EndpointOperator()
networkpolicy_util = NetworkPolicyUtil()


class k8sPodDelete(WorkflowTask):

    def requires(self):
        logger.info("Requires {task}".format(task=self.__class__.__name__))
        return []

    def run(self):
        logger.info("Run {task}".format(task=self.__class__.__name__))

        eps = endpoints_opr.store.get_eps_in_pod(self.param.name)

        endpoints_opr.store.pod_names_to_be_ignored_by_networkpolicy.add(self.param.name)
        networkpolicy_util.handle_pod_delete_for_networkpolicy(self.param.name, self.param.namespace, self.param.diff, eps)
        endpoints_opr.store.pod_names_to_be_ignored_by_networkpolicy.remove(self.param.name)

        logger.info("Deleting Pod {}".format(self.param.name))

        for ep in eps.values():
            endpoints_opr.delete_simple_endpoint(ep)

        self.finalize()
