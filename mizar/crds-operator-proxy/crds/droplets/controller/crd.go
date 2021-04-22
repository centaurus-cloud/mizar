package controller

import (
	"fmt"
	"time"

	apiextensions "k8s.io/apiextensions-apiserver/pkg/apis/apiextensions/v1beta1"
	metav1 "k8s.io/apimachinery/pkg/apis/meta/v1"
	"k8s.io/apimachinery/pkg/util/wait"
	"k8s.io/klog"
	dropletv1 "mizar.com/crds-operator-proxy/crds/droplets/apis/v1"
)

func (c *Controller) doesCRDExist() (bool, error) {
	crd, err := c.apiextensionsclientset.ApiextensionsV1beta1().CustomResourceDefinitions().Get(dropletv1.Name, metav1.GetOptions{})

	if err != nil {
		return false, err
	}

	// Check whether the CRD is accepted.
	for _, condition := range crd.Status.Conditions {
		if condition.Type == apiextensions.Established &&
			condition.Status == apiextensions.ConditionTrue {
			return true, nil
		}
	}

	return false, fmt.Errorf("CRD is not accepted")
}

func (c *Controller) waitCRDAccepted() error {
	err := wait.Poll(1*time.Second, 10*time.Second, func() (bool, error) {
		return c.doesCRDExist()
	})

	return err
}

// CreateCRD creates a custom resource definition, Vpc.
func (c *Controller) CreateCRD() error {
	if result, _ := c.doesCRDExist(); result {
		return nil
	}

	crd := &apiextensions.CustomResourceDefinition{
		ObjectMeta: metav1.ObjectMeta{
			Name: dropletv1.Name,
		},
		Spec: apiextensions.CustomResourceDefinitionSpec{
			Group:   dropletv1.GroupName,
			Version: dropletv1.Version,
			Scope:   apiextensions.NamespaceScoped,
			Names: apiextensions.CustomResourceDefinitionNames{
				Plural:     dropletv1.Plural,
				Singular:   dropletv1.Singluar,
				Kind:       dropletv1.Kind,
				ShortNames: []string{dropletv1.ShortName},
			},
			Validation: &apiextensions.CustomResourceValidation{
				OpenAPIV3Schema: &apiextensions.JSONSchemaProps{
					Type: "object",
					Properties: map[string]apiextensions.JSONSchemaProps{
						"spec": {
							Type: "object",
							Properties: map[string]apiextensions.JSONSchemaProps{
								"mac":             	{Type: "string"},
								"ip":         		{Type: "string"},
								"status":           {Type: "string"},
								"interface":       	{Type: "string"},
								"createtime":     	{Type: "string"},
								"provisiondelay": 	{Type: "string"},
							},
							Required: []string{"mac", "ip", "interface"},
						},
					},
				},
			},
			AdditionalPrinterColumns: []apiextensions.CustomResourceColumnDefinition{
				{
					Name:     "ip",
					Type:     "string",
					JSONPath: ".spec.ip",
				},
				{
					Name:     "prefix",
					Type:     "string",
					JSONPath: ".spec.prefix",
				},
				{
					Name:     "vni",
					Type:     "string",
					JSONPath: ".spec.vni",
				},
				{
					Name:     "dividers",
					Type:     "string",
					JSONPath: ".spec.dividers",
				},
				{
					Name:     "status",
					Type:     "string",
					JSONPath: ".spec.status",
				},
				{
					Name:     "createtime",
					Type:     "string",
					JSONPath: ".spec.createtime",
				},
				{
					Name:     "provisiondelay",
					Type:     "string",
					JSONPath: ".spec.provisiondelay",
				},
			},
		},
	}

	_, err := c.apiextensionsclientset.ApiextensionsV1beta1().CustomResourceDefinitions().Create(crd)

	if err != nil {
		klog.Fatalf(err.Error())
	}

	return c.waitCRDAccepted()
}

