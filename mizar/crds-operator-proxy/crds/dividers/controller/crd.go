package controller

import (
	"fmt"
	"time"

	apiextensions "k8s.io/apiextensions-apiserver/pkg/apis/apiextensions/v1beta1"
	metav1 "k8s.io/apimachinery/pkg/apis/meta/v1"
	"k8s.io/apimachinery/pkg/util/wait"
	"k8s.io/klog"
	dividerv1 "mizar.com/crds-operator-proxy/crds/dividers/apis/v1"
)

func (c *Controller) doesCRDExist() (bool, error) {
	crd, err := c.apiextensionsclientset.ApiextensionsV1beta1().CustomResourceDefinitions().Get(dividerv1.Name, metav1.GetOptions{})

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
			Name: dividerv1.Name,
		},
		Spec: apiextensions.CustomResourceDefinitionSpec{
			Group:   dividerv1.GroupName,
			Version: dividerv1.Version,
			Scope:   apiextensions.NamespaceScoped,
			Names: apiextensions.CustomResourceDefinitionNames{
				Plural:     dividerv1.Plural,
				Singular:   dividerv1.Singluar,
				Kind:       dividerv1.Kind,
				ShortNames: []string{dividerv1.ShortName},
			},
			Validation: &apiextensions.CustomResourceValidation{
				OpenAPIV3Schema: &apiextensions.JSONSchemaProps{
					Type: "object",
					Properties: map[string]apiextensions.JSONSchemaProps{
						"spec": {
							Type: "object",
							Properties: map[string]apiextensions.JSONSchemaProps{
								"vpc":            {Type: "string"},
								"ip":             {Type: "string"},
								"mac":            {Type: "string"},
								"droplet":        {Type: "string"},
								"status":         {Type: "string"},
								"createtime":     {Type: "string"},
								"provisiondelay": {Type: "string"},
							},
							Required: []string{"vpc", "ip", "mac"},
						},
					},
				},
			},
			AdditionalPrinterColumns: []apiextensions.CustomResourceColumnDefinition{
				{
					Name:     "vpc",
					Type:     "string",
					JSONPath: ".spec.vpc",
				},
				{
					Name:     "ip",
					Type:     "string",
					JSONPath: ".spec.ip",
				},
				{
					Name:     "mac",
					Type:     "string",
					JSONPath: ".spec.mac",
				},
				{
					Name:     "droplet",
					Type:     "string",
					JSONPath: ".spec.droplet",
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
