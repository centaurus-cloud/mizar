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

DIR=${1:-.}
MODE=${2:-dev}
DOCKER_ACC=${3:-"localhost:5000"}
YAML_FILE="dev.daemon.deploy.yaml"

if [[ "$MODE" == "user" || "$MODE" == "final" ]]; then
    DOCKER_ACC="mizarnet"
    YAML_FILE="daemon.deploy.yaml"
fi

# Build the daemon image
if [[ "$MODE" == "dev" || "$MODE" == "final" ]]; then
    docker image build -t $DOCKER_ACC/dropletd:latest -f $DIR/etc/docker/daemon.Dockerfile $DIR
    docker image push $DOCKER_ACC/dropletd:latest
fi

# Delete existing deployment and deploy
kubectl delete daemonset.apps/mizar-daemon 2> ${KUBECTL_LOG}
kubectl apply -f $DIR/etc/deploy/$YAML_FILE
