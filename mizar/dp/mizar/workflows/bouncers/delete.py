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
from mizar.dp.mizar.operators.bouncers.bouncers_operator import *
from mizar.dp.mizar.operators.endpoints.endpoints_operator import *
from mizar.dp.mizar.operators.nets.nets_operator import *

logger = logging.getLogger()

dividers_opr = DividerOperator()
bouncers_opr = BouncerOperator()
endpoints_opr = EndpointOperator()
nets_opr = NetOperator()


class BouncerDelete(WorkflowTask):

    def requires(self):
        logger.info("Requires {task}".format(task=self.__class__.__name__))
        return []

    def run(self):
        logger.info("Run {task}".format(task=self.__class__.__name__))
        bouncer = bouncers_opr.store.get_bouncer(self.param.name)
        bouncer.set_obj_spec(self.param.spec)
        net = nets_opr.store.get_net(bouncer.net)
        # Call update_net on all divider objects
        # Call delete_substrate of bouncer droplet on all divider droplets

        # Call delete_ep on bouncer droplet for all endpoints
        # Call update_agent on all endpoints with new list of bouncers
        endpoints_opr.delete_bouncer_from_endpoints(bouncer)
        dividers_opr.delete_bouncer_from_dividers(bouncer, net)
        endpoints_opr.delete_endpoints_from_bouncers(bouncer)
        bouncers_opr.delete_vpc(bouncer)
        bouncer.delete_obj()
        bouncers_opr.store.delete_bouncer(bouncer.name)
        self.finalize()
