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

# Arktos Network and Mizar Integration

## Introduction

This document is designated for Arktos service using Mizar as its underline network system. In this document, you will find the design detail on how to integration Arktos multi-tenancy network object with Mizar as its network platform, so that Arktos can fully utilize network objects offered in Mizar, including vpcs, nets, services, pods, bouncers and dividers. 

## Arktos Network Object Overview: 
The Arktos network object contains its type and type-specific configurations, which translates into spec fields type and vpcID:
```yaml
apiVersion: arktos.futurewei.com/v1
kind: Network
metadata:
  name: vpc-1
spec:
  type: mizar
  vpcID: vpc-1a2b3c4d
```
Both ```type``` and ```vpcID``` are required fields, and we also need ensure that the vpc listed in ```vpcID``` has **at least one** subnet. ```type``` field identifies the underline network service (which in our case is mizar). In other words, Mizar only reacts to Network creation when ```type``` is "mizar". ```vpcID``` field specifies an existing vpc object name in Mizar. 

The Arktos network object and VPC have **many to one relationship**, where multiple Arktos network objects can reference the same VPC network. 

## Pod Definition

When a pod is associated with a Arktos network object, it needs to set its "network" in labels:
```yaml
apiVersion: v1
kind: Pod
metadata:
  name: nginx
  labels:
    arktos.futurewei.com/network: vpc-1
spec:
  containers:
  - name: nginx
    image: nginx
    ports:
      - containerPort: 443
```
User also has the option to specify desired subnet and ip information. These network specification should be included in annotations section as ```arktos.futurewei.com/nic```

```yaml

apiVersion: v1
kind: Pod
metadata:
  name: nginx
  labels:
    arktos.futurewei.com/network: vpc-1
  annotations:
    arktos.futurewei.com/nic: {"name": "eth0", "ip": "10.10.1.12", "subnet": "net1"}
spec:
  containers:
  - name: nginx
    image: nginx
    ports:
      - containerPort: 443

```

Please note that annotation ```arktos.futurewei.com/nic``` is optional. In case where any of the three fields is missing, Mizar needs to create a pod with remaining information. For example, definition might only has  ```ip``` field specified; in this case, Mizar needs to figure out which subnet this ip falls into, and then creates the pod into the correct subnet. 
If both ip and subnet are missing, Mizar will choose the first subnet within ```vpc-1``` to launch the Pod and assign an ip address to that Pod. 
If subnet is the only specified information, Mizar will launch the Pod in ```net1``` and assign an ip address to the Pod. Please note that Mizar only supports Pod creation into existing subnet. In other words, if ```net1``` listed in definition does not exist, Mizar will throw an error. 

## Network Object Association

In Mizar, the association of all network objects are stored as key/value sets in Mizar’s object store. For instance, 

The Arktos tenant is associated with Arktos network objects as: 
	```self.store.arktosnets_tenant_store[tenant.name] = {}```
    ```self.store.arktosnets_tenant_store[tenant.name][arktosnet.name] = arktosnet_object```

The Arktos network object is associated with Mizar vpc as: 
	```self.store.vpcs_arktosnet_store[arktosnet.name] = {}```
    ```self.store.vpcs_arktosnet_store[arktosnet.name][vpc.name] = vpc_object```

The Arktos network object is associated with services in similar ways: 
    ```self.store.services_arktosnet_store[arktosnet.name] = {}```
    ```self.store.services_arktosnet_store[arktosnet.name][service.name] = service_object```

In addition, Mizar also provides key/value sets to store relationships between pods, subnets/nets and vpcs: 

* ```nets_vpc_store```
* ```pods_net_store```

## Arktos Network Object Creation 

For now, Mizar will only support VPC object and Arktos network object association, meaning Arktos must pass in a vpc id that already exist. 

Mizar has operator that is in charge of managing lifecycle of Arktos network object. 

When creating a mizar type network: 
```yaml
apiVersion: arktos.futurewei.com/v1
kind: Network
metadata:
  name: vpc-1
spec:
  type: mizar
  vpcID: vpc-1a2b3c4d
```
* When customer initiates an Arktos Network creation, Mizar will first create an Arktos network object named ```vpc-1```, then it search for an existing VPC named ```vpc-1a2b3c4d```. Once the specified vpcID is found, it then associates the vpc object ```vpc-1a2b3c4d``` with the Arktos network object ```vpc-1```:   
    * ```self.store.vpcs_arktosnet_store[vpc-1][vpc-1a2b3c4d] = vpc-1a2b3c4d object```
* Then, mizar retrieves metadata information: ```metadata.tenant = customerA```, and map ```metadata.tenant``` with arktos network object: ```self.store.arktosnets_tenant_store[customerA][vpc-1] = arktosnet_object```

## Pod Creation
 Sample Pod definition:
