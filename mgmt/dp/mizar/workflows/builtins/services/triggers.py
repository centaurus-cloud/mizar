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

import kopf
import logging
import luigi
from common.common import *
from common.constants import *
from common.wf_factory import *
from common.wf_param import *

logger = logging.getLogger()
mizar_service_annotation_key = "service.beta.kubernetes.io/mizar-scaled-endpoint-type"
mizar_service_annotation_val = "scaled-endpoint"
annotations_filter = {mizar_service_annotation_key: mizar_service_annotation_val}

@kopf.on.resume('', 'v1', 'services', annotations=annotations_filter)
@kopf.on.update('', 'v1', 'services', annotations=annotations_filter)
@kopf.on.create('', 'v1', 'services', annotations=annotations_filter)
async def services_opr_on_services(body, spec, **kwargs):
	param = HandlerParam()
	param.name = kwargs['name']
	param.body = body
	param.spec = spec
	run_task(wffactory().k8sServiceCreate(param=param))


@kopf.on.resume('', 'v1', 'endpoints')
@kopf.on.update('', 'v1', 'endpoints')
@kopf.on.create('', 'v1', 'endpoints')
async def services_opr_on_endpoints(body, spec, **kwargs):
	param = HandlerParam()
	param.name = kwargs['name']
	param.body = body
	param.spec = spec
	run_task(wffactory().k8sEndpointsUpdate(param=param))