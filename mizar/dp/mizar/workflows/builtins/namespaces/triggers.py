# SPDX-License-Identifier: MIT
# Copyright (c) 2021 The Authors.

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

import kopf
import logging
import luigi
from mizar.common.common import *
from mizar.common.constants import *
from mizar.common.wf_factory import *
from mizar.common.wf_param import *

logger = logging.getLogger()


@kopf.on.resume('', 'v1', 'namespaces', retries=OBJ_DEFAULTS.kopf_max_retries, when=LAMBDAS.k8s_provider_vanilla)
@kopf.on.update('', 'v1', 'namespaces', retries=OBJ_DEFAULTS.kopf_max_retries, when=LAMBDAS.k8s_provider_vanilla)
@kopf.on.create('', 'v1', 'namespaces', retries=OBJ_DEFAULTS.kopf_max_retries, when=LAMBDAS.k8s_provider_vanilla)
async def builtins_on_namespace(body, spec, **kwargs):
    param = HandlerParam()
    param.name = kwargs['name']
    param.body = body
    param.spec = spec
    param.diff = kwargs['diff']
    run_workflow(wffactory().k8sNamespaceCreate(param=param))


@kopf.on.delete('', 'v1', 'namespaces', retries=OBJ_DEFAULTS.kopf_max_retries, when=LAMBDAS.k8s_provider_vanilla)
async def builtins_on_namespace_delete(body, spec, **kwargs):
    param = HandlerParam()
    param.name = kwargs['name']
    param.body = body
    param.spec = spec
    param.diff = kwargs['diff']
    run_workflow(wffactory().k8sNamespaceDelete(param=param))
