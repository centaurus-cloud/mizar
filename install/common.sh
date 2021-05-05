#!/bin/bash

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

function delete_pods {
    NAME=$1
    TYPE=$2
    kubectl delete $TYPE.apps/$NAME 2> ${KUBECTL_LOG}
    echo -n "Waiting for ${NAME} pods to terminate."
    kubectl get pods | grep $NAME > /dev/null
    while [[ $? -eq 0 ]]; do
        echo -n "."
        sleep 1
        kubectl get pods | grep $NAME > /dev/null
    done
    echo
}

function common:check_cluster_ready {
    local show_cluster_status=${1:-1}

    local function_name="common:check_cluster_ready"
    echo "[$function_name] Checking cluster readyness by getting node status."
    if [[ $show_cluster_status == 1 ]]; then
        kubectl cluster-info
    fi

    local cluster_status=`kubectl cluster-info | grep Kubernetes | grep is | grep running`
    if [[ -z "$cluster_status" ]]; then
        return 0
    else
        return 1
    fi
}

# Checks for status: Provisioned for given object
function common:get_object_status {
    local object=$1

    kubectl get $object 2> ${KUBECTL_LOG} | awk '
    NR==1 {
        for (i=1; i<=NF; i++) {
            f[$i] = i
        }
    }
    { print $f["STATUS"] }
    ' | grep Provisioned > /dev/null

    return $?
}

# Checks for mizar status by provisioned objects array
function common:check_mizar_ready {
    local objects=("droplets" "vpcs" "subnets" "dividers" "bouncers")
    local sum=0
    for i in "${objects[@]}"
    do
        common:get_object_status $i
        let sum+=$((sum + $?))
    done
    if [[ $sum == 0 ]]; then
        echo "[common:check_mizar_ready] Mizar is up and running."
        return 1
    else
        return 0
    fi
}



# Polymorphism implementation: There are multiple environment adaptors. Each one is implementation differently based on environment, but share same function.
function common:source_environment_adaptor {    
    local mizar_environment=${1:-"k8s_kind"}
    adaptor_file_name="install/environment_adaptors/${mizar_environment}_adaptor.sh"
    if [[ -f $adaptor_file_name ]]; then
        source $adaptor_file_name
    else
        source "install/environment_adaptors/default_adaptor.sh"
    fi
}

# Keey retry function execution until succeed or timeout
# param1: function execution script. For example, "func_one param_one". Function needs to have return value
# param2: expected returned function result
# param3: succeed message
# param4: failed message
# param5: timeout in seconds
# param5: flag indicating whether exit program when failed
# return: 1 as succeed, 0 as failing then timeout
function common:execute_and_retry {
    local function_script=$1
    local expected_result=$2
    local succeed_message=${3:-""}
    local failed_message=${4:-""}
    local timeout_in_seconds=${5:-60}
    local exit_when_timeout=${6:-1}

    local start_time=$(date +%s)
    while true; do
        ($function_script) > /dev/null
        local actual_result=$?
        if [[ $actual_result == $expected_result ]]; then
            break;
        fi
        sleep 2
        echo -n "."
        local elapsed=$(($(date +%s) - ${start_time}))
        if [[ ${elapsed} -gt $timeout_in_seconds ]]; then
            if [[ ! -z $failed_message ]]; then
                echo $failed_message
            fi
            if [[ $exit_when_timeout == 1 ]]; then
                exit 1
            else
                return 0
            fi
        fi
    done
    if [[ ! -z $succeed_message ]]; then
        echo $succeed_message
    fi
    return 1
}

function common:build_docker_images {
    local docker_account="localhost:5000"
    local reg_name='local-registry'

    local registry_running="$(docker inspect -f '{{.State.Running}}' "${reg_name}" 2>/dev/null || true)"
    if [ "${registry_running}" != "true" ]; then
        docker run -d --restart=always -p "5000:5000" --name "${reg_name}" registry:2
    fi

    docker image build -t $docker_account/mizar:latest -f etc/docker/mizar.Dockerfile .
    docker image build -t $docker_account/dropletd:latest -f etc/docker/daemon.Dockerfile .
    docker image build -t $docker_account/endpointopr:latest -f etc/docker/operator.Dockerfile .
    docker image build -t $docker_account/testpod:latest -f etc/docker/test.Dockerfile .
    docker image build -t $docker_account/mizarcni:latest -f etc/docker/mizarcni.Dockerfile .
}

function common:check_pod_by_image {
    local image_name=$1
    local pod_name=$(kubectl get pods -l run=$image_name | grep "$image_name" | awk '{print $1}')
    if [[ -z $pod_name ]]; then
        return 0
    else
        return 1
    fi
}

function common:check_pod_running_in_mizar {
    local image_name="pod-test"
    
    kubectl run $image_name --image=localhost:5000/testpod
    sleep 2
    kubectl wait --for=condition=Ready pod -l run=$image_name --timeout=60s

    local pod_name=$(kubectl get pods -l run=$image_name -o wide | grep " 10.0." | awk '{print $1}')
    kubectl delete deploy $image_name
    common:execute_and_retry "common:check_pod_by_image $image_name" 0 "" "" 60 0
    if [[ -z $pod_name ]]; then
        return 0
    else
        return 1
    fi
}
