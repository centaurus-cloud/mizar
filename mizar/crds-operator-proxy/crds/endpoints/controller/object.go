package controller

import (
	"fmt"
	"log"

	corev1 "k8s.io/api/core/v1"
	metav1 "k8s.io/apimachinery/pkg/apis/meta/v1"
	endpointv1 "mizar.com/crds-operator-proxy/crds/endpoints/apis/v1"
)

// Create a Endpoint object.
func (c *Controller) CreateObject() error {
	object := &endpointv1.Endpoint{
		ObjectMeta: metav1.ObjectMeta{
			Name:      "endpoint1",
			Namespace: corev1.NamespaceDefault,
		},
		Spec: endpointv1.EndpointSpec{
			Type:           "Type1",
			Mac:            "AA-BB-CC-00-11-22",
			Ip:             "10.0.0.1",
			Gw:             "Gw1",
			Status:         "Status1",
			Network:        "Network1",
			Vpc:            "Vpc1",
			Vni:            "Vni1",
			Droplet:        "Droplet1",
			Interface:      "Interface1",
			Veth:           "Veth1",
			HostIp:         "172.0.0.1",
			HostMac:        "AA-BB-CC-00-00-00",
			CreateTime:     "CreateTime1",
			ProvisionDelay: "ProvisionDelay1",
			CniDelay:       "CniDelay1",
		},
	}
	_, err := c.endpointclientset.MizarV1().Endpoints(corev1.NamespaceDefault).Create(object)
	errorMessage := err
	if err != nil {
		log.Fatalf("could not create: %v", errorMessage)
	}
	log.Printf("Created Endpoint: %s", object.Name)
	fmt.Printf("Created Endpoint %s", object.Name)
	return err
}
