package controller

import (
	"fmt"
	"time"

	apiextensions "k8s.io/apiextensions-apiserver/pkg/apis/apiextensions/v1beta1"
	metav1 "k8s.io/apimachinery/pkg/apis/meta/v1"
	"k8s.io/apimachinery/pkg/util/wait"
	"k8s.io/klog"
	endpointv1 "mizar.com/crds-operator-proxy/crds/endpoints/apis/v1"
)

func (c *Controller) doesCRDExist() (bool, error) {
	crd, err := c.apiextensionsclientset.ApiextensionsV1beta1().CustomResourceDefinitions().Get(endpointv1.Name, metav1.GetOptions{})

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
			Name: endpointv1.Name,
		},
		Spec: apiextensions.CustomResourceDefinitionSpec{
			Group:   endpointv1.GroupName,
			Version: endpointv1.Version,
			Scope:   apiextensions.NamespaceScoped,
			Names: apiextensions.CustomResourceDefinitionNames{
				Plural:     endpointv1.Plural,
				Singular:   endpointv1.Singluar,
				Kind:       endpointv1.Kind,
				ShortNames: []string{endpointv1.ShortName},
			},
			Validation: &apiextensions.CustomResourceValidation{
				OpenAPIV3Schema: &apiextensions.JSONSchemaProps{
					Type: "object",
					Properties: map[string]apiextensions.JSONSchemaProps{
						"spec": {
							Type: "object",
							Properties: map[string]apiextensions.JSONSchemaProps{
								"type":           {Type: "string"},
								"mac":            {Type: "string"},
								"ip":             {Type: "string"},
								"gw":             {Type: "string"},
								"status":         {Type: "string"},
								"network":        {Type: "string"},
								"vpc":            {Type: "string"},
								"vni":            {Type: "string"},
								"droplet":        {Type: "string"},
								"interface":      {Type: "string"},
								"veth":           {Type: "string"},
								"hostip":         {Type: "string"},
								"hostmac":        {Type: "string"},
								"createtime":     {Type: "string"},
								"provisiondelay": {Type: "string"},
								"cnidelay":       {Type: "string"},
							},
							Required: []string{"type", "mac", "ip"},
						},
					},
				},
			},
			AdditionalPrinterColumns: []apiextensions.CustomResourceColumnDefinition{
				{
					Name:     "type",
					Type:     "string",
					JSONPath: ".spec.type",
				},
				{
					Name:     "mac",
					Type:     "string",
					JSONPath: ".spec.mac",
				},
				{
					Name:     "ip",
					Type:     "string",
					JSONPath: ".spec.ip",
				},

				{
					Name:     "gw",
					Type:     "string",
					JSONPath: ".spec.gw",
				},
				{
					Name:     "staus",
					Type:     "string",
					JSONPath: ".spec.status",
				},
				{
					Name:     "network",
					Type:     "string",
					JSONPath: ".spec.network",
				},
				{
					Name:     "vpc",
					Type:     "string",
					JSONPath: ".spec.vpc",
				},
				{
					Name:     "vni",
					Type:     "string",
					JSONPath: ".spec.vni",
				},
				{
					Name:     "droplet",
					Type:     "string",
					JSONPath: ".spec.droplet",
				},
				{
					Name:     "interface",
					Type:     "string",
					JSONPath: ".spec.interface",
				},
				{
					Name:     "veth",
					Type:     "string",
					JSONPath: ".spec.veth",
				},
				{
					Name:     "hostip",
					Type:     "string",
					JSONPath: ".spec.hostip",
				},
				{
					Name:     "hostmac",
					Type:     "string",
					JSONPath: ".spec.hostmac",
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
				{
					Name:     "cnidelay",
					Type:     "string",
					JSONPath: ".spec.cnidelay",
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

