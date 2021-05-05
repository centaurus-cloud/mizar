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
from mizar.dp.mizar.operators.vpcs.vpcs_operator import *
from mizar.dp.mizar.operators.droplets.droplets_operator import *

logger = logging.getLogger()
vpcs_opr = VpcOperator()
droplets_opr = DropletOperator()


class VpcCreate(WorkflowTask):

    def requires(self):
        logger.info("Requires {task}".format(task=self.__class__.__name__))
        return []

    def run(self):
        logger.info("Run {task}".format(task=self.__class__.__name__))
        v = vpcs_opr.store.get_vpc(self.param.name)
        if not v:
            logger.info("VpcCreate: VPC not found in store. Creating new VPC object for {}".format(
                self.param.name))
            v = vpcs_opr.get_vpc_stored_obj(self.param.name, self.param.spec)
        if len(droplets_opr.store.get_all_droplets()) == 0:
            self.raise_temporary_error(
                "Task: {} VPC: {} No droplets available.".format(self.__class__.__name__, v.name))
        logger.info("VpcCreate VPC IP is {}".format(v.ip))
        vpcs_opr.allocate_vni(v)
        vpcs_opr.create_vpc_dividers(v, v.n_dividers)
        vpcs_opr.set_vpc_provisioned(v)
        vpcs_opr.store_update(v)
        self.finalize()
