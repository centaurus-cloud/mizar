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
    :
}

function environment_adaptor:deploy_mizar {
    local is_mizar_production=${1:-0}

    local cwd=$(pwd)
    source install/create_crds.sh $cwd

    kubectl apply -f etc/deploy/deploy.initialize.yaml
    sleep 5 # Wait when initialize pod is being created
    kubectl wait --for=condition=Ready pod -l job=mizar-initialize --timeout=30s

    local process_ids=""
    for pod_name in $(kubectl get pods -l job=mizar-initialize | grep 'Running' | awk '{print $1}'); 
    do 
        deploy_for_pod_initialize $pod_name &
        local process_ids="$process_ids $!"
    done
    for process_id in $process_ids; do
        echo "waiting for $process_id"
        wait $process_id
    done
    
    kubectl apply -f /home/ubuntu/mizar/etc/deploy/deploy.daemon.dev.yaml
    sleep 5 # Wait when initialize pod is being created
    kubectl wait --for=condition=Ready pod -l job=mizar-daemon --timeout=30s

    local process_ids=""
    for pod_name in $(kubectl get pods -l job=mizar-daemon | grep 'Running' | awk '{print $1}'); 
    do 
        deploy_for_python_pod $pod_name &
        local process_ids="$process_ids $!"
    done
    for process_id in $process_ids; do
        echo "waiting for $process_id"
        wait $process_id
    done

    kubectl apply -f /home/ubuntu/mizar/etc/deploy/deploy.operator.dev.yaml
    sleep 5 # Wait when initialize pod is being created
    kubectl wait --for=condition=Ready pod -l app=mizar-operator --timeout=30s

    local pod_name=$(kubectl get pods -l app=mizar-operator | grep 'Running' | awk '{print $1}')
    deploy_for_python_pod $pod_name
    kubectl exec $pod_name -- ln -snf /var/mizar/build/bin /trn_bin
    kubectl cp ./etc/luigi.cfg $pod_name:/etc/luigi/luigi.cfg

    echo "Waiting for Mizar to be up and running."
    local timeout=60
    common:execute_and_retry "common:check_mizar_ready" 1 "" "ERROR: Mizar setup timed out after $timeout seconds!" $timeout 1

}

function deploy_for_python_pod {
    local pod_name=$1

    kubectl exec $pod_name -- apt-get update -y
    kubectl exec $pod_name -- apt-get install net-tools
    kubectl exec $pod_name -- pip3 install PyYAML
    kubectl exec $pod_name -- pip3 install kopf
    kubectl exec $pod_name -- pip3 install netaddr
    kubectl exec $pod_name -- pip3 install ipaddress
    kubectl exec $pod_name -- pip3 install pyroute2
    kubectl exec $pod_name -- pip3 install rpyc
    kubectl exec $pod_name -- pip3 install kubernetes==11.0.0
    kubectl exec $pod_name -- pip3 install luigi==2.8.12
    kubectl exec $pod_name -- pip3 install grpcio
    kubectl exec $pod_name -- pip3 install protobuf
    kubectl exec $pod_name -- pip3 install fs
    kubectl exec $pod_name -- mkdir -p /var/run/luigi
    kubectl exec $pod_name -- mkdir -p /var/log/luigi
    kubectl exec $pod_name -- mkdir -p /var/lib/luigi
    kubectl exec $pod_name -- mkdir -p /etc/luigi

    kubectl cp ../mizar $pod_name:/var/mizar/

    kubectl exec $pod_name -- pip3 install /var/mizar/
}

function deploy_for_pod_initialize {
    local pod_name=$1
    deploy_binaries_to_remote $pod_name
    install_dependencies_to_remote $pod_name
}

function deploy_binaries_to_remote {
    local pod_name=$1
    kubectl cp ../mizar $pod_name:/home
}

function install_dependencies_to_remote {
    local pod_name=$1

    kubectl exec $pod_name -- nsenter -t 1 -m -u -n -i apt-get update -y
    kubectl exec $pod_name -- nsenter -t 1 -m -u -n -i apt-get install -y sudo rpcbind rsyslog libelf-dev iproute2 net-tools iputils-ping ethtool curl python3 python3-pip
    kubectl exec $pod_name -- nsenter -t 1 -m -u -n -i mkdir -p /opt/cni/bin
    #kubectl exec $pod_name -- nsenter -t 1 -m -u -n -i rm -rf /etc/cni/net.d
    kubectl exec $pod_name -- nsenter -t 1 -m -u -n -i mkdir -p /etc/cni/net.d
    kubectl exec $pod_name -- nsenter -t 1 -m -u -n -i pip3 install --upgrade protobuf
    kubectl exec $pod_name -- nsenter -t 1 -m -u -n -i pip3 install --ignore-installed /var/mizar/
    kubectl exec $pod_name -- nsenter -t 1 -m -u -n -i ln -snf /sys/fs/bpf /bpffs
    kubectl exec $pod_name -- nsenter -t 1 -m -u -n -i ln -snf /var/mizar/build/bin /trn_bin
    kubectl exec $pod_name -- nsenter -t 1 -m -u -n -i ln -snf /var/mizar/build/xdp /trn_xdp
    kubectl exec $pod_name -- nsenter -t 1 -m -u -n -i ln -snf /var/mizar/etc/cni/10-mizarcni.conf /etc/cni/net.d/00-mizarcni.conf
    kubectl exec $pod_name -- nsenter -t 1 -m -u -n -i ln -snf /var/mizar/mizar/cni.py /opt/cni/bin/mizarcni
    kubectl exec $pod_name -- nsenter -t 1 -m -u -n -i ln -snf /var/mizar/build/tests/mizarcni.config /etc/mizarcni.config
}
