#!/bin/bash

####################

echo Setup: Install go \(currently limited to version 1.13.9\)

sudo apt-get update -y -q
sudo apt-get install build-essential -y -q
sudo apt-get install zlib1g-dev -y -q
sudo apt-get install libreadline-gplv2-dev -y -q
sudo apt-get install libncursesw5-dev -y -q
sudo apt-get install libssl-dev -y -q
sudo apt-get install libnss3-dev -y -q
sudo apt-get install libsqlite3-dev -y -q
sudo apt-get install tk-dev -y -q
sudo apt-get install libgdbm-dev -y -q
sudo apt-get install libc6-dev -y -q
sudo apt-get install libbz2-dev -y -q
sudo apt-get install libreadline-dev -y -q
sudo apt-get install libffi-dev -y -q

cd /tmp
wget https://dl.google.com/go/go1.13.9.linux-amd64.tar.gz
sudo tar -C /usr/local -xzf go1.13.9.linux-amd64.tar.gz
rm -rf go1.13.9.linux-amd64.tar.gz

####################

echo Setup: Install bazel

sudo apt install g++ unzip zip -y -q
sudo apt-get install openjdk-8-jdk -y -q
cd /tmp
wget https://github.com/bazelbuild/bazel/releases/download/0.26.1/bazel-0.26.1-installer-linux-x86_64.sh
chmod +x bazel-0.26.1-installer-linux-x86_64.sh
./bazel-0.26.1-installer-linux-x86_64.sh --user

####################

echo Setup: Enlist arktos

cd ~
git clone https://github.com/CentaurusInfra/arktos.git ~/go/src/k8s.io/arktos
cd ~/go/src/k8s.io
ln -s ./arktos kubernetes

git config --global credential.helper 'cache --timeout=3600000'

####################

echo Setup: Install etcd

cd ~/go/src/k8s.io/arktos/
git tag v1.15.0
./hack/install-etcd.sh

####################

echo Setup: Install Docker

sudo apt -y install docker.io
sudo gpasswd -a $USER docker

####################

echo Setup: Install crictl

cd /tmp
wget https://github.com/kubernetes-sigs/cri-tools/releases/download/v1.17.0/crictl-v1.17.0-linux-amd64.tar.gz
sudo tar -zxvf crictl-v1.17.0-linux-amd64.tar.gz -C /usr/local/bin
rm -f crictl-v1.17.0-linux-amd64.tar.gz

touch /tmp/crictl.yaml
echo runtime-endpoint: unix:///run/containerd/containerd.sock >> /tmp/crictl.yaml
echo image-endpoint: unix:///run/containerd/containerd.sock >> /tmp/crictl.yaml
echo timeout: 10 >> /tmp/crictl.yaml
echo debug: true >> /tmp/crictl.yaml
sudo mv /tmp/crictl.yaml /etc/crictl.yaml

sudo mkdir -p /etc/containerd
sudo rm -rf /etc/containerd/config.toml
sudo containerd config default > /tmp/config.toml
sudo mv /tmp/config.toml /etc/containerd/config.toml
sudo systemctl restart containerd

####################

echo Setup: Install miscellaneous

wget -O $HOME/Python-3.8.8.tgz https://www.python.org/ftp/python/3.8.8/Python-3.8.8.tgz
tar -C $HOME -xzf $HOME/Python-3.8.8.tgz
cd $HOME/Python-3.8.8
sudo ./configure --enable-optimizations
sudo make
sudo make altinstall
sudo ln -sfn /usr/local/bin/python3.8 /usr/bin/python3
sudo apt remove -fy python3-apt
sudo apt install -fy python3-apt
sudo apt update
sudo apt install -fy python3-pip

sudo sed -i '1c\#!/usr/bin/python3.6 -Es' /usr/bin/lsb_release
sudo /usr/local/bin/python3.8 -m pip install --upgrade pip

sudo apt install awscli -y -q
sudo apt install jq -y -q

sudo rm -rf $HOME/Python-3.8.8.tgz
sudo rm -rf $HOME/Python-3.8.8

####################

echo Setup: Mizar Related

cd ~/mizar
sudo apt-get update
sudo apt-get install -y \
    build-essential clang-7 llvm-7 \
    libelf-dev \
    python3 \
    python3-pip \
    libcmocka-dev \
    lcov

sudo pip3 install netaddr docker scapy
sudo systemctl unmask docker.service
sudo systemctl unmask docker.socket
sudo systemctl start docker
sudo systemctl enable docker

sudo docker build -f ./test/Dockerfile -t buildbox:v2 ./test

git submodule update --init --recursive

