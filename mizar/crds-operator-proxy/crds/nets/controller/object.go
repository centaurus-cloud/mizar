package controller

import (
	"fmt"
	"log"

	corev1 "k8s.io/api/core/v1"
	metav1 "k8s.io/apimachinery/pkg/apis/meta/v1"
	netv1 "mizar.com/crds-operator-proxy/crds/nets/apis/v1"
)

// Create a Vpc object.
func (c *Controller) CreateObject() error {
	object := &netv1.Net{
		ObjectMeta: metav1.ObjectMeta{
			Name:      "net1",
			Namespace: corev1.NamespaceDefault,
		},
		Spec: netv1.NetSpec{
			Ip:             "10.0.0.1",
			Prefix:         "10.0.0.0",
			Vni:            "Vni1",
			Vpc:       		"Vpc1",
			Status:         "Status1",
			Bouncers:       "Bouncers1",
			CreateTime:     "CreateTime1",
			ProvisionDelay: "ProvisionDelay1",
		},
	}
	_, err := c.netclientset.MizarV1().Nets(corev1.NamespaceDefault).Create(object)
	errorMessage := err
	if err != nil {
		log.Fatalf("could not create: %v", errorMessage)
	}
	log.Printf("Created Net: %s", object.Name)
	fmt.Printf("Created Net %s", object.Name)
	return err
}

