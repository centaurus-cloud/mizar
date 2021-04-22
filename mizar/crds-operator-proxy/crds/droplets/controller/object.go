package controller

import (
	"fmt"
	"log"

	corev1 "k8s.io/api/core/v1"
	metav1 "k8s.io/apimachinery/pkg/apis/meta/v1"
	dropletv1 "mizar.com/crds-operator-proxy/crds/droplets/apis/v1"
)

// Create a Vpc object.
func (c *Controller) CreateObject() error {
	object := &dropletv1.Droplet{
		ObjectMeta: metav1.ObjectMeta{
			Name:      "droplet1",
			Namespace: corev1.NamespaceDefault,
		},
		Spec: dropletv1.DropletSpec{
			Mac:            "AA-BB-CC-00-11-22",
			Ip:             "10.0.0.1",
			Status:         "Status1",
			Interface:      "Interface1",
			CreateTime:     "CreateTime1",
			ProvisionDelay: "ProvisionDelay1",
		},
	}
	_, err := c.dropletclientset.MizarV1().Droplets(corev1.NamespaceDefault).Create(object)
	errorMessage := err
	if err != nil {
		log.Fatalf("could not create: %v", errorMessage)
	}
	log.Printf("Created Droplet: %s", object.Name)
	fmt.Printf("Created Droplet %s", object.Name)
	return err
}
