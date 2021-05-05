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
from mizar.dp.mizar.operators.nets.nets_operator import *
from mizar.dp.mizar.operators.dividers.dividers_operator import *
from mizar.dp.mizar.operators.bouncers.bouncers_operator import *
from mizar.dp.mizar.operators.endpoints.endpoints_operator import *
from mizar.dp.mizar.operators.droplets.droplets_operator import *

logger = logging.getLogger()

nets_opr = NetOperator()
dividers_opr = DividerOperator()
bouncers_opr = BouncerOperator()
endpoints_opr = EndpointOperator()
droplets_opr = DropletOperator()


class NetCreate(WorkflowTask):

    def requires(self):
        logger.info("Requires {task}".format(task=self.__class__.__name__))
        return []

    def run(self):
        logger.info("Run {task}".format(task=self.__class__.__name__))
        n = nets_opr.store.get_net(self.param.name)
        if not n:
            logger.info("NetCreate: Net not found in store. Creating new network object for {}".format(
                self.param.name))
            n = nets_opr.get_net_stored_obj(self.param.name, self.param.spec)
        if len(droplets_opr.store.get_all_droplets()) == 0:
            self.raise_temporary_error(
                "Task: {} Net: {} No droplets available.".format(self.__class__.__name__, n.name))
        if len(dividers_opr.store.get_dividers_of_vpc(n.vpc)) < 1:
            self.raise_temporary_error(
                "Task: {} Net: {} Dividers not available".format(self.__class__.__name__, n.name))
        logger.info("NetCreate Net ip is {}".format(n.ip))
        nets_opr.create_net_bouncers(n, n.n_bouncers)
        nets_opr.set_net_provisioned(n)
        nets_opr.store_update(n)
        ep = endpoints_opr.create_gw_endpoint(
            self.param.name + "_gw", n.get_gw_ip(), n.vni, n.vpc, n.name)
        endpoints_opr.store_update(ep)
        self.finalize()
