# SPDX-License-Identifier: MIT
# Copyright (c) 2021 The Authors.

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

class IPv4Trie:
    def __init__(self):
        self.head = IPv4Node()        

    def insert(self, cidr, obj):
        bit_array = IPv4Util.get_bit_array_from_ipv4_cidr(cidr)

        node = self.head
        for bit in bit_array:
            if bit not in node.children:
                node.children[bit] = IPv4Node(bit)
            node = node.children[bit]
        node.is_end = True
        node.cidr = IPv4Util.get_standard_cidr(cidr)
        node.obj = obj

    def find_all(self, cidr):
        result = []
        bit_array = IPv4Util.get_bit_array_from_ipv4_cidr(cidr)

        node = self.head
        if node.is_end:
            result.append((node.cidr, node.obj))
        for bit in bit_array:
            if bit not in node.children:
                return result
            node = node.children[bit]
            if node.is_end:
                result.append((node.cidr, node.obj))
        return result

class IPv4Node:
    def __init__(self, value = False):
        self.children = {}
        self.is_end = False
        self.value = value
        self.cidr = ""
        self.obj = None

class IPv4Util:
    @staticmethod
    def get_bit_array_from_ipv4_cidr(cidr):
        splitted = cidr.split("/")
        if len(splitted) == 0 or len(splitted) > 2:
            raise ValueError("{} is not a valid IPv4 or CIDR.".format(cidr))
        ip = splitted[0]
        length_str = "32" if len(splitted) == 1 else splitted[1]
        if not length_str.isdigit():
            raise ValueError("{} is not a valid IPv4 or CIDR.".format(cidr))
        length = int(length_str)

        splitted = ip.split(".")
        if len(splitted) != 4:
            raise ValueError("{} is not a valid IPv4 or CIDR.".format(cidr))

        for segment in splitted:
            if not segment.isdigit():
                raise ValueError("{} is not a valid IPv4 or CIDR.".format(cidr))
            value = int(segment)
            if value < 0 or value > 255:
                raise ValueError("{} is not a valid IPv4 or CIDR.".format(cidr))

        result = []
        if length == 0:
            return result
        index = 0
        for segment in splitted:
            bits = IPv4Util.get_8_bit_list(cidr, segment)
            for i in range(8):
                result.append(bits[i])
                index += 1
                if index == length:
                    return result
        return result

    @staticmethod
    def get_8_bit_list(cidr, segment):
        binary = "{0:b}".format(int(segment))        

        result = []
        for _ in range(8 - len(binary)):
            result.append(False)
        for i in range(len(binary)):
            result.append(bool(int(binary[i])))

        return result

    # Standarlize cidr. For example, 0.1.2.3/0 => 0.0.0.0/0
    @staticmethod
    def get_standard_cidr(cidr):
        bit_array = IPv4Util.get_bit_array_from_ipv4_cidr(cidr)
        length = len(bit_array)
        for _ in range(32 - length):
            bit_array.append(False)

        str_list = []
        for i in range(4):
            segment_chars = []
            for j in range(8):
                segment_chars.append(str(int(bit_array[i * 8 + j])))
            str_list.append(str(int("".join(segment_chars), 2)))
            if i < 3:
                str_list.append(".")

        str_list.append("/")
        str_list.append(str(length))
        return "".join(str_list)
