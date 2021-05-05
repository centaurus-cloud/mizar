from teste2e.common.helper import *
from time import sleep

SCRIPTS_DIR = '/var/mizar/test/scripts'


class k8sPod:

    def __init__(self, api, name, ip):
        self.api = api
        self.name = name
        self.ip = ip

    @property
    def eptype(self):
        return "simple"

    def do_ping(self, server, count=1, wait=5):
        ip = server.ip
        cmd = 'ping -w {} -c {} {}'.format(wait, count, ip)
        err = self.api.pod_exec(self.name, cmd)
        logger.info("do_ping output {}, got: {}".format(self.name, err))
        return err['status'] == "Success"

    def do_curl_hostname(self, server, ep_type="simple"):
        ip = server.ip
        cmd = f'''/bin/bash {SCRIPTS_DIR}/curl_client_hostname.sh {ip} {ep_type}'''
        out = self.api.pod_exec_stdout(self.name, cmd)
        logger.info("do_curl_client {}, got: {}".format(self.name, out))

        if server.eptype == "simple":
            return out == server.name

        if server.eptype == "scaled":
            return out in server.backends

    def do_tcp_hostname(self, server, ep_type="simple"):
        ip = server.ip
        cmd = f'''/bin/bash {SCRIPTS_DIR}/nc_client_hostname.sh {ip} tcp {ep_type}'''
        out = self.api.pod_exec_stdout(self.name, cmd)
        logger.info("do_tcp_client {}, got: {}".format(self.name, out))

        if server.eptype == "simple":
            return out == server.name

        if server.eptype == "scaled":
            return out in server.backends

    def do_udp_hostname(self, server, ep_type="simple"):
        ip = server.ip
        cmd = f'''/bin/bash {SCRIPTS_DIR}/nc_client_hostname.sh {ip} udp {ep_type}'''
        out = self.api.pod_exec_stdout(self.name, cmd)
        logger.info("do_udp_client {}, got: {}".format(self.name, out))

        if server.eptype == "simple":
            return out == server.name

        if server.eptype == "scaled":
            return out in server.backends
