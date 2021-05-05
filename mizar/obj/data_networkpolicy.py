# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Authors.

# Authors: Hong Chang   <@Hong-Chang>

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

class CidrNetworkPolicy:
    def __init__(self, vni, local_ip, cidr, cidr_length, policy_bit_value, cidr_type):
        self.vni = vni
        self.local_ip = local_ip
        self.cidr = cidr
        self.cidr_length = cidr_length
        self.policy_bit_value = policy_bit_value
        self.cidr_type = cidr_type

    def get_cidr_type_int(self):
        if self.cidr_type == "no_except":
            return "0"
        elif self.cidr_type == "with_except":
            return "1"
        else:
            return "2"


class PortNetworkPolicy:
    def __init__(self, vni, local_ip, protocol, port, policy_bit_value):
        self.vni = vni
        self.local_ip = local_ip
        self.protocol = protocol
        self.port = port
        self.policy_bit_value = policy_bit_value

    def get_protocol_int(self):
        protocol = self.protocol.lower()
        if protocol == "tcp":
            return "6"
        elif protocol == "udp":
            return "17"
        elif protocol == "sctp":
            return "132"
        elif protocol == "any":
            return "0"
        else:
            return "-1"

class EndpointEnforced:
    def __init__(self, vni, ip):
        self.vni = vni
        self.ip = ip
