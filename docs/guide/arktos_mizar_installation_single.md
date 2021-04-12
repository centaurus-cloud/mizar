<!--
SPDX-License-Identifier: MIT
Copyright (c) 2020 The Authors.

Authors: Catherine Lu      <@clu2>
         Hongwei Chen      <@hong.chen>
         Hong Chang        <@hchang>

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

# Arktos and Mizar Single Node Installation Guide

## Introduction

This document is intended for new users to install Arktos platform with Mizar as the underlying network technology. 

For more details on Arktos installation, please refer to [this link](https://github.com/centaurus-cloud/arktos/blob/master/docs/setup-guide/arktos-enforces-network-feature.md)

## Installation Steps
1. Prepare lab machine, the prefered OS is **Ubuntu 18.04**. If you are using AWS, the recommanded instance size is ```t2.2xlarge``` and the storage size is ```128GB``` or more 

```bash
cd
git clone https://github.com/centaurus-cloud/mizar.git
cd mizar
./setup-machine-arktos.sh
```
The lab machine will be rebooted once above script is completed, you will be automatically logged out of the lab machine. 

2. Log onto your lab machine, then run ```bootstrap.sh``` script from the mizar project folder to bootstrap your lab machine. 

3. Once bootstrap is completed, you can then compile mizar. Make sure run these in ```sudo``` mode: 

```bash
cd ~/mizar
sudo su
make
```

4. Modify your lab machine's network configurations: 

please ensure the hostname and its ip address in ```/etc/hosts```. For instance, if the hostname is ip-172-31-41-177, ip address is 172.31.41.177:
```text
127.0.0.1 localhost
172.31.41.177 ip-172-31-41-177
```

5. Prepare machine for arktos

```bash
sudo rm -f /etc/resolv.conf
sudo ln -s /run/systemd/resolve/resolv.conf /etc/resolv.conf
cd $HOME/go/src/k8s.io/arktos
./hack/setup-dev-node.sh
```

6. Replace the Artos containerd and start arktos API server:

```bash
cd $HOME/mizar
./replace-containerd_start_arktos_api.sh
```

Then wait till you see: 

```
To start using your cluster, you can open up another terminal/tab and run:

  export KUBECONFIG=/var/run/kubernetes/admin.kubeconfig
Or
  export KUBECONFIG=/var/run/kubernetes/adminN(N=0,1,...).kubeconfig

  cluster/kubectl.sh

Alternatively, you can write to the default kubeconfig:

  export KUBERNETES_PROVIDER=local

  cluster/kubectl.sh config set-cluster local --server=https://ip-172-31-16-157:6443 --certificate-authority=/var/run/kubernetes/server-ca.crt
  cluster/kubectl.sh config set-credentials myself --client-key=/var/run/kubernetes/client-admin.key --client-certificate=/var/run/kubernetes/client-admin.crt
  cluster/kubectl.sh config set-context local --cluster=local --user=myself
  cluster/kubectl.sh config use-context local
  cluster/kubectl.sh
```

If you continuously to see 
```
Waiting for node ready at api server
Waiting for node ready at api server
Waiting for node ready at api server
Waiting for node ready at api server
```
Or any another arktos-up.sh related error message, try re-run ```replace-containerd_start_arktos_api.sh``` script untill you see cluster/kubectl.sh

7. Deploy Mizar. Open a new terminal window, and run: 
```bash
cd $HOME/mizar
./deploy-mizar.sh
```

Please refer to [Running Mizar Management Plane](https://mizar.readthedocs.io/en/latest/user/getting_started/#running-mizar-management-plane) for how to inspect resources. 

8. Start Arktos network controller. From a new terminal window, run:

```bash
cd $HOME/go/src/k8s.io/arktos
make all WHAT=cmd/arktos-network-controller
./_output/local/bin/linux/amd64/arktos-network-controller --kubeconfig=/var/run/kubernetes/admin.kubeconfig --kube-apiserver-ip=xxx.xxx.xxx.xxx
```
where the ```kube-apiserver-ip``` is your lab machine's **private ip address**

## Reboot or Restart machine
If you have stoped arktos API server, and would like to restart the environment. You will need re-run step 6 through 8 to restart the mizar-arktos cluster. 

## Test

To test your environment, you can first create an arktos network object ```my-net-mizar-1```. Then create a pod in ```my-net-mizar-1```:

```bash
cd $HOME/go/src/k8s.io/mizar

kubectl create -f mizar/obj/tests/test_arktos_net.yaml

kubectl create -f mizar/obj/tests/test_pod_arktos_vpc1.yaml

```

