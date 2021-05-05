<!--
SPDX-License-Identifier: MIT
Copyright (c) 2020 The Authors.

Authors: Catherine Lu      <@clu2>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:The above copyright
notice and this permission notice shall be included in all copies or
substantial portions of the Software.THE SOFTWARE IS PROVIDED "AS IS",
WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
THE USE OR OTHER DEALINGS IN THE SOFTWARE.
-->

# Arktos and Mizar Multi Node Installation Guide

## Introduction
This document is intended to capture the installation steps for multi node Arktos platform with Mizar as the underlying network service. 

## Installation Steps
1. Before begin, please complete [Single Node Installation Guide](arktos_mizar_installation_single.md) before proceed this installation guide

For more details on Arktos installation, please refer to [Arktos Setup Guide](https://github.com/centaurus-cloud/arktos/blob/master/docs/setup-guide/multi-node-dev-cluster.md)

If this machine will be used as the master of a multi-node cluster, please set adequate permissive security groups. Fro AWS VM in this lab, we allowed inbound rule of ALL-Traffic 0.0.0.0/0.

2. From the **master node** machine, start Arktos server if haven't already: 

```bash
cd /home/ubuntu/go/src/k8s.io/arktos
./hack/arktos-up.sh
```

In a new terminal window, start Arktos network controller if haven't already: 
```bash
./_output/local/bin/linux/amd64/arktos-network-controller --kubeconfig=/var/run/kubernetes/admin.kubeconfig --kube-apiserver-ip=xxx.xxx.xxx.xxx
```
where the ```kube-apiserver-ip``` is your lab machine's **private ip address**

In a new terminal window, start Mizar if haven't already: 
```bash
cd /home/ubuntu/go/src/k8s.io/mizar
./deploy-mizar.sh
```

3. Prepare worker node lab machine: Open a new terminal window, ssh into your **worker node** lab machine. To add worker nodes, please ensure following worker secret files copied from the master node. From worker node lab machine, run:
```bash
mkdir -p /tmp/arktos
scp <master-node-instance>:/var/run/kubernetes/kubelet.kubeconfig /tmp/arktos/
scp <master-node-instance>:/var/run/kubernetes/client-ca.crt /tmp/arktos/
```
NOTE: you need to re-run the above command once you restart the machine.

Then at worker node, run following commands:
```bash
export KUBELET_IP=<worker-ip>
./hack/arktos-worker-up.sh
```
where, the <worker-ip> is your lab machine's private ip address.

After the script returns, go to master node terminal and run command "[arktos_repo]/cluster/kubectl.sh get nodes", you should see the work node is displayed and its status should be "Ready".