```yaml
apiVersion: v1
kind: Pod
metadata:
  name: nginx
  labels:
    arktos.futurewei.com/network: vpc-1
  annotations:
    arktos.futurewei.com/nic: {"name": "eth0", "ip": "10.10.1.12", "subnet": "net1"}

spec:
  containers:
  - name: nginx
    image: nginx
    ports:
      - containerPort: 443
```
Operator kicks off creation process:
* List Kubernetes cluster object / Arktos newtork object
* Retrieve metadata information: ```metadata.tenant```, ```metadata.namespace```, ```metadata.labels``` and ```metadata.annotations```: 
    * ```tenant```: customerA
    * ```namespace```: internal
    * ```arktos.futurewei.com/network```: vpc-1
    * ```arktos.futurewei.com/nic```: {"name": "eth0", "ip": "10.10.1.12", "subnet": "net1"}
* Set pod name as {```name```}-{```namespace```}-{```tenant```}
* Find Arktos network object that is listed in ```arktos.futurewei.com/network```
* Find subnet and ip information from ```arktos.futurewei.com/nic```, and choose the correct subnet to place the new pod. 
  * If subnet is the only specified information, Mizar will launch the Pod in ```net1``` and assign an ip address to the Pod. Please note that Mizar only supports Pod creation in an existing subnet. In other words, if ```net1``` listed in definition does not exist, Mizar will throw an error. 
  * If only  ```ip``` is specified, Mizar needs to figure out which subnet this ip falls into, and then creates the pod into the correct subnet. 
  * If both ip and subnet are missing, Mizar will choose the first subnet within ```vpc-1``` to launch the Pod and assign an ip address to that Pod. 
* Update object store:
    * map subnet object (which is equivalent to net object in Mizar) with pod: ```self.store.pods_net_store[net1][nginx-internal-customerA] = pod object```
* After Pod creation is completed, update Pod object with new annotations: ```mizar.futurewei.com/network_user_input``` and ```arktos.futurewei.com/network-readiness```.      
* ```mizar.futurewei.com/network_user_input``` will include Mizar network information such as ```vpc name```, ```subnet name``` and ```ip address```
* In cni daemon service, it will retrieve newly created Pod object, and pass in required network configuration. See below: 

```
    def cni_add(self, params):
        pod = self.get_pod_obj(params)

        result = {
            "cniVersion": params.cni_version,
            "interfaces": [
                {
                    "name": pod.veth_name,
                    "mac": pod.mac,
                    "sandbox": pod.netns
                }
            ],
            "ips": [
                {
                    "version": "4",
                    "address": "{}/{}".format(pod.ip, pod.prefix),
                    "gateway": pod.gw,
                    "interface": 0
                }
            ]
        }

    def get_pod_obj(self, params):
        name = ""
        namespace = ""
        tenant = ""
        if 'K8S_POD_NAME' in params.cni_args_dict:
            name = params.cni_args_dict['K8S_POD_NAME']
        if 'K8S_POD_NAMESPACE' in params.cni_args_dict:
            namespace = params.cni_args_dict['K8S_POD_NAMESPACE']
        if 'K8S_POD_TENANT' in params.cni_args_dict:
            tenant = params.cni_args_dict['K8S_POD_TENANT']

        name = name + "-" + namespace + "-" + tenant

        if CniService.store.contains_pod(name):
            return CniService.store.get_pod(name)
```

* Once creation is completed, operator will set ```arktos.futurewei.com/network-readiness == true```. 

* In return, the annotations becomes: 
```yaml
  annotations:
    arktos.futurewei.com/nic: {"name": "eth0", "ip": "10.10.1.12", "subnet": "net1"}
    arktos.futurewei.com/network-readiness: "true"
    mizar.futurewei.com/network_user_input: {"vpc": "vpc-1", "net": "net1", "ip": "10.10.1.12"}
```
Note: Integrity checks are needed here, where we need make sure that the ip address falls within subnet CIDR range and service CIDR range, and subnet CIDR range falls within the VPC's (vpc-1) CIDR range.


### Service Creation 

* List Kubernetes cluster object / Arktos newtork object
* Waiting on customer to create service using yaml file:  
```yaml
apiVersion: v1
kind: Service
metadata:
  name: my-service
  labels:
      arktos.futurewei.com/network: vpc-1
spec:
  selector:
    app: MyApp
  ports:
    - protocol: TCP
      port: 80
      targetPort: 9376
```
* Note that if ```arktos.futurewei.com/network``` not listed, and default network is selected
* Build-in operator kicks off ```my-service``` creation
* Use endpoint operator to create a scaled endpoint object within ```vpc-1```. 
* Once scaled endpoint is created, ```my-service``` is assigned with the ip address of this scaled endpoint object, and set spec.ClusterIP with this ip address. 
* Update the scaled endpoint with bouncers within ```vpc-1```
* Update object store: map Arktos network with services: ```self.store.services_arktosnet_store[vpc-1][my-service] = service_object```