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

KINDCONF=${1:-"${PWD}/build/tests/kind/config"}
MODE=${2:-dev}
NODES=${3:-1}

# create registry container unless it already exists
reg_name='local-kind-registry'
reg_port='5000'
reg_network='kind'
running="$(docker inspect -f '{{.State.Running}}' "${reg_name}" 2>/dev/null || true)"
reg_url=$reg_name
kind_version=$(kind version)
if [ "${running}" != 'true' ]; then
  docker run \
    -d --restart=always -p "${reg_port}:5000" --name "${reg_name}" \
    registry:2
fi

case "${kind_version}" in
  "kind v0.7."* | "kind v0.6."* | "kind v0.5."*)
    reg_url="$(docker inspect -f '{{.NetworkSettings.IPAddress}}' "${reg_name}")"
    reg_network='bridge'
    ;;
esac

if [[ $MODE == "dev" ]]; then
  PATCH="containerdConfigPatches:
- |-
  [plugins.\"io.containerd.grpc.v1.cri\".registry.mirrors.\"localhost:${reg_port}\"]
    endpoint = [\"http://${reg_url}:${reg_port}\"]"
  REPO="localhost:5000"
else
  PATCH=""
  REPO="mizarnet"
fi

NODE_TEMPLATE="  - role: worker
    image: ${REPO}/kindnode:latest
"
FINAL_NODES=""

for ((i=1; i<=NODES; i++));
do
  FINAL_NODES=$FINAL_NODES$NODE_TEMPLATE
done

# create a cluster with the local registry enabled in containerd and n Nodes.
cat <<EOF | kind create cluster --name kind --kubeconfig ${KINDCONF} --config=-
kind: Cluster
apiVersion: kind.x-k8s.io/v1alpha4
networking:
  disableDefaultCNI: true
${PATCH}
nodes:
  - role: control-plane
    image: ${REPO}/kindnode:latest
${FINAL_NODES}
EOF

if [[ $reg_network == "kind" ]]; then
  docker network connect $reg_network "${reg_name}"
  for node in $(kind get nodes); do
    kubectl annotate node "${node}" "kind.x-k8s.io/registry=localhost:${reg_port}";
  done
fi
