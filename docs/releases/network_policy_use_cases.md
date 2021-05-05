<!--
SPDX-License-Identifier: MIT
Copyright (c) 2021 The Authors.

Authors: Hong Chang         <@Hong-Chang>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:The above copyright
notice and this permission notice shall be included in all copies or
substantial portions of the Software.THE SOFTWARE IS PROVIDED "AS IS",
WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
THE USE OR OTHER DEALINGS IN THE SOFTWARE.
-->

In release 0.7, Mizar addes support for Kubernetes Network Policy. Following are the user cases we performed and passed.

## 1. Policy Ingress Rule
| Steps | Expected Result |
|----------|--------------|
| (P1) Create Network Policy with: Ingress Rule: ipBlock Defined, no except | <li>Ingress traffic from Pods within ipBlock: Passed </br><li>Ingress traffic from Pods Not within ipBlock: Blocked |
| (P1) Ingress Rule: ipBlock Defined, with except | <li>Ingress traffic from Pods within except of ipBlock: Blocked </br><li>Ingress traffic from Pods Not within except of ipBlock: Passed |
| (P1) Ingress Rule: podSelector | <li>Ingress traffic from selected Pods: Passed </br><li>Ingress traffic from Not-selected Pods: Blocked |
| Ingress Rule: namespaceSelector | <li>Ingress traffic from pods in selected namespace: Passed </br><li>Ingress traffic from pods Not in selected namespace: Blocked |
| (P1) Ingress Rule: podSelect + namespaceSelector | <li>Ingress traffic from selected pods and in selected namespace: Passed </br><li>Ingress traffic Not from combination of selected pods and selected namespace: Blocked |
| (P1) Ingress ports: protocol and port | <li>Ingress traffic from the protocol and port: Passed </br><li>Ingress traffic Not from combination of the protocol and port: Blocked |
| (P1) Two ingress rules | <li>Two rules are in “Or” relationship and traffic pass or blocked as expected |
| (P1) Two policies, their cidrs are in contained relation | <li>For a certain cidr, two policies will be returned |

## 2. Policy Egress Rule
| Steps | Expected Result |
|----------|--------------|
| (P1) Create Network Policy with: Egress Rule: ipBlock Defined, no except | <li>Egress traffic to Pods within ipBlock: Passed </br><li>Egress traffic to Pods Not within ipBlock: Blocked |
| (P1) Egress Rule: ipBlock Defined, with except | <li>Egress traffic to Pods within except of ipBlock: Blocked </br><li>Egress traffic to Pods Not within except of ipBlock: Passed |
| (P1) Egress Rule: podSelector | <li>Egress traffic to selected Pods: Passed </br><li>Egress traffic to Not-selected Pods: Blocked |
| (P1) Egress Rule: namespaceSelector | <li>Egress traffic to pods in selected namespace: Passed </br><li>Egress traffic to pods Not in selected namespace: Blocked |
| (P1) Egress Rule: podSelect + namespaceSelector | <li>Egress traffic to selected pods and in selected namespace: Passed </br><li>Egress traffic Not to combination of selected pods and selected namespace: Blocked |
| (P1) Egress ports: protocol and port | <li>Egress traffic to the protocol and port: Passed </br><li>Egress traffic Not to combination of the protocol and port: Blocked |

## 3. Policy PodSelector
| Steps | Expected Result |
|----------|--------------|
| (P1) Network Policy spec has podSelector defined with ingress rule | <li>Selected pods have ingress traffic blocked </br><li>Other pods’ ingress traffic are not blocked |
| (P1) Network Policy spec has podSelector defined with egress rule | <li>Selected pods have ingress traffic blocked </br><li>Other pods’ ingress traffic are not blocked |

## 4. Default Policies
| Steps | Expected Result |
|----------|--------------|
| (P1) Default deny all ingress traffic in a namespace | <li>All ingress traffic blocked in the policy’s namespace </br><li>Other namespaces are not affected |
| Default allow all ingress traffic in a namespace | <li>All ingress traffic passed in the policy’s namespace </br><li>Other namespaces are not affected |
| Default deny all egress traffic in a namespace | <li>All egress traffic blocked in the policy’s namespace </br><li>Other namespaces are not affected |
| (P1) Default allow all egress traffic in a namespace | <li>All egress traffic passed in the policy’s namespace </br><li>Other namespaces are not affected |

