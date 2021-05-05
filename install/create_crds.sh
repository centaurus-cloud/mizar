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

# Create the CRDs
kubectl delete bouncers.mizar.com --all 2> ${KUBECTL_LOG}
kubectl delete dividers.mizar.com --all 2> ${KUBECTL_LOG}
kubectl delete droplets.mizar.com --all 2> ${KUBECTL_LOG}
kubectl delete endpoints.mizar.com --all 2> ${KUBECTL_LOG}
kubectl delete subnets.mizar.com --all 2> ${KUBECTL_LOG}
kubectl delete vpcs.mizar.com --all 2> ${KUBECTL_LOG}

kubectl apply -f $DIR/etc/crds/bouncers.crd.yaml
kubectl apply -f $DIR/etc/crds/dividers.crd.yaml
kubectl apply -f $DIR/etc/crds/droplets.crd.yaml
kubectl apply -f $DIR/etc/crds/endpoints.crd.yaml
kubectl apply -f $DIR/etc/crds/subnets.crd.yaml
kubectl apply -f $DIR/etc/crds/vpcs.crd.yaml
