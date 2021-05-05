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

# Checks for status: Provisioned for given object
function get_status() {
    OBJECT=$1

    kubectl get $OBJECT 2> ${KUBECTL_LOG} | awk '
    NR==1 {
        for (i=1; i<=NF; i++) {
            f[$i] = i
        }
    }
    { print $f["STATUS"] }
    ' | grep Provisioned > /dev/null

    return $?
}

# Checks for status Provisioned of array of objects
function check_ready() {
    objects=("droplets" "vpcs" "subnets" "dividers" "bouncers")
    sum=0
    for i in "${objects[@]}"
    do
        get_status $i
        let sum+=$((sum + $?))
    done
    if [[ $sum == 0 ]]; then
        return 1
    else
        sleep 2
        echo -n "."
        return 0
    fi
}

CWD=$(pwd)
KINDCONF="${HOME}/mizar/build/tests/kind/config"
MIZARCONF="${HOME}/mizar/build/tests/mizarcni.config"
KINDUSERCONFDIR="${HOME}/.kube"
KINDUSERCONF="${KINDUSERCONFDIR}/config"
KUBECTL_LOG="/tmp/${USER}_kubetctl.err"
MODE=${1:-user}
NODES=${2:-3}
timeout=240

if [[ "$MODE" == "dev" ]]; then
    DOCKER_ACC="localhost:5000"
else
    DOCKER_ACC="mizarnet"
fi

# Delete old kind cluser, kind docker network, and images.
kind delete cluster
imgs=$(docker images -a | grep $DOCKER_ACC | awk '{print $3}')
for img in ${imgs[@]}; do
  docker rmi -f $img
done
docker rm -f local-kind-registry 2> /dev/null
docker network rm kind 2> /dev/null

if [[ "$MODE" == "dev" ]]; then
  make clean
  make all
fi

# All interfaces in the network have an MTU of 9000 to
# simulate a real datacenter. Since all container traffic
# goes through the docker bridge, we must ensure the bridge
# interfaces also has the same MTU to prevent ip fragmentation.
docker network create -d bridge \
  --subnet=172.18.0.0/16 \
  --gateway=172.18.0.1 \
  --opt com.docker.network.driver.mtu=9000 \
  kind

docker image build -t $DOCKER_ACC/kindnode:latest -f k8s/kind/Dockerfile .

source k8s/kind/create_cluster.sh $KINDCONF $MODE $NODES

# Install kubectl
which kubectl
if [ $? -ne 0 ]; then
    k8s_ver="$(docker exec -t kind-control-plane kubelet --version | cut -d'v' -f2 | tr -d '\r')-00"
    curl -s https://packages.cloud.google.com/apt/doc/apt-key.gpg | sudo apt-key add -
    echo "deb http://apt.kubernetes.io/ kubernetes-xenial main" | sudo tee /etc/apt/sources.list.d/kubernetes.list
    sudo apt-get update -y
    sudo apt-get install -y kubectl="${k8s_ver}"
fi

api_ip=`docker inspect -f '{{range .NetworkSettings.Networks}}{{.IPAddress}}{{end}}' kind-control-plane`
sed "s/server: https:\/\/127.0.0.1:[[:digit:]]\+/server: https:\/\/$api_ip:6443/" $KINDCONF > $MIZARCONF
ln -snf ${KINDCONF} ${KINDUSERCONF}

source install/create_crds.sh $CWD
source install/create_service_account.sh $CWD

source install/deploy_daemon.sh $CWD $MODE $DOCKER_ACC
source install/deploy_operator.sh $CWD $MODE $DOCKER_ACC
source install/create_testimage.sh $CWD $MODE $DOCKER_ACC

end=$((SECONDS + $timeout))
echo -n "Waiting for cluster to come up."
while [[ $SECONDS -lt $end ]]; do
    check_ready || break
done
echo
if [[ $SECONDS -lt $end ]]; then
    echo "Cluster now ready!"
else
    echo "ERROR: Cluster setup timed out after $timeout seconds!"
    exit 1
fi
