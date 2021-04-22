package controller

import (
	"fmt"
	"log"

	corev1 "k8s.io/api/core/v1"
	metav1 "k8s.io/apimachinery/pkg/apis/meta/v1"
	vpcv1 "mizar.com/crds-operator-proxy/crds/vpcs/apis/v1"
)

// Create a Vpc object.
func (c *Controller) CreateObject() error {
	object := &vpcv1.Vpc{
		ObjectMeta: metav1.ObjectMeta{
			Name:      "vpc1",
			Namespace: corev1.NamespaceDefault,
		},
		Spec: vpcv1.VpcSpec{
			Ip:             "10.0.0.1",
			Prefix:         "10.0.0.0",
			Vni:            "vni1",
			Dividers:       "Dividers1",
			Status:         "Status1",
			CreateTime:     "CreateTime1",
			ProvisionDelay: "ProvisionDelay1",
		},
	}
	_, err := c.vpcclientset.MizarV1().Vpcs(corev1.NamespaceDefault).Create(object)
	errorMessage := err
	if err != nil {
		log.Fatalf("could not create: %v", errorMessage)
	}
	log.Printf("Created Vpc: %s", object.Name)
	fmt.Printf("Created Vpc %s", object.Name)
	return err
}
