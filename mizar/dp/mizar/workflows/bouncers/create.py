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
from mizar.dp.mizar.operators.dividers.dividers_operator import *
from mizar.dp.mizar.operators.droplets.droplets_operator import *
from mizar.dp.mizar.operators.bouncers.bouncers_operator import *
from mizar.dp.mizar.operators.endpoints.endpoints_operator import *
from mizar.dp.mizar.operators.nets.nets_operator import *
from mizar.dp.mizar.operators.vpcs.vpcs_operator import *

logger = logging.getLogger()

dividers_opr = DividerOperator()
droplets_opr = DropletOperator()
bouncers_opr = BouncerOperator()
endpoints_opr = EndpointOperator()
nets_opr = NetOperator()
vpcs_opr = VpcOperator()

logger = logging.getLogger()


class BouncerCreate(WorkflowTask):

    def requires(self):
        logger.info("Requires {task}".format(task=self.__class__.__name__))
        return []

    def run(self):
        logger.info("Run {task}".format(task=self.__class__.__name__))
        bouncer = bouncers_opr.get_bouncer_stored_obj(
            self.param.name, self.param.spec)
        if not droplets_opr.is_bootstrapped():
            self.raise_temporary_error(
                "Task: {} Bouncer: {} Droplet operator not ready.".format(self.__class__.__name__, bouncer.name))
        net = nets_opr.store.get_net(bouncer.net)
        if not net:
            self.raise_temporary_error(
                "Task: {} Bouncer: {} Net {} not ready.".format(self.__class__.__name__, bouncer.name, bouncer.net))
        if not droplets_opr.assign_bouncer_droplet(bouncer):
            self.raise_temporary_error("Task: {} Bouncer: {} No droplets available.".format(
                self.__class__.__name__, bouncer.name))

        # Update vpc on bouncer
        # Needs a list of all dividers
        bouncer.set_vni(vpcs_opr.store.get_vpc(bouncer.vpc).vni)
        dividers_opr.update_vpc(bouncer)

        # Update net on dividers

        net.bouncers[bouncer.name] = bouncer
        dividers_opr.update_divider_with_bouncers(bouncer, net)
        endpoints_opr.update_bouncer_with_endpoints(bouncer)
        endpoints_opr.update_endpoints_with_bouncers(bouncer)
        bouncer.load_transit_xdp_pipeline_stage()
        bouncers_opr.set_bouncer_provisioned(bouncer)
        bouncers_opr.store.update_bouncer(bouncer)
        self.finalize()
