import logging
import sys
import os
import subprocess
import mizar.proto.droplet_pb2 as droplet_pb2
import mizar.proto.droplet_pb2_grpc as droplet_pb2_grpc
import time
import grpc
from concurrent import futures
from google.protobuf import empty_pb2

logger = logging.getLogger()


class DropletServer(droplet_pb2_grpc.DropletServiceServicer):

    def __init__(self):
        self.itf = 'eth0'
        cmd = 'ip addr show eth0 | grep "inet\\b" | awk \'{print $2}\' | cut -d/ -f1'
        r = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE)
        self.ip = r.stdout.read().decode().strip()

        cmd = 'ip addr show eth0 | grep "link/ether\\b" | awk \'{print $2}\' | cut -d/ -f1'
        r = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE)
        self.mac = r.stdout.read().decode().strip()

        cmd = 'hostname'
        r = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE)
        self.name = r.stdout.read().decode().strip()

        # Disable TSO and checksum offload as xdp currently does not support
        cmd = "ethtool -K {} tso off gso off ufo off".format(self.itf)
        r = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE)
        cmd = "ethtool --offload {} rx off tx off".format(self.itf)
        r = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE)

    def GetDropletInfo(self, request, context):
        droplet = droplet_pb2.Droplet(
            name=self.name,
            ip=self.ip,
            mac=self.mac,
            itf=self.itf
        )
        return droplet


class DropletClient():
    def __init__(self, ip):
        self.channel = grpc.insecure_channel('{}:50051'.format(ip))
        self.stub = droplet_pb2_grpc.DropletServiceStub(self.channel)

    def GetDropletInfo(self):
        resp = self.stub.GetDropletInfo(empty_pb2.Empty())
        return resp
