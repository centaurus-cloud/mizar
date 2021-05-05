#!/usr/bin/python3

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

import subprocess
import logging
import time
import asyncio
import os.path
import time
from kopf import cli
from mizar.common.wf_param import *
from mizar.dp.mizar.workflows.vpcs.triggers import *
from mizar.dp.mizar.workflows.nets.triggers import *
from mizar.dp.mizar.workflows.dividers.triggers import *
from mizar.dp.mizar.workflows.bouncers.triggers import *
from mizar.dp.mizar.workflows.droplets.triggers import *
from mizar.dp.mizar.workflows.endpoints.triggers import *
from mizar.dp.mizar.workflows.builtins.services.triggers import *
from mizar.dp.mizar.workflows.builtins.nodes.triggers import *
from mizar.dp.mizar.workflows.builtins.pods.triggers import *
from mizar.dp.mizar.workflows.builtins.namespaces.triggers import *
from mizar.dp.mizar.workflows.builtins.networkpolicies.triggers import *
import grpc
import threading
from google.protobuf import empty_pb2
from concurrent import futures
from mizar.proto import builtins_pb2_grpc as builtins_pb2_grpc
from mizar.arktos.arktos_service import ArktosService
from kubernetes import client, config
import socket
from mizar.common.constants import *

logger = logging.getLogger()
LOCK: asyncio.Lock
POOL_WORKERS = 10


@kopf.on.startup()
async def on_startup(logger, **kwargs):
    start_time = time.time()
    global LOCK
    LOCK = asyncio.Lock()
    param = HandlerParam()
    config.load_incluster_config()
    sched = 'luigid --background --port 8082 --pidfile /var/run/luigi/luigi.pid --logdir /var/log/luigi --state-path /var/lib/luigi/luigi.state'
    subprocess.call(sched, shell=True)
    while not os.path.exists("/var/run/luigi/luigi.pid"):
        pass
    with open('/var/run/luigi/luigi.pid', 'r') as f:
        pid = f.read()
    # Wait for daemon to run
    while not os.path.exists("/proc/{}".format(pid)):
        pass
    logger.info("Running luigid central scheduler pid={}!".format(pid))

    threading.Thread(target=grpc_server).start()
    create_config_map()
    configmap = read_config_map()
    if configmap:
        if read_config_map().data["name"] == "arktos":
            logger.info("Found config map, disabling builtin kopf triggers!")
            COMPUTE_PROVIDER.k8s = False
    start_time = time.time()

    run_workflow(wffactory().DropletOperatorStart(param=param))
    run_workflow(wffactory().DividerOperatorStart(param=param))

    run_workflow(wffactory().VpcOperatorStart(param=param))

    run_workflow(wffactory().BouncerOperatorStart(param=param))

    run_workflow(wffactory().NetOperatorStart(param=param))
    run_workflow(wffactory().EndpointOperatorStart(param=param))

    logger.info("Bootstrap time:  %s seconds ---" % (time.time() - start_time))


def grpc_server():
    server = grpc.server(futures.ThreadPoolExecutor(max_workers=POOL_WORKERS))

    builtins_pb2_grpc.add_BuiltinsServiceServicer_to_server(
        ArktosService(), server
    )
    server.add_insecure_port('[::]:50052')
    server.start()
    logger.info("Running gRPC server for Network Controller!")
    server.wait_for_termination()


def create_config_map():
    metadata = metadata = client.V1ObjectMeta(
        name="mizar-grpc-service",
        namespace="default",
        labels=dict(service="mizar")
    )
    data = dict(host=socket.gethostbyname(socket.gethostname()))
    configmap = client.V1ConfigMap(
        api_version="v1",
        kind="ConfigMap",
        data=data,
        metadata=metadata
    )
    kube_create_config_map(client.CoreV1Api(), "default", configmap)


def read_config_map():
    return kube_read_config_map(client.CoreV1Api(), "system-source", "kube-system")
