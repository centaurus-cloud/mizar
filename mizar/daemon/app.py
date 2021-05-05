
import grpc
import time
import subprocess
import json
from google.protobuf import empty_pb2
from concurrent import futures
from mizar.daemon.interface_service import InterfaceServer
from mizar.daemon.droplet_service import DropletServer
from mizar.common.constants import CONSTANTS
import mizar.proto.interface_pb2_grpc as interface_pb2_grpc
import mizar.proto.interface_pb2 as interface_pb2
import mizar.proto.droplet_pb2_grpc as droplet_pb2_grpc
import mizar.proto.droplet_pb2 as droplet_pb2
import os
import logging
import sys


logging.basicConfig(level=logging.DEBUG)
logger = logging.getLogger()

POOL_WORKERS = 10


def init():
    # Setup the droplet's host
    script = (f''' bash -c '\
    nsenter -t 1 -m -u -n -i ls -1 /etc/cni/net.d/*conf* | grep -v '10-mizarcni.conf$' | xargs rm -rf && \
    nsenter -t 1 -m -u -n -i /etc/init.d/rpcbind restart && \
    nsenter -t 1 -m -u -n -i /etc/init.d/rsyslog restart && \
    nsenter -t 1 -m -u -n -i sysctl -w net.ipv4.tcp_mtu_probing=2 && \
    nsenter -t 1 -m -u -n -i ip link set dev eth0 up mtu 9000' ''')
    r = subprocess.Popen(script, shell=True, stdout=subprocess.PIPE)
    output = r.stdout.read().decode().strip()
    logging.info("Setup done")

    cmd = 'nsenter -t 1 -m -u -n -i ip addr show eth0 | grep "inet\\b" | awk \'{print $2}\' | cut -d/ -f1'
    r = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE)
    ip = r.stdout.read().decode().strip()

    cmd = "nsenter -t 1 -m -u -n -i ip link set dev eth0 xdpgeneric off"

    r = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE)
    output = r.stdout.read().decode().strip()
    logging.info("Removed existing XDP program: {}".format(output))

    cmd = "nsenter -t 1 -m -u -n -i /trn_bin/transitd &"
    r = subprocess.Popen(cmd, shell=True)
    logging.info("Running transitd")
    time.sleep(1)
    config = {
        "xdp_path": "/trn_xdp/trn_transit_xdp_ebpf_debug.o",
        "pcapfile": "/bpffs/transit_xdp.pcap",
        "xdp_flag": CONSTANTS.XDP_GENERIC
        }
    config = json.dumps(config)
    cmd = (
        f'''nsenter -t 1 -m -u -n -i /trn_bin/transit -s {ip} load-transit-xdp -i eth0 -j '{config}' ''')

    r = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE)
    output = r.stdout.read().decode().strip()
    logging.info("Running load-transit-xdp: {}".format(output))


def serve():
    server = grpc.server(futures.ThreadPoolExecutor(max_workers=POOL_WORKERS))

    droplet_pb2_grpc.add_DropletServiceServicer_to_server(
        DropletServer(), server
    )

    interface_pb2_grpc.add_InterfaceServiceServicer_to_server(
        InterfaceServer(), server
    )

    server.add_insecure_port('[::]:50051')
    server.start()
    logger.info("Transit daemon is ready")
    try:
        while True:
            time.sleep(100000)
    except KeyboardInterrupt:
        server.stop(0)


init()
serve()
