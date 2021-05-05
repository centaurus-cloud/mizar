#!/bin/bash

tput setaf 1
echo "NOTE: This script will reboot the system if you opt to allow kernel update."
echo "      If reboot is not required, it will log you out and require re-login for new permissions to take effect."
echo ""
read -n 1 -s -r -p "Press Ctrl-c to quit, any key to continue..."
tput sgr0

logout_needed=false

sudo apt-get update
sudo apt-get install -y \
    build-essential clang-7 llvm-7 \
    libelf-dev \
    python3 \
    python3-pip \
    libcmocka-dev \
    lcov

sudo apt-get install -y docker.io
sudo pip3 install --upgrade pip
sudo pip3 install netaddr docker scapy grpcio-tools
sudo systemctl unmask docker.service
sudo systemctl unmask docker.socket

cat /etc/group | grep docker | grep ${USER} &> /dev/null
if [ $? -ne 0 ]; then
  sudo usermod -aG docker ${USER}
  logout_needed=true
fi

sudo systemctl start docker
sudo systemctl enable docker

# Install kind
if [ ! -f /usr/local/bin/kind ]; then
    pushd /tmp
    ver=$(curl -s https://api.github.com/repos/kubernetes-sigs/kind/releases/latest | grep -oP '"tag_name": "\K(.*)(?=")')
    curl -Lo kind "https://github.com/kubernetes-sigs/kind/releases/download/$ver/kind-$(uname)-amd64"
    chmod +x kind
    sudo mv kind /usr/local/bin
    popd
fi

sudo docker build -f ./test/Dockerfile -t buildbox:v2 ./test

git submodule update --init --recursive

kernel_ver=`uname -r`
echo "Running kernel version: $kernel_ver"
mj_ver=$(echo $kernel_ver | cut -d. -f1)
mn_ver=$(echo $kernel_ver | cut -d. -f2)

if [[ "$mj_ver" < "5" ]] || [[ "$mn_ver" < "6" ]]; then
        tput setaf 1
	echo "Mizar requires an updated kernel: linux-5.6-rc2 for TCP to function correctly. Current version is $kernel_ver"

	read -p "Execute kernel update script (y/n)?" choice
	tput sgr0
	case "$choice" in
	  y|Y ) sh ./kernelupdate.sh;;
	  n|N ) echo "Please run kernelupdate.sh to download and update the kernel!"; exit;;
	  * ) echo "Please run kernelupdate.sh to download and update the kernel!"; exit;;
	esac
fi

if [ "$logout_needed" = true ]; then
    logout
fi
