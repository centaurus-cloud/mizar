# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Authors.

# Authors: Sherif Abdelwahab <@zasherif>
#          Phu Tran          <@phudtran>

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

FROM kindest/node:v1.18.2
RUN apt-get update -y
RUN apt-get install -y apt-utils
RUN apt-get install -y sudo
RUN apt-get install -y vim
RUN apt-get install -y rpcbind
RUN apt-get install -y rsyslog
RUN apt-get install -y libelf-dev
RUN apt-get install -y iproute2
RUN apt-get install -y net-tools
RUN apt-get install -y iputils-ping
RUN apt-get install -y ethtool
RUN apt-get install -y curl
RUN apt-get install -y python3
RUN apt-get install -y python3-pip
RUN apt-get install -y tcpdump
RUN pip3 install PyYAML
RUN pip3 install kopf
RUN pip3 install netaddr
RUN pip3 install ipaddress
RUN pip3 install pyroute2
RUN pip3 install rpyc
RUN pip3 install kubernetes==11.0.0
RUN pip3 install luigi==2.8.12
RUN pip3 install grpcio
RUN pip3 install protobuf
RUN pip3 install fs
RUN mkdir -p /var/mizar/
RUN mkdir -p /opt/cni/bin
RUN mkdir -p /etc/cni/net.d
RUN ln -snf /sys/fs/bpf /bpffs
