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
from mizar.dp.mizar.operators.nets.nets_operator import *
logger = logging.getLogger()

dividers_opr = DividerOperator()
droplets_opr = DropletOperator()
bouncers_opr = BouncerOperator()
nets_opr = NetOperator()


class DividerCreate(WorkflowTask):

    def requires(self):
        logger.info("Requires {task}".format(task=self.__class__.__name__))
        return []

    def run(self):
        logger.info("Run {task}".format(task=self.__class__.__name__))
        divider = dividers_opr.get_divider_stored_obj(
            self.param.name, self.param.spec)
        if not droplets_opr.is_bootstrapped():
            self.raise_temporary_error(
                "Task: {} Divider: {} Droplet operator not ready.".format(self.__class__.__name__, divider.name))
        if not droplets_opr.assign_divider_droplet(divider):
            self.raise_temporary_error(
                "Task: {} Divider: {} No droplets available.".format(self.__class__.__name__, divider.name))

        # Update vpc on bouncers
        bouncers_opr.update_bouncers_with_divider(divider)

        # Update divider with bouncers
        nets = nets_opr.store.get_nets_in_vpc(divider.vpc)
        for net in nets.values():
            dividers_opr.update_net(net, set([divider]))

        dividers_opr.set_divider_provisioned(divider)
        dividers_opr.store.update_divider(divider)
        self.finalize()
