#!/bin/bash

# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Mizar Authors.

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

set -eu

mkdir -p /etc/cni/net.d
mkdir -p /opt/cni/bin

apt-get update -y
apt-get install -y \
    sudo \
    rpcbind \
    rsyslog \
    libelf-dev \
    iproute2  \
    net-tools \
    iputils-ping \
    ethtool \
    curl \
    python3 \
    python3-pip

pip3 install --upgrade protobuf
pip3 install --ignore-installed /var/mizar/
ln -snf /sys/fs/bpf /bpffs
ln -snf /var/mizar/build/bin /trn_bin
ln -snf /var/mizar/build/xdp /trn_xdp
ln -snf /var/mizar/etc/cni/10-mizarcni.conf /etc/cni/net.d/10-mizarcni.conf
ln -snf /var/mizar/mizar/cni.py /opt/cni/bin/mizarcni
ln -snf /var/mizar/build/tests/mizarcni.config /etc/mizarcni.config

echo "mizarcni installed"
