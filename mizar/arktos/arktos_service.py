# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Authors.

# Authors: Phu Tran          <@phudtran>

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
import grpc
import json
from mizar.proto.builtins_pb2_grpc import BuiltinsServiceServicer, BuiltinsServiceStub
from mizar.common.workflow import *
from mizar.dp.mizar.operators.vpcs.vpcs_operator import *
from mizar.proto.builtins_pb2 import *
from mizar.common.wf_param import *
from mizar.common.wf_factory import wffactory

vpc_opr = VpcOperator()
logger = logging.getLogger()


class ArktosService(BuiltinsServiceServicer):

    def CreatePod(self, request, context):
        if request.host_ip == '':
            return ReturnCode(
                code=CodeType.TEMP_ERROR,
                message="Missing hostIP during pod create or update"
            )
        logger.info("Creating pod from Arktos Service {}".format(request.name))
        param = reset_param(HandlerParam())
        param.name = request.name
        param.namespace = request.namespace
        param.body['status'] = {}
        param.body['metadata'] = {}
        param.body['status']['hostIP'] = request.host_ip
        param.body['metadata']['namespace'] = request.namespace
        param.body['status']['phase'] = request.phase
        param.body['metadata']['tenant'] = request.tenant
        param.extra = {}
        if request.arktos_network != "":
            param.extra["arktos_network"] = request.arktos_network
        if len(request.interfaces) > 0:
            param.extra["interfaces"] = list()
            itf_string = '['
            for interface in request.interfaces:
                itf_string += '{"name": "{}", "ip": "{}", "subnet": "{}"},'.format(
                    interface.name, interface.ip, interface.subnet)
            itf_string = itf_string[:-1] + ']'
            param.extra.interfaces = itf_string
        return run_arktos_workflow(wffactory().k8sPodCreate(param=param))

    def CreateNode(self, request, context):
        logger.info(
            "Creating droplet from Arktos Service {}".format(request.ip))
        param = reset_param(HandlerParam())
        param.body['status'] = {}
        param.body['status']['addresses'] = list()
        param.body['status']['addresses'].append({})

        param.body['status']['addresses'][0]["type"] = "InternalIP"
        param.body['status']['addresses'][0]["address"] = request.ip
        return run_arktos_workflow(wffactory().k8sDropletCreate(param=param))

    def CreateService(self, request, context):
        logger.info(
            "Create scaled endpoint from Network Controller {} {} {}.".format(request.name, request.namespace, request.tenant))
        param = reset_param(HandlerParam())
        param.name = request.name
        param.body['metadata'] = {}
        param.extra = {}

        param.body['metadata']['namespace'] = request.namespace
        param.spec["clusterIP"] = request.ip
        logger.info("From grpc server: Service IP is {}".format(request.ip))
        param.extra["arktos_network"] = request.arktos_network
        param.extra["tenant"] = request.tenant
        return run_arktos_workflow(wffactory().k8sServiceCreate(param=param))

    def CreateServiceEndpointProtobuf(self, request, context):
        logger.info(
            "Create Service Endpoint from Network Controller {}".format(request.name))
        param = HandlerParam()
        param.name = request.name
        param.body['metadata'] = {}
        param.body["metadata"]["namespace"] = request.namespace
        param.extra = request
        return run_arktos_workflow(wffactory().k8sEndpointsUpdate(param=param))

    def CreateServiceEndpoint(self, request, context):
        logger.info(
            "Create Service Endpoint from Network Controller {}".format(request.name))
        param = reset_param(HandlerParam())
        param.name = request.name
        param.body['metadata'] = {}
        param.body["metadata"]["namespace"] = request.namespace
        ips = json.loads(request.backend_ips_json)
        ports = json.loads(request.ports_json)
        param.extra = {}
        param.extra["request"] = request
        param.extra["backend_ips"] = ips
        param.extra["ports"] = ports
        for ip in ips:
            logger.info(ip)
        for port in ports:
            logger.info(port)
        return run_arktos_workflow(wffactory().k8sEndpointsUpdate(param=param))

    def CreateArktosNetwork(self, request, context):
        logger.info(
            "Arktos Network create from Network Controller {} -> {}.".format(request.name, request.vpc))
        rc = ReturnCode(
            code=CodeType.OK,
            message="OK"
        )
        vpc = request.vpc
        if request.name == "default" and request.vpc == "system-default-network":
            vpc = OBJ_DEFAULTS.default_ep_vpc
        vpc_opr.store_get(vpc)
        if not vpc_opr.store_get(request.vpc):
            rc.code = CodeType.PERM_ERROR
            rc.message = "ERROR: VPC {} does not exist for arktos network {}.".format(
                request.vpc, request.name)
        else:
            vpc_opr.store.update_arktosnet_vpc(request.name, request.vpc)
        return rc

    def CreateNetworkPolicy(self, request, context):
        logger.info("Creating network policy from Arktos Service {}".format(request.name))
        param = reset_param(HandlerParam())
        param.name = request.name
        param.namespace = request.namespace
        param.body['status'] = {}
        param.body['metadata'] = {}
        param.body['metadata']['namespace'] = request.namespace
        param.body['metadata']['tenant'] = request.tenant
        param.extra = {}
        return run_arktos_workflow(wffactory().k8sNetworkPolicyCreate(param=param))

    def UpdateArktosNetwork(self, request, context):
        return self.CreateArktosNetwork(request, context)

    def ResumeArktosNetowrk(self, request, context):
        return self.CreateArktosNetwork(request, context)

    def ResumePod(self, request, context):
        return self.CreatePod(request, context)

    def ResumeNode(self, request, context):
        return self.CreateNode(request, context)

    def ResumeService(self, request, context):
        return self.CreateService(request, context)

    def ResumeServiceEndpoint(self, request, context):
        return self.CreateServiceEndpoint(request, context)

    def ResumeNetworkPolicy(self, request, context):
        return self.CreateNetworkPolicy(request, context)

    def UpdatePod(self, request, context):
        return self.CreatePod(request, context)

    def UpdateNode(self, request, context):
        return self.CreateNode(request, context)

    def UpdateService(self, request, context):
        return self.CreateService(request, context)

    def UpdateServiceEndpoint(self, request, context):
        return self.CreateServiceEndpoint(request, context)

    def UpdateNetworkPolicy(self, request, context):
        return self.CreateNetworkPolicy(request, context)

    def DeleteNode(self, request, context):
        logger.info(
            "Deleting droplet from Network Controller {}".format(request.name))
        param = reset_param(HandlerParam())
        param.name = request.name
        return run_arktos_workflow(wffactory().DropletDelete(param=param))

    def DeletePod(self, request, context):
        logger.info(
            "Deleting pod from Network Controller {}".format(request.name))
        param = reset_param(HandlerParam())
        param.name = request.name
        param.namespace = request.namespace        
        return run_arktos_workflow(wffactory().k8sPodDelete(param=param))

    def DeleteService(self, request, context):
        logger.info(
            "Deleting servoce from Network Controller {}".format(request.name))
        param = reset_param(HandlerParam())
        param.name = request.name
        param.body['metadata'] = {}
        param.body["metadata"]["namespace"] = request.namespace
        param.body['metadata']['tenant'] = request.tenant
        return run_arktos_workflow(wffactory().k8sServiceDelete(param=param))

    def DeleteNetworkPolicy(self, request, context):
        logger.info(
            "Deleting network policy from Network Controller {}".format(request.name))
        param = reset_param(HandlerParam())
        param.name = request.name
        param.body['metadata'] = {}
        param.body["metadata"]["namespace"] = request.namespace
        param.extra = request.tenant
        return run_arktos_workflow(wffactory().k8sNetworkPolicyDelete(param=param))

