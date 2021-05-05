<!--
SPDX-License-Identifier: MIT
Copyright (c) 2020 The Authors.

Authors: Sherif Abdelwahab <@zasherif>
         Phu Tran          <@phudtran>

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

# Basic Usage

To start running Mizar, first you will need a Kubernetes cluster. Running Mizar will  install experimental features on your cluster. **It is recommended that you use a test cluster to run Mizar.**

## Prerequisites

### Kind (Kubernetes in Docker)

**Note**: As of writing this, there is an error in the installation instructions for Kind.
The commands below can be used to download and install the latest release binary for Kind.

MacOS/Linux
```
ver=$(curl -s https://api.github.com/repos/kubernetes-sigs/kind/releases/latest | grep -oP '"tag_name": "\K(.*)(?=")')
curl -Lo kind "https://github.com/kubernetes-sigs/kind/releases/download/$ver/kind-$(uname)-amd64"
chmod +x kind
mv kind /usr/local/bin
```

The recommended way to try out Mizar is with Kind.
Kind can be used to run a multi-node Kubernetes cluster with Docker containers locally on your machine.
You can find [instructions for installing Kind on it's official site here.](https://kind.sigs.k8s.io/docs/user/quick-start/)

* Enter the mizar directory and run the ```bootstrap.sh``` script.
    * This script will install the neccessary components to compile Mizar, and run it's unit tests. These include
        * [Clang-7](https://clang.llvm.org) (For code compilation)
        * [Llvm-7](https://llvm.org) (For code compilation)
         * [Cmocka](https://cmocka.org) (For unit testing)
        * [Build Essentials](https://packages.ubuntu.com/xenial/build-essential) (For code compilation)
        * [Python](https://www.python.org) (For running the management plane and tests)
        * [Docker](https://www.docker.com) (For management plane and tests)
    * There is also an optional kernel update included in the script if you wish to update to a compatible version

#### New Kind Cluster
**Note**: Before proceeding with the setup script below, please make sure you have [kind](https://kind.sigs.k8s.io/docs/user/quick-start/) and [kubectl](https://kubernetes.io/docs/tasks/tools/install-kubectl/) installed.
Validate your kind and kubectl installations by running:

```
$ kind --version

$ kubectl version --client
```

If you are testing out Mizar with Kind, a script has been included to setup a local Kubernetes cluster, and install all of the components needed to run Mizar.
Simply run the script below in the Mizar directory.

```
$ ./kind-setup.sh
```

This script does the following:

* Create a multi-node local Kubernetes cluster with Kind.
* Build the Kind-Node, Mizar-Daemon, and Mizar-Operator docker images
* Apply all of the Mizar [CRDs](https://kubernetes.io/docs/concepts/extend-kubernetes/api-extension/custom-resources/).
* Deploy the Mizar [Operators](https://kubernetes.io/docs/concepts/extend-kubernetes/operator/).
* Deploy the Mizar Daemon
* Install the Mizar CNI Plugin

### Install Mizar as Kubernetes CNI plugin
Mizar can be installed as network plugin to any Kubernetes cluster. Below has been verified to work for Ubuntu 20.04.1 LTS (on AWS EC2 VM).

If a Kuberneetes cluster has kube-proxy daemonSet running, the simple one line would have Mizar installed:
```bash
kubectl apply -f https://raw.githubusercontent.com/CentaurusInfra/mizar/dev-next/etc/deploy/deploy.mizar.components.yaml
```

If the cluster has no kube-proxy (e.g. cluster being started using kubeadm init --skip-phases=addon/kube-proxy), you need to run below first:
```bash
kubectl create configmap mizar-k8s-config --from-literal=k8sapihost="<kube-api-server-ip>" --from-literal=k8sapiport="<kube-api-server-port>"
```
where the kube-api-server-ip and kube-api-server-port can be identified by ```kubectl get endpoint kubernetes```,
then run following command to have Mizar plugin installed
```bash
kubectl apply -f https://raw.githubusercontent.com/CentaurusInfra/mizar/dev-next/etc/deploy/deploy.mizar.components.direct-api-access.yaml
```

### Linux Kernel Update

For TCP to function properly, you will need to update your Kernel version to at least 5.6-rc2 on every node. A script, ```kernel_update.sh``` is provided in the Mizar repo to download and update your machine's kernel if you do not wish to build the kernel source code yourself.

## Running Mizar Management Plane

When a cluster is brought up with Mizar, a default VPC and default Network are created. We will call these **vpc0** and **net0**. The default VPC will have an initial default number of Dividers provisioned. Similarly the default Network will also have an initial default number of Bouncers provisioned. These resources will automatically scale in and out depending on the load of the cluster. A user may also manually provision more Bouncers and Dividers at will to fit their needs.

### Inspecting the resources.

There are two states for the resources listed below. *Init* and *Provisioned*. When a resource is initially brought online, its state is set to *Init*. Once it has entered the *Provisioned* state, the resource is officially up and ready for use.

You can inspect the following resources by running the commands below with ```kubectl```.


 * Get all VPCs in the cluster

```
$ kubectl get vpcs
```

 * Get all Networks in the cluster

```
$ kubectl get subnets
```

 * Get all Dividers in the cluster

```
$ kubectl get dividers
```

 * Get all Bouncers in the cluster

```
$ kubectl get bouncers
```

 * Get all endpoints in the cluster

```
$ kubectl get endpoints
```

For futhure information on the operations taking place, please look at the logs for the Mizar-Operator, and Mizar-Daemon pods with kubectl.


### Creating an endpoint

Now that the steps above have been completed, simply create a deployment of any kind to test out the network functionality of Mizar.
A sample deployment with nginx has been included below.

```
apiVersion: apps/v1
kind: Deployment
metadata:
  name: nginx-deployment
spec:
  selector:
    matchLabels:
      app: nginx
  replicas: 10
  template:
    metadata:
      labels:
        app: nginx
    spec:
      containers:
        - name: nginx
          image: nginx:1.7.9
          ports:
            - containerPort: 80
```
Note: To deploy, simply copy this to a file and run
```
$ kubectl create -f file_name_here.yaml
```

With 10 replicas as shown in the example, we have Mizar 10 simple endpoints. These endpoints will be a part of the default Network **net0** which itself is part of the default VPC **vpc0**.

Try out your favorite network test to see the capabilities of Mizar!

### Creating a new VPC

If a user wishes to create a new VPC instead of using the initially provided default one, they just need to deploy a yaml file with their desired VPC specifications. A sample has been included below.

```
apiVersion: mizar.com/v1
kind: Vpc
metadata:
  name: vpc2
spec:
  vni: "1"
  ip: "20.0.0.0"
  prefix: "16"
  dividers: 10
  status: "Init"
```
Note: To deploy, simply copy this to a file and run
```
$ kubectl create -f file_name_here.yaml
```

This yaml will create a VPC with CIDR range 20.0.0.0/16, a VNI of 1, and 10 initial Dividers.

### Creating a new Network

If a user wishes to create a new Network instead of using the initially provided default one, they just need to deploy a yaml file with their desired Network specifications. A sample has been included below.

```
apiVersion: mizar.com/v1
kind: Net
metadata:
  name: net2
spec:
  vni: "1"
  ip: "20.0.20.0"
  prefix: "24"
  bouncers: 10
  vpc: "vpc2"
  status: "Init"
```
Note: To deploy, simply copy this to a file and run
```
$ kubectl create -f file_name_here.yaml
```

This yaml will create a Network with CIDR range 20.0.20.0/24,a VNI of 1, and 10 initial Bouncers. Notice that the Network's VNI is 1. This Network belongs to the VPC with VNI 1. Thus, its CIDR range is also a subset of the VPC with VNI 1.
