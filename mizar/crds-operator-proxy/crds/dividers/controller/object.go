package controller

import (
	"fmt"
	"log"

	corev1 "k8s.io/api/core/v1"
	metav1 "k8s.io/apimachinery/pkg/apis/meta/v1"
	dividerv1 "mizar.com/crds-operator-proxy/crds/dividers/apis/v1"
)

// Create a Vpc object.
func (c *Controller) CreateObject() error {
	object := &dividerv1.Divider{
		ObjectMeta: metav1.ObjectMeta{
			Name:      "divider1",
			Namespace: corev1.NamespaceDefault,
		},
		Spec: dividerv1.DividerSpec{
			Vpc:            "vpc1",
			Ip:             "10.0.0.1",
			Mac:            "AA-BB-CC-DD-00-11",
			Droplet:        "Droplet1",
			Status:         "Status1",
			CreateTime:     "CreateTime1",
			ProvisionDelay: "ProvisionDelay1",
		},
	}
	_, err := c.dividerclientset.MizarV1().Dividers(corev1.NamespaceDefault).Create(object)
	errorMessage := err
	if err != nil {
		log.Fatalf("could not create: %v", errorMessage)
	}
	log.Printf("Created Divider: %s", object.Name)
	fmt.Printf("Created Divider %s", object.Name)
	return err
}