class ArktosServiceClient():
    def __init__(self, ip):
        self.channel = grpc.insecure_channel('{}:50052'.format(ip))
        self.stub_builtins = BuiltinsServiceStub(self.channel)

    def CreatePod(self, BuiltinsPodMessage):
        resp = self.stub_builtins.CreatePod(BuiltinsPodMessage)
        return resp

    def CreateService(self, BuiltinsServiceMessage):
        resp = self.stub_builtins.CreateService(BuiltinsServiceMessage)
        return resp

    def CreateNode(self, BuiltinsNodeMessage):
        resp = self.stub_builtins.CreateNode(BuiltinsNodeMessage)
        return resp

    def CreateArktosNetwork(self, BuiltinsArktosMessage):
        resp = self.stub_builtins.CreateArktosNetwork(BuiltinsArktosMessage)
        return resp

    def CreateNetworkPolicy(self, BuiltinsNetworkPolicyMessage):
        resp = self.stub_builtins.CreateNetworkPolicy(BuiltinsNetworkPolicyMessage)
        return resp

    def UpdatePod(self, BuiltinsPodMessage):
        resp = self.stub_builtins.UpdatePod(BuiltinsPodMessage)
        return resp

    def UpdateService(self, BuiltinsServiceMessage):
        resp = self.stub_builtins.UpdateService(BuiltinsServiceMessage)
        return resp

    def UpdateNode(self, BuiltinsNodeMessage):
        resp = self.stub_builtins.UpdateNode(BuiltinsNodeMessage)
        return resp

    def UpdateArktosNetwork(self, BuiltinsArktosMessage):
        resp = self.stub_builtins.UpdateArktosNetwork(BuiltinsArktosMessage)
        return resp

    def UpdateNetworkPolicy(self, BuiltinsNetworkPolicyMessage):
        resp = self.stub_builtins.UpdateNetworkPolicy(BuiltinsNetworkPolicyMessage)
        return resp

    def ResumePod(self, BuiltinsPodMessage):
        resp = self.stub_builtins.ResumePod(BuiltinsPodMessage)
        return resp

    def ResumeService(self, BuiltinsServiceMessage):
        resp = self.stub_builtins.ResumeService(BuiltinsServiceMessage)
        return resp

    def ResumeNode(self, BuiltinsNodeMessage):
        resp = self.stub_builtins.ResumeNode(BuiltinsNodeMessage)
        return resp

    def ResumeArktosNetwork(self, BuiltinsArktosMessage):
        resp = self.stub_builtins.ResumeArktosNetwork(BuiltinsArktosMessage)
        return resp

    def ResumeNetworkPolicy(self, BuiltinsNetworkPolicyMessage):
        resp = self.stub_builtins.ResumeNetworkPolicy(BuiltinsNetworkPolicyMessage)
        return resp

    def DeleteNode(self, BuiltinsNodeMessage):
        resp = self.stub_builtins.DeleteNode(BuiltinsNodeMessage)
        return resp

    def DeleteService(self, BuiltinsServiceMessage):
        resp = self.stub_builtins.DeleteService(BuiltinsServiceMessage)
        return resp

    def DeletePod(self, BuiltinsPodMessage):
        resp = self.stub_builtins.DeletePod(BuiltinsPodMessage)
        return resp

    def DeleteNetworkPolicy(self, BuiltinsNetworkPolicyMessage):
        resp = self.stub_builtins.DeleteNetworkPolicy(BuiltinsNetworkPolicyMessage)
        return resp