ver=$(curl -s https://api.github.com/repos/kubernetes-sigs/kind/releases/latest | grep -oP '"tag_name": "\K(.*)(?=")')
curl -Lo kind https://github.com/kubernetes-sigs/kind/releases/download/$ver/kind-$(uname)-amd64
chmod +x kind
sudo mv kind /usr/local/bin

curl -LO https://storage.googleapis.com/kubernetes-release/release/`curl -s https://storage.googleapis.com/kubernetes-release/release/stable.txt`/bin/linux/amd64/kubectl
chmod +x ./kubectl
sudo mv ./kubectl /usr/local/bin/kubectl

sudo pip3 install fs
sudo pip3 install protobuf
sudo pip3 install grpcio
sudo pip3 install grpcio-tools
sudo pip3 install luigi==2.8.12
sudo pip3 install kubernetes==11.0.0
sudo pip3 install rpyc
sudo pip3 install pyroute2
sudo pip3 install ipaddress
sudo pip3 install netaddr
sudo pip3 install kopf
sudo pip3 install PyYAML

#####################

echo Setup: Install Containerd

cd $HOME
wget https://github.com/containerd/containerd/releases/download/v1.4.2/containerd-1.4.2-linux-amd64.tar.gz
cd /usr
sudo tar -xvf $HOME/containerd-1.4.2-linux-amd64.tar.gz
sudo rm -rf $HOME/containerd-1.4.2-linux-amd64.tar.gz

####################

echo Setup: Setup profile

echo PATH=\"\$HOME/go/src/k8s.io/arktos/third_party/etcd:/usr/local/go/bin:\$HOME/go/bin:\$HOME/go/src/k8s.io/arktos/_output/bin:\$HOME/go/src/k8s.io/arktos/_output/dockerized/bin/linux/amd64:\$PATH\" >> ~/.profile
echo GOPATH=\"\$HOME/go\" >> ~/.profile
echo GOROOT=\"/usr/local/go\" >> ~/.profile
echo >> ~/.profile
echo alias arktos=\"cd \$HOME/go/src/k8s.io/arktos\" >> ~/.profile
echo alias k8s=\"cd \$HOME/go/src/k8s.io/kubernetes\" >> ~/.profile
echo alias mizar=\"cd \$HOME/mizar\" >> ~/.profile
echo alias up=\"\$HOME/go/src/k8s.io/arktos/hack/arktos-up.sh\" >> ~/.profile
echo alias status=\"git status\" >> ~/.profile
echo alias pods=\"kubectl get pods -A -o wide\" >> ~/.profile
echo alias nets=\"echo 'kubectl get subnets'\; kubectl get subnets\; echo\; echo 'kubectl get droplets'\; kubectl get droplets\; echo\; echo 'kubectl get bouncers'\; kubectl get bouncers\; echo\; echo 'kubectl get dividers'\; kubectl get dividers\; echo\; echo 'kubectl get vpcs'\; kubectl get vpcs\; echo\; echo 'kubectl get eps'\; kubectl get eps\; echo\; echo 'kubectl get networks'\; kubectl get networks\" >> ~/.profile

echo alias kubectl=\'$HOME/go/src/k8s.io/arktos/cluster/kubectl.sh\'  >> ~/.profile
echo alias kubeop=\"kubectl get pods \|\ grep mizar-operator \|\ awk \'{print \$1}\' \|\ xargs -i kubectl logs {}\"  >> ~/.profile
echo alias kubed=\"kubectl get pods \|\ grep mizar-daemon \|\ awk \'{print \$1}\' \|\ xargs -i kubectl logs {}\"  >> ~/.profile
echo export CONTAINER_RUNTIME_ENDPOINT=\"\containerRuntime,container,/run/containerd/containerd.sock\" >> ~/.profile
echo export PYTHONPATH=\"\$HOME/mizar/\" >> ~/.profile
echo export KUBECTL_LOG=\"\/tmp/${USER}_kubetctl.err\" >> ~/.profile
echo export GPG_TTY=\$\(tty\) >> ~/.profile

echo cd \$HOME/go/src/k8s.io/arktos >> ~/.profile

source "$HOME/.profile"

####################

echo Setup: Install Kind

cd ~/go/src/
GO111MODULE="on" go get sigs.k8s.io/kind@v0.7.0

####################

echo Setup: Install kubetest

cd ~/go/src/k8s.io
git clone https://github.com/kubernetes/test-infra.git
cd ~/go/src/k8s.io/test-infra/
GO111MODULE=on go install ./kubetest
GO111MODULE=on go mod vendor

####################

echo Setup: Basic settings

sudo systemctl stop ufw
sudo systemctl disable ufw
sudo swapoff -a
sudo sed -i '/swap/s/^/#/g' /etc/fstab
sudo partprobe
IP_ADDR=$(hostname -I | awk '{print $1}')
HOSTNAME=$(hostname)
sudo sed -i '2s/.*/'$IP_ADDR' '$HOSTNAME'/' /etc/hosts
sudo rm -f /etc/resolv.conf
sudo ln -s /run/systemd/resolve/resolv.conf /etc/resolv.conf

#####################

echo Setup: Machine setup completed!

sudo reboot
