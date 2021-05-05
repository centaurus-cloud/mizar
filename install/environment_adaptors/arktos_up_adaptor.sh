#!/bin/bash

# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Authors.

# Authors: Hong Chang        <@Hong-Chang>

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

source install/common.sh

function environment_adaptor:prepare_binary {
    # Current for arktos-up env, we use images from mizarnet instead of localhost:5000
    # So skip invoking common:build_docker_images
    # common:build_docker_images 
    :
}

function environment_adaptor:deploy_mizar {
    local is_mizar_production=${1:-0}

    local cwd=$(pwd)
    source install/create_crds.sh $cwd

    # Indicate the system is from Arktos
    kubectl create configmap system-source --namespace=kube-system --from-literal=name=arktos --from-literal=company=futurewei

    # Deploy daemon first, then deploy operator after daemon pod is running. We hold operator, wait until daemon is running. Mizar won't work correctly if directly deploying operator without waiting for daemon. 
    kubectl apply -f etc/deploy/deploy.daemon.yaml
    sleep 5 # Wait when daemon pod is being created
    echo "Waiting for daemon pod running. It may cost up to 30 minutes because it needs to setup pip3 modules such as grpcio which needs quite some time for the first time."
    kubectl wait --for=condition=Ready pod -l job=mizar-daemon --timeout=30m
    
    kubectl apply -f etc/deploy/deploy.operator.yaml

    echo "Waiting for Mizar to be up and running."
    local timeout=60
    common:execute_and_retry "common:check_mizar_ready" 1 "Mizar is now ready!" "ERROR: Mizar setup timed out after $timeout seconds!" $timeout 1
   }
