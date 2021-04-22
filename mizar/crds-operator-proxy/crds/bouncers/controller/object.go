package controller

import (
	"fmt"
	"log"

	corev1 "k8s.io/api/core/v1"
	metav1 "k8s.io/apimachinery/pkg/apis/meta/v1"
	bouncerv1 "mizar.com/crds-operator-proxy/crds/bouncers/apis/v1"
)

// Create a Bouncer object.
func (c *Controller) CreateObject() error {
	object := &bouncerv1.Bouncer{
		ObjectMeta: metav1.ObjectMeta{
			Name:      "bouncer1",
			Namespace: corev1.NamespaceDefault,
		},
		Spec: bouncerv1.BouncerSpec{
			Vpc:            "vpc1",
			Net:            "net1",
			Ip:             "10.0.0.1",
			Mac:            "AA-BB-CC-DD-00-11",
			Droplet:        "droplet1",
			Status:         "Status1",
			CreateTime:     "CreateTime1",
			ProvisionDelay: "ProvisionDelay1",
		},
	}
	_, err := c.bouncerclientset.MizarV1().Bouncers(corev1.NamespaceDefault).Create(object)
	errorMessage := err
	if err != nil {
		log.Fatalf("could not create: %v", errorMessage)
	}
	log.Printf("Created Bouncer: %s", object.Name)
	fmt.Printf("Created Bouncer %s", object.Name)
	return err
}
