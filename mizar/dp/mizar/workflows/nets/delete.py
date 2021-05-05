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
logger = logging.getLogger()

nets_opr = NetOperator()
dividers_opr = DividerOperator()
bouncers_opr = BouncerOperator()
endpoints_opr = EndpointOperator()


class NetDelete(WorkflowTask):

    def requires(self):
        logger.info("Requires {task}".format(task=self.__class__.__name__))
        return []

    def run(self):
        logger.info("Run {task}".format(task=self.__class__.__name__))
        n = nets_opr.store.get_net(self.param.name)
        n.set_obj_spec(self.param.spec)
        # TODO: Handle the error when all endpoints have not been deleted.
        while len(endpoints_opr.store.get_eps_in_net(n.name)):
            pass

        nets_opr.delete_net_bouncers(n, n.n_bouncers)
        while len(bouncers_opr.store.get_bouncers_of_net(n.name)) > 1:
            pass
        dividers_opr.delete_net(n)
        n.delete_obj()
        nets_opr.store.delete_net(n.name)
        self.finalize()
