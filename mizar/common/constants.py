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

# Constants
group = 'mizar.com'
version = 'v1'


class CONSTANTS:
    TRAN_SUBSTRT_EP = 0
    TRAN_SIMPLE_EP = 1
    TRAN_SCALED_EP = 2
    TRAN_GATEWAY_EP = 3
    ON_XDP_TX = "ON_XDP_TX"
    ON_XDP_PASS = "ON_XDP_PASS"
    ON_XDP_REDIRECT = "ON_XDP_REDIRECT"
    ON_XDP_DROP = "ON_XDP_DROP"
    ON_XDP_SCALED_EP = "ON_XDP_SCALED_EP"
    IPPROTO_TCP = "6"
    IPROTO_UDP = "17"
    MAX_CLI_CHAR_LENGTH = 2000
    XDP_GENERIC = "2"
    XDP_DRIVER = "4"
    XDP_OFFLOAD = "8"

class OBJ_STATUS:
    obj_init = 'Init'
    obj_provisioned = 'Provisioned'
    ep_status_init = obj_init
    ep_status_allocated = 'Alloc'
    ep_status_bouncer_ready = 'BouncerReady'
    ep_status_provisioned = obj_provisioned

    net_status_init = obj_init
    net_status_allocated = 'Alloc'
    net_status_ready = 'Ready'
    net_status_provisioned = obj_provisioned

    vpc_status_init = obj_init
    vpc_status_allocated = 'Alloc'
    vpc_status_ready = 'Ready'
    vpc_status_provisioned = obj_provisioned

    droplet_status_init = obj_init
    droplet_status_allocated = 'Alloc'
    droplet_status_ready = 'Ready'
    droplet_status_provisioned = obj_provisioned

    bouncer_status_init = obj_init
    bouncer_status_allocated = 'Alloc'
    bouncer_status_ready = 'Ready'
    bouncer_status_provisioned = obj_provisioned
    bouncer_status_placed = 'Placed'
    bouncer_status_endpoint_ready = 'EndpointReady'
    bouncer_status_divider_ready = 'DividerReady'

    divider_status_init = obj_init
    divider_status_allocated = 'Alloc'
    divider_status_provisioned = 'Ready'
    divider_status_provisioned = obj_provisioned
    divider_status_placed = 'Placed'


class OBJ_DEFAULTS:
    default_ep_vpc = 'vpc0'
    default_ep_net = 'net0'
    default_ep_type = 'simple'
    default_vpc_vni = '1'
    default_vpc_ip = '20.0.0.0'
    default_vpc_prefix = '8'
    default_net_ip = '20.0.0.0'
    default_net_gw = '20.0.0.1'
    default_net_prefix = '8'
    default_n_bouncers = 1
    default_n_dividers = 1
    default_host_ep_prefix = "32"
    ep_type_simple = 'simple'
    ep_type_scaled = 'scaled'
    ep_type_host = 'host'
    ep_type_gateway = 'gateway'
    droplet_eps = [ep_type_simple, ep_type_host]

    mizar_service_annotation_key = "service.beta.kubernetes.io/mizar-scaled-endpoint-type"
    mizar_service_annotation_val = "scaled-endpoint"

    arktos_pod_label = "arktos.futurewei.com/network"
    arktos_pod_annotation = "arktos.futurewei.com/nic"
    kopf_max_retries = 10


class RESOURCES:
    endpoints = "endpoints"
    nets = "subnets"
    vpcs = "vpcs"
    droplets = "droplets"
    bouncers = "bouncers"
    dividers = "dividers"


class COMPUTE_PROVIDER:
    kubernetes = "kubernetes"
    k8s = True
    arktos = "arktos"


class LAMBDAS:
    ep_status_init = lambda body, **_: body.get('spec', {}).get(
        'status', '') == OBJ_STATUS.ep_status_init
    ep_status_allocated = lambda body, **_: body.get('spec', {}).get(
        'status', '') == OBJ_STATUS.ep_status_allocated
    ep_status_provisioned = lambda body, **_: body.get('spec', {}).get(
        'status', '') == OBJ_STATUS.ep_status_provisioned
    ep_status_bouncer_ready = lambda body, **_: body.get('spec', {}).get(
        'status', '') == OBJ_STATUS.ep_status_bouncer_ready

    net_status_init = lambda body, **_: body.get('spec', {}).get(
        'status', '') == OBJ_STATUS.net_status_init
    net_status_allocated = lambda body, **_: body.get('spec', {}).get(
        'status', '') == OBJ_STATUS.net_status_allocated
    net_status_ready = lambda body, **_: body.get('spec', {}).get(
        'status', '') == OBJ_STATUS.net_status_ready
    net_status_provisioned = lambda body, **_: body.get('spec', {}).get(
        'status', '') == OBJ_STATUS.net_status_provisioned

    vpc_status_init = lambda body, **_: body.get('spec', {}).get(
        'status', '') == OBJ_STATUS.vpc_status_init
    vpc_status_allocated = lambda body, **_: body.get('spec', {}).get(
        'status', '') == OBJ_STATUS.vpc_status_allocated
    vpc_status_ready = lambda body, **_: body.get('spec', {}).get(
        'status', '') == OBJ_STATUS.vpc_status_ready
    vpc_status_provisioned = lambda body, **_: body.get('spec', {}).get(
        'status', '') == OBJ_STATUS.vpc_status_provisioned

    droplet_status_init = lambda body, **_: body.get('spec', {}).get(
        'status', '') == OBJ_STATUS.droplet_status_init
    droplet_status_allocated = lambda body, **_: body.get('spec', {}).get(
        'status', '') == OBJ_STATUS.droplet_status_allocated
    droplet_status_ready = lambda body, **_: body.get('spec', {}).get(
        'status', '') == OBJ_STATUS.droplet_status_ready
    droplet_status_provisioned = lambda body, **_: body.get('spec', {}).get(
        'status', '') == OBJ_STATUS.droplet_status_provisioned

    bouncer_status_init = lambda body, **_: body.get('spec', {}).get(
        'status', '') == OBJ_STATUS.bouncer_status_init
    bouncer_status_allocated = lambda body, **_: body.get('spec', {}).get(
        'status', '') == OBJ_STATUS.bouncer_status_allocated
    bouncer_status_ready = lambda body, **_: body.get('spec', {}).get(
        'status', '') == OBJ_STATUS.bouncer_status_ready
    bouncer_status_provisioned = lambda body, **_: body.get('spec', {}).get(
        'status', '') == OBJ_STATUS.bouncer_status_provisioned
    bouncer_status_placed = lambda body, **_: body.get('spec', {}).get(
        'status', '') == OBJ_STATUS.bouncer_status_placed
    bouncer_status_endpoint_ready = lambda body, **_: body.get('spec', {}).get(
        'status', '') == OBJ_STATUS.bouncer_status_endpoint_ready

    divider_status_init = lambda body, **_: body.get('spec', {}).get(
        'status', '') == OBJ_STATUS.divider_status_init
    divider_status_allocated = lambda body, **_: body.get('spec', {}).get(
        'status', '') == OBJ_STATUS.divider_status_allocated
    divider_status_provisioned = lambda body, **_: body.get('spec', {}).get(
        'status', '') == OBJ_STATUS.divider_status_provisioned
    divider_status_placed = lambda body, **_: body.get('spec', {}).get(
        'status', '') == OBJ_STATUS.divider_status_placed
    k8s_provider_vanilla = lambda **_: COMPUTE_PROVIDER.k8s == True
