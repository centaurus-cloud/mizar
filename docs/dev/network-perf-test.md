# How to test Mizar network performance
This document records the considerations of Mizar network performance testing, and the tools used and steps of processes in metric collecting.

Also, it has one sample collection of network metrics for the reference, which was the result of one of the basic performance testings.


## Background
Mizar is based on new technology of eBPF/XDP, provides brand new networking for containers. In order to evaluate Mizar's advantages, systematic process is desired to collect its network performance metric data, and compare with other network providers like Cilium and Kindnet (the defult cni plugin of Kind cluster).


## Target network metrics
* TCP stream throughput (not to confuse with bandwidth)
* UDP PPS throughput
* Network latency


## Basic Test Process
Basic test is not for large scale and stress purpose; it only microscopes at communication between two endpoints.

### Test env setup
For its simplicity, Kind based K8S is recommended as the test env: 1 control plane node, 2 worker nodes: so-called Kind 1+2 env. The two pods involving in the test are deployed in anti-affinity mode.

After the cluster is up, run following command to load needed images to Kind nodes:
```bash
kind load docker-image localhost:5000/testpod
kind load docker-image alectolytic/netperf
```

### Throughput testing
iperf3 available in the testpod image is used. Run following commands to start iperf3 server:
```bash
kubectl run iperf-s --image=localhost:5000/testpod --image-pull-policy=Never --overrides='{"spec":{"nodeName":"kind-worker"}}' -- iperf3 -s
```
Run following command to create iperf3 client:
```bash
kubectl run -it iperf-c --image=localhost:5000/testpod --image-pull-policy=Never --overrides='{"spec":{"nodeName":"kind-worker2"}}' -- bash
```
To get TCP stream throughput:
```bash
iperf3 -c iperf-s -t 60
```
To get UDP PPS metrics:
```bash
iperf3 -c iperf-s -t 60 -u -b 1000M -l 64
```
After the throughput test is done, delete iperf3 server and client pods.

### Network latency testing
netperf tool is used to collect network latency metrics, in form of TCP_RR(request/response with 1 byte message body).
To start netperf server (netserver):
```bash
kubectl run np-s --image alectolytic/netperf --image-pull-policy=Never --overrides='{"spec":{"nodeName":"kind-worker"}}'
```
To start netperf client, and test TCP/UDP latency:
```bash
kubectl run np-c --rm -it --image alectolytic/netperf --image-pull-policy=Never --overrides='{"spec":{"nodeName":"kind-worker2"}}' -- /bin/sh
netperf -H np-s -l 60 -t TCP_RR -- -o min_latency,mean_latency,max_latency,stddev_latency,transaction_rate
```

### Sample network metrics in Kind 1+2 env
Kind host: AWS t2.xlarge (4 cpu, 16 GB mem, 40 GB disk), Ubuntu 20.04

Below metrcis were colloected using Mizar commit# c2ed3f9c0535cc2ce804dbad76ed6b1850eccec0:
| TCP stream throughput |
| --------------------- |
| 867 Mbps |

| UDP PPS | recv lost rate |
| ------- | -------------- |
| 32,028  | 0.75% |

| number of simultaneous connections | TCP_RR Latency in microseconds |
| ---------------------------------- | ------------------------ |
| 1  | 95.30 |
| 10 | 246.34 |
| 20 | 485.70 |
| 40 | 896.15 |

Below metrics were collected using Mizar commit# a244c3083be2bfa62bca48bd78ef55821068d992 (which has network policy data plane implemented):
| TCP stream throughput |
| --------------------- |
| 812 Mbps |

| UDP PPS | recv lost rate |
| ------- | -------------- |
| 31,166  | 0.57% |

| number of simultaneous connections | TCP_RR Latency in microseconds |
| ---------------------------------- | ------------------------ |
| 1  | 102.87 |
| 10 | 262.18 |
| 20 | 491.54 |
| 40 | 947.54 |                               

### Metrics collected with other providers
In the same Kind 1+2 env; same test processes as the above are followed.

#### Kindnet
Followed [Configuring Your Kind Cluster](https://kind.sigs.k8s.io/docs/user/quick-start/#configuring-your-kind-cluster) to setup Kind 1+2 cluster, with default cni plugin which is Kindnet.

| TCP stream throughput |
| --------------------- |
| 14.2 Gbps |

| UDP PPS | recv lost rate |
| ------- | -------------- |
| 42,748  | 0.32% |

| number of simultaneous connections | TCP_RR Latency in microseconds |
| ---------------------------------- | ------------------------ |
| 1  | 83.27 |
| 10 | 169.18 |
| 20 | 364.36 |
| 40 | 754.13 |

#### Cilium
Cilium 1.9 is used in this test. Followed instruction of [Cluster Setup w/ Cilium plugin](https://docs.cilium.io/en/v1.9/gettingstarted/kind/) to set up Kind 1+2 cluster w/ Cilium cni plugin.

| TCP stream throughput |
| --------------------- |
| 10.6 Gbps |

| UDP PPS | recv lost rate |
| ------- | -------------- |
| 26,780  | 0.19% |

| number of simultaneous connections | TCP_RR Latency in microseconds |
| ---------------------------------- | ------------------------ |
| 1  | 107.03 |
| 10 | 280.51 |
| 20 | 591.13 |
| 40 | 1217.12 |

## Scale Test Process
TODO