## 5. Policy Updating
| Steps | Expected Result |
|----------|--------------|
| (P1) Update Policy, spec.podSelector changed | <li>De-selected pods are not longer be ruled by the policy </br><li>Newly-selected pods are ruled by the policy |
| (P1) Update Policy, ingress, ipBlock add, changed, or removed | <li>Traffic is ruled as expected, both for deleted rule and newly added rule |
| (P1) Update Policy, ingress, podSelector add, changed, or removed | <li>Traffic is ruled as expected, both for deleted rule and newly added rule |
| Update Policy, ingress, namespaceSelector add, changed, or removed | <li>Traffic is ruled as expected, both for deleted rule and newly added rule |
| Update Policy, ingress, podSelector + namespaceSelector add, changed, or removed | <li>Traffic is ruled as expected, both for deleted rule and newly added rule |
| Update Policy, ingress, port add, changed, or removed | <li>Traffic is ruled as expected, both for deleted rule and newly added rule |
| (P1) Update Policy, egress, ipBlock add, changed, or removed | <li>Traffic is ruled as expected, both for deleted rule and newly added rule |
| (P1) Update Policy, egress, podSelector add, changed, or removed | <li>Traffic is ruled as expected, both for deleted rule and newly added rule |
| (P1) Update Policy, egress, namespaceSelector add, changed, or removed | <li>Traffic is ruled as expected, both for deleted rule and newly added rule |
| Update Policy, egress, podSelector + namespaceSelector add, changed, or removed | <li>Traffic is ruled as expected, both for deleted rule and newly added rule |
| (P1) Update Policy, egress, port add, changed, or removed | <li>Traffic is ruled as expected, both for deleted rule and newly added rule |
| (P1) Create an ingress only policy. Then update the policy to be egress only | <li>First the policy has ingress effect, then the policy has egress effect |
| Create an network policy that has cidr, namespace selector, pod selector and protocol port. </br>Then remove cidr portion or remove pod selector portion and update network policy | <li>Traffic is ruled as expected, both for deleted rule and newly added rule |

## 6. Policy Deleting
| Steps | Expected Result |
|----------|--------------|
| (P1) Delete Policy | <li>All the effects from the policy are gone |

## 7. Pod Creating
| Steps | Expected Result |
|----------|--------------|
| (P1) New pod created, it fits for Policy.spec.podSelector | <li>The pod begins to take effect on the policy |
| (P1) New pod created, it fits for ingress/egress ipBlock | <li>Traffic ingress from or egress to the pod will be affected |
| New pod created, it fits for ingress/egress ipBlock’s except part | <li>Traffic ingress from or egress to the pod will be affected |
| (P1) New pod created, it fits for ingress/egress podSelector | <li>Traffic ingress from or egress to the pod will be affected |
| New pod created, its namespace is fits for ingress/egress namespaceSelector | <li>Traffic ingress from or egress to the pod will be affected |

## 8. Pod Updating
| Steps | Expected Result |
|----------|--------------|
| Pod added/updated label, it fits for Policy.spec.podSelector | <li>The pod begins to take effect on the policy |
| Pod updated/deleted label, it no longer fits for Policy.spec.podSelector | <li>The pod stops to take effect on the policy |
| (P1) Pod added/updated label, it fits for ingress/egress podSelector | <li>Traffic ingress from or egress to the pod will be affected |
| Pod updated/deleted label, it no longer fits for ingress/egress podSelector | <li>Traffic ingress from or egress to the pod will stop to be affected |
| Pod added/updated label, it fits for ingress/egress podSelector + namespaceSelector | <li>Traffic ingress from or egress to the pod will be affected |
| Pod updated/deleted label, it no longer fits for ingress/egress podSelector + namespaceSelector | <li>Traffic ingress from or egress to the pod will stop to be affected |

## 9. Pod Deleting
| Steps | Expected Result |
|----------|--------------|
| (P1) Pod deleted, it fits for Policy.spec.podSelector | <li>The endpoint of the pod stops to take effect on the policy |
| (P1) Pod deleted, it fits for ingress/egress podSelector | <li>All the policy’s endpoints will update data |
| (P1) Pod deleted, it fits for ingress/egress namespaceSelector | <li>All the policy’s endpoints will update data |

## 10. Namespace Updating
| Steps | Expected Result |
|----------|--------------|
| (P1) Namespace add/update label, new label fit for namespaceSelector | <li>All the policy’s endpoints will update data to add pods under the namespace </br><li>Traffic ingress from or egress to the pods under the namespace will be affected |
| (P1) Namespace add/update label, old label fit for namespaceSelector | <li>All the policy’s endpoints will update data to remove pods under the namespace </br><li>Traffic ingress from or egress to the pods under the namespace will stop to be affected |
| (P1) Namespace add/update label, new label fit for podSelector + namespaceSelector | <li>All the policy’s endpoints will update data to add selected pods under the namespace </br><li>Traffic ingress from or egress to the selected pods under the namespace will be affected |
| (P1) Namespace add/update label, old label fit for podSelector + namespaceSelector | <li>All the policy’s endpoints will update data to remove selected pods under the namespace </br><li>Traffic ingress from or egress to the selected pods under the namespace will stop to be affected |

## 11. Service Scenarios
| Steps | Expected Result |
|----------|--------------|
| (P1) Egress traffic, policy describes pods by ipBlock, then visit the pods through its service ip | <li>Traffic blocked |
| (P1) Add service ip into ipBlock | <li>Traffic passed |

## 12. Connection Tracking
| Steps | Expected Result |
|----------|--------------|
| Create a policy that allows a specific IP access. Try to start a connection | <li>Check the connection tracking map from user space. The item should be there, and the status is allowed (0 - tcp, 4 - udp) |
| Modify the policy to not allow the IP; try to send more data | <li>Check the connection tracking map from user space: the status is denied (1 - tcp, 5 - udp) |
| Create a policy that denies a specific IP access; try to start the connection and send data | <li>Check the connection tracking map from user space. The item should be there, and the status is denied(1 - tcp, 5 - udp) |
| Modify the policy to allow the IP; try to send data | <li>Check the connection tracking map from user space: the status is allowed(0 - tcp, 4 - udp) |
