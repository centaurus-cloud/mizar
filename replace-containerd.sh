#!/bin/bash

cd $HOME/go/src/k8s.io/arktos
wget -qO- https://github.com/CentaurusInfra/containerd/releases/download/tenant-cni-args/containerd.zip | zcat > /tmp/containerd
sudo chmod +x /tmp/containerd
sudo systemctl stop containerd
sudo mv /usr/bin/containerd /usr/bin/containerd.bak
sudo mv /tmp/containerd /usr/bin/
sudo systemctl restart containerd
sudo systemctl restart docker
