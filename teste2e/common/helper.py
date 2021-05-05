import os
import logging
import subprocess
from netaddr import *

logger = logging.getLogger("transit_test")
logging.basicConfig(level=os.environ.get("LOGLEVEL", "INFO"))

TEST_CLUSTER = 'kind'


def run_cmd(cmd_list):
    result = subprocess.Popen(cmd_list, shell=True, stdout=subprocess.PIPE)
    text = result.stdout.read().decode()
    returncode = result.returncode
    return (returncode, text)


def do_common_tests(test, ep1, ep2):
    do_ping_test(test, ep1, ep2)
    do_http_hostname_test(test, ep1, ep2)
    do_tcp_hostname_test(test, ep1, ep2)
    do_udp_hostname_test(test, ep1, ep2)


def do_ping_test(test, ep1, ep2):
    logger.info(
        "-------Test {}: ping {} {} ------".format(type(test).__name__, ep1.name, ep2.name))
    test.assertTrue(ep1.do_ping(ep2))
    test.assertTrue(ep2.do_ping(ep1))


def do_http_hostname_test(test, ep1, ep2):
    logger.info("-------Test {}: HTTP hostname {} {} ------".format(
        type(test).__name__, ep1.name, ep2.name))
    test.assertTrue(ep1.do_curl_hostname(ep2))
    test.assertTrue(ep2.do_curl_hostname(ep1))


def do_tcp_hostname_test(test, ep1, ep2):
    logger.info(
        "-------Test {}: TCP hostname {} {} ------".format(type(test).__name__, ep1.name, ep2.name))
    test.assertTrue(ep1.do_tcp_hostname(ep2))
    test.assertTrue(ep2.do_tcp_hostname(ep1))


def do_udp_hostname_test(test, ep1, ep2):
    logger.info(
        "-------Test {}: UDP hostname {} {} ------".format(type(test).__name__, ep1.name, ep2.name))
    test.assertTrue(ep1.do_udp_hostname(ep2))
    test.assertTrue(ep2.do_udp_hostname(ep1))
