# Mizar – High Scale and High Performance Cloud Network #

Mizar is a large scale and high-performance cloud network to run virtual
machines, containers, and other compute workloads. Mizar offers flexible
in-network processing, which simplifies the programming of data-plane to scale
compared to traditional flow-based programming models. Unlike traditional
networking solutions, Mizar relies on the natural partitioning of a cloud
network to scale.

We built Mizar from ground-up on top of
[XDP](https://prototype-kernel.readthedocs.io/en/latest/networking/XDP/).
Mizar's main building block is an XDP program that runs on each host. The
program implements virtual functions including overlay switching, routing,
virtual endpoints, load-balancing, NAT, etc.

Mizar network has the following advantages:

- Support large scale of network endpoints in one cluster
- Fast network provisioning
- High network throughput and low latency
- Extendable data plane
- Unified data plane for VM and container
- Multi-tenant isolation for traffic and address spaces

We think of Mizar as a server-less platform for networking functions, in which
developers extend it with capabilities without compromising performance or
scale. The following diagram illustrates Mizar's high-level architecture:

![Mizar Overview](docs/design/png/overall_mgmt_dp.png)

Mizar's [data-plane](docs/design/dp_overview.md) provides high performance and
extensible packet processing pipeline and functions that achieve Mizar's
functional, scale, and performance goals. Mizar's
[management-plane](docs/design/mp_overview.md) programs the data-plane by
translating typical networking related APIs and resources to Mizar specific
configuration. The programmability of the data-plane involves loading and
unloading network functions at various stages of the packet processing pipeline.
Mizar management-plane can support different networking technologies. The
data-plane has been successfuly integrated as well in other implementations of
management-planes.

## Why Mizar is different?

Unlike traditional networking solutions, Mizar relies on the natural
partitioning of a cloud network to scale. Mizar simplifies the programming of
data-plane to scale by flexible in-network processing, compared to flow-based
programming models. As it primarily targets use cases of cloud-networking among
virtual machines and containers, Mizar reduces the control-plane overhead of
several routing and switching protocols within a cloud environment (e.g., L2
learning, ARP, BGP, etc.).

The following diagram illustrates the overall logical architecture of Mizar:

![Mizar Overview](docs/design/png/Mizar.png)

* Virtual Private Cloud (VPC) domain: A flat-network of endpoints specific to a
  single tenant.
* Networks within a VPC: a group of Endpoints within a VPC. An operator may
  identify Networks as subnets of the VPC address space or any other
  partitioning scheme.
* Endpoint within a Network: the group of endpoints forming a network. Endpoints
  of a network have IP addresses from the VPC address space and need not have IP
  address of one subnet.

Traditionally routing between VPCs and subnets is managed by virtual switches
and routers. These mandates, for example, that endpoints belong to the same
subnets, and a network of endpoints must have a subnet address. Mizar does not
have this restriction.

Mizar, introduces new **abstract** components called **Bouncers** and
**Dividers**. Bouncers and Dividers are in-network and horizontally scalable
hash tables that facilitate packet delivery between endpoints. The
management-plane populates the Bouncers and Dividers tables according to network
domain partitioning.

Bouncers' decision domain is constrained to a network. A Bouncer holds the
configuration of endpoints within a network. When a packet arrives at a Bouncer,
it is expected to find the destination endpoint's host and __bounce__ the packet
back to the host. Unlike a switch - where packet forwarding is performed by L2
learning - Bouncer's configuration maintains a mapping of an endpoint within a
VPC to its host. The endpoint is identified by its IP address within a VPC
(VNI). Bouncers rewrite the destination IP address of the outer packet to the
endpoint's host.

Dividers' decision domain is constrained to VPCs. A Divider holds the
configuration of all networks within a VPC; hence it divides (shards) the
traffic inside the VPC across multiple bouncers. Dividers do not maintain
endpoint-to-host mapping information. When a divider receives a packet, it
determines which bouncer has the host information of the destination endpoint
**according to the network partitioning logic** and rewrites the destination IP
of the outer packet to the bouncer.

This overall architecture allows - among many advantages - to accelerate
endpoints provisioning, as the management plane programs a finite number of
hosts designated as Bouncers instead of propagating the endpoint configuration
to each host.

## Learn more about Mizar

* [*Documentation*](https://mizar.readthedocs.io/en/latest/)
* [*Data-plane*](docs/design/dp_overview.md)
* [*Management Plane*](docs/design/mp_overview.md)

### Attend Mizar's Weekly Open Source Meeting
- Thursday at 6:00PM - 8:00PM Pacific Time (weekly)
- [**Zoom Meeting**](https://futurewei.zoom.us/j/421246133) 
- [**Meeting Minutes**](https://docs.google.com/document/d/1cz9j_QFriaM8DRBTP8HBhg8bm9NIrLl8MlZ0iQCDqUM/edit) 

### Join the Mizar Community

- [**Slack**](https://join.slack.com/t/mizar-group/shared_invite/zt-9md5gw7m-Fp8n1iV9H04Nn8P3lwTEMA)
- [**Email Group**](https://groups.google.com/forum/#!forum/mizar-cloud-networking)
