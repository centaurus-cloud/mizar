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

import luigi
from mizar.common.workflow import *
from mizar.dp.mizar.workflows.vpcs.bootstrap import *
from mizar.dp.mizar.workflows.vpcs.create import *
from mizar.dp.mizar.workflows.vpcs.provisioned import *
from mizar.dp.mizar.workflows.vpcs.delete import *

from mizar.dp.mizar.workflows.dividers.bootstrap import *
from mizar.dp.mizar.workflows.dividers.create import *
from mizar.dp.mizar.workflows.dividers.provisioned import *
from mizar.dp.mizar.workflows.dividers.delete import *

from mizar.dp.mizar.workflows.droplets.bootstrap import *
from mizar.dp.mizar.workflows.droplets.create import *
from mizar.dp.mizar.workflows.droplets.provisioned import *
from mizar.dp.mizar.workflows.droplets.delete import *

from mizar.dp.mizar.workflows.nets.bootstrap import *
from mizar.dp.mizar.workflows.nets.create import *
from mizar.dp.mizar.workflows.nets.provisioned import *
from mizar.dp.mizar.workflows.nets.delete import *

from mizar.dp.mizar.workflows.bouncers.bootstrap import *
from mizar.dp.mizar.workflows.bouncers.create import *
from mizar.dp.mizar.workflows.bouncers.provisioned import *
from mizar.dp.mizar.workflows.bouncers.delete import *

from mizar.dp.mizar.workflows.endpoints.bootstrap import *
from mizar.dp.mizar.workflows.endpoints.create import *
from mizar.dp.mizar.workflows.endpoints.provisioned import *
from mizar.dp.mizar.workflows.endpoints.delete import *

from mizar.dp.mizar.workflows.builtins.services.bootstrap import *
from mizar.dp.mizar.workflows.builtins.services.create import *
from mizar.dp.mizar.workflows.builtins.services.provisioned import *
from mizar.dp.mizar.workflows.builtins.services.delete import *

from mizar.dp.mizar.workflows.builtins.nodes.bootstrap import *
from mizar.dp.mizar.workflows.builtins.nodes.create import *
from mizar.dp.mizar.workflows.builtins.nodes.provisioned import *
from mizar.dp.mizar.workflows.builtins.nodes.delete import *

from mizar.dp.mizar.workflows.builtins.pods.bootstrap import *
from mizar.dp.mizar.workflows.builtins.pods.create import *
from mizar.dp.mizar.workflows.builtins.pods.provisioned import *
from mizar.dp.mizar.workflows.builtins.pods.delete import *

from mizar.dp.mizar.workflows.builtins.namespaces.bootstrap import *
from mizar.dp.mizar.workflows.builtins.namespaces.create import *
from mizar.dp.mizar.workflows.builtins.namespaces.provisioned import *
from mizar.dp.mizar.workflows.builtins.namespaces.delete import *

from mizar.dp.mizar.workflows.builtins.networkpolicies.bootstrap import *
from mizar.dp.mizar.workflows.builtins.networkpolicies.create import *
from mizar.dp.mizar.workflows.builtins.networkpolicies.provisioned import *
from mizar.dp.mizar.workflows.builtins.networkpolicies.delete import *

class MizarWorkflowFactory():

    def VpcOperatorStart(self, param):
        return VpcOperatorStart(param=param)

    def VpcCreate(self, param):
        return VpcCreate(param=param)

    def VpcProvisioned(self, param):
        return VpcProvisioned(param=param)

    def VpcDelete(self, param):
        return VpcDelete(param=param)

    def DividerOperatorStart(self, param):
        return DividerOperatorStart(param=param)

    def DividerCreate(self, param):
        return DividerCreate(param=param)

    def DividerProvisioned(self, param):
        return DividerProvisioned(param=param)

    def DividerDelete(self, param):
        return DividerDelete(param=param)

    def DropletOperatorStart(self, param):
        return DropletOperatorStart(param=param)

    def DropletCreate(self, param):
        return DropletCreate(param=param)

    def DropletProvisioned(self, param):
        return DropletProvisioned(param=param)

    def DropletDelete(self, param):
        return DropletDelete(param=param)

    def NetOperatorStart(self, param):
        return NetOperatorStart(param=param)

    def NetCreate(self, param):
        return NetCreate(param=param)

    def NetProvisioned(self, param):
        return NetProvisioned(param=param)

    def NetDelete(self, param):
        return NetDelete(param=param)

    def BouncerOperatorStart(self, param):
        return BouncerOperatorStart(param=param)

    def BouncerCreate(self, param):
        return BouncerCreate(param=param)

    def BouncerProvisioned(self, param):
        return BouncerProvisioned(param=param)

    def BouncerDelete(self, param):
        return BouncerDelete(param=param)

    def EndpointOperatorStart(self, param):
        return EndpointOperatorStart(param=param)

    def EndpointCreate(self, param):
        return EndpointCreate(param=param)

    def EndpointProvisioned(self, param):
        return EndpointProvisioned(param=param)

    def EndpointDelete(self, param):
        return EndpointDelete(param=param)

    def k8sServiceCreate(self, param):
        return k8sServiceCreate(param=param)

    def k8sEndpointsUpdate(self, param):
        return k8sEndpointsUpdate(param=param)

    def k8sServiceDelete(self, param):
        return k8sServiceDelete(param=param)

    def k8sDropletCreate(self, param):
        return k8sDropletCreate(param=param)

    def k8sDropletDelete(self, param):
        return k8sDropletDelete(param=param)

    def k8sPodCreate(self, param):
        return k8sPodCreate(param=param)

    def k8sPodDelete(self, param):
        return k8sPodDelete(param=param)

    def k8sNamespaceCreate(self, param):
        return k8sNamespaceCreate(param=param)

    def k8sNamespaceDelete(self, param):
        return k8sNamespaceDelete(param=param)

    def k8sNetworkPolicyCreate(self, param):
        return k8sNetworkPolicyCreate(param=param)

    def k8sNetworkPolicyDelete(self, param):
        return k8sNetworkPolicyDelete(param=param)
