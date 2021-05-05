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
chmod 755 setup-machine-arktos.sh
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

5. Build arktos-network-controller (as it is not part of arktos-up.sh yet)

```bash
cd $HOME/go/src/k8s.io/arktos
sudo ./hack/setup-dev-node.sh
make all WHAT=cmd/arktos-network-controller
sudo rm -f /etc/resolv.conf
sudo ln -s /run/systemd/resolve/resolv.conf /etc/resolv.conf
```

6. Update containerd version to v1.4.2

```bash
cd $HOME
wget https://github.com/containerd/containerd/releases/download/v1.4.2/containerd-1.4.2-linux-amd64.tar.gz
cd /usr 
sudo tar xvf $HOME/containerd-1.4.2-linux-amd64.tar.gz
```

To confirm if containerd version is successfully updated, run: 

```bash
❯ containerd --version
containerd github.com/containerd/containerd v1.4.2 b321d358e6eef9c82fa3f3bb8826dca3724c58c6
```

7. Replace the Artos containerd:

```bash
cd $HOME/go/src/k8s.io/arktos
wget -qO- https://github.com/futurewei-cloud/containerd/releases/download/tenant-cni-args/containerd.zip | zcat > /tmp/containerd
sudo chmod +x /tmp/containerd
sudo systemctl stop containerd
sudo mv /usr/bin/containerd /usr/bin/containerd.bak
sudo mv /tmp/containerd /usr/bin/
sudo systemctl restart containerd
sudo systemctl start docker
export CONTAINER_RUNTIME_ENDPOINT="containerRuntime,container,/run/containerd/containerd.sock"
```

To make sure containerd is running as expected, run: 

```bash
sudo systemctl status containerd.service
```

8. Before deploying Mizar, you will need first start up Arktos API server: 

```bash
cd $HOME/go/src/k8s.io/arktos
./hack/arktos-up.sh
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

9. Once you see above text, your arktos server is now running. 
To verify, you can open a new terminal and run ```kubectl get nodes```, you should see a node running with the name starts with "ip" followed by the private ip address of your lab machine. 

You also want make sure the default kubernetes bridge network configuration file is deleted: 

```bash
sudo ls /etc/cni/net.d
sudo rm /etc/cni/net.d/bridge.conf
```

10. Start Arktos network controller. From a new terminal window, run:

```bash
cd $HOME/go/src/k8s.io/arktos
./_output/local/bin/linux/amd64/arktos-network-controller --kubeconfig=/var/run/kubernetes/admin.kubeconfig --kube-apiserver-ip=xxx.xxx.xxx.xxx
```
where the ```kube-apiserver-ip``` is your lab machine's **private ip address**

11. Set Arktos and Mizar alias: 

```bash
alias kubectl='$HOME/go/src/k8s.io/arktos/cluster/kubectl.sh'
alias kubeop="kubectl get pods | grep mizar-operator | awk '{print \$1}' | xargs -i kubectl logs {}"
alias kubed="kubectl get pods | grep mizar-daemon | awk '{print \$1}' | xargs -i kubectl logs {}"
```

12. Deploy Mizar. Open a new terminal window, and run: 
```bash
cd $HOME/mizar
./deploy-mizar.sh
```

Please refer to [Running Mizar Management Plane](https://mizar.readthedocs.io/en/latest/user/getting_started/#running-mizar-management-plane) for how to inspect resources. 

13. Restart containerd 

```bash
sudo systemctl restart containerd.service
```

To make sure containerd is running as expected, run: 

```bash
sudo systemctl status containerd.service
```

## Test

To test your environment, you can first create an arktos network object ```my-net-mizar-1```. Then create a pod in ```my-net-mizar-1```:

```bash
cd $HOME/go/src/k8s.io/mizar

kubectl create -f mizar/obj/tests/test_arktos_net.yaml

kubectl create -f mizar/obj/tests/test_pod_arktos_vpc1.yaml

```

