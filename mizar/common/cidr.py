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

import ipaddress


class Cidr:
    def __init__(self, prefixlen, ip):
        """
        Defines an IPv4 CIDR block
        """
        self.prefixlen = prefixlen
        self.ip = ip
        self.ipnet = ipaddress.ip_network(
            "{}/{}".format(self.ip, self.prefixlen))
        self.subnets = self.ipnet.subnets(new_prefix=30)

        self._hosts = set()
        self.gw = self.get_ip(1)
        self.allocated = set()

    @property
    def hosts(self):

        pool = next(self.subnets)
        self._hosts.update(set(pool.hosts()))
        self._hosts.discard(self.gw)
        return self._hosts

    def get_ip(self, idx):
        return self.ipnet[idx]

    def get_hosts(self):
        return self.hosts

    def allocate_ip(self):
        if not len(self.hosts):
            return None
        ip = self.hosts.pop()
        # TODO: bad hack, search the list and remove it!!
        while ip in self.allocated:
            ip = self.hosts.pop()
        self.allocated.add(ip)
        return str(ip)

    def mark_ip_as_allocated(self, ip):
        self.allocated.add(ipaddress.IPv4Address(ip))

    def deallocate_ip(self, ip):
        ip = ipaddress.IPv4Address(ip)
        if ip in self.hosts:
            if ip in self.allocated:
                self.allocated.remove(ip)
            ip = self._hosts.add(ip)
