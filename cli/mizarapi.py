import logging
import datetime
from kubernetes import client, config

logger = logging.getLogger()


class MizarApi:

    def __init__(self):
        config.load_kube_config()
        self.obj_api = client.CustomObjectsApi()

    def create_vpc(self, name, ip, prefix, dividers=1, vni=None):
        logger.info("Creating a vpc!!!")
        spec = {
            "ip": ip,
            "prefix": prefix,
            "vni": vni,
            "dividers": dividers,
        }
        self.create_obj(name, "Vpc", spec, "vpcs")

        logger.debug("Created {} {}".format("Vpc", name))

    def delete_vpc(self, name):
        logger.info("Delete a vpc!!!")
        self.delete_obj(name, "vpcs")

    def create_net(self, name, ip, prefix, vpc, bouncers=1, vni=None):
        logger.info("Creating a network!!!")
        spec = {
            "ip": ip,
            "prefix": prefix,
            "vni": vni,
            "vpc": vpc,
            "bouncers": bouncers
        }
        self.create_obj(name, "Subnet", spec, "subnets")

        logger.debug("Created {} {}".format("Network", name))

    def delete_net(self, name):
        logger.info("Delete a network!!!")
        self.delete_obj(name, "subnets")

    def create_obj(self, name, kind, spec, plural):
        spec["status"] = "Init"
        body = {
            "apiVersion": "mizar.com/v1",
            "kind": kind,
            "metadata": {
                "name": name
            },
            "spec": spec
        }
        body['spec']['createtime'] = datetime.datetime.now().isoformat()
        self.obj_api.create_namespaced_custom_object(
            group="mizar.com",
            version="v1",
            namespace="default",
            plural=plural,
            body=body,
        )

    def delete_obj(self, name, plural):
        self.obj_api.delete_namespaced_custom_object(
            group="mizar.com",
            version="v1",
            namespace="default",
            plural=plural,
            name=name,
            body=client.V1DeleteOptions(),
            propagation_policy="Orphan")
