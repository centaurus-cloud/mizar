package v1

import metav1 "k8s.io/apimachinery/pkg/apis/meta/v1"

// These const variables are used in our custom controller.
const (
	GroupName string = "mizar.com"
	Kind      string = "Vpc"
	Version   string = "v1"
	Plural    string = "vpcs"
	Singluar  string = "vpc"
	ShortName string = "vpc"
	Name      string = Plural + "." + GroupName
)

// Vpcspec specifies the 'spec' of Vpc CRD.
// filed_name type tag (e.g: `json:"ip"`)
type VpcSpec struct {
	Ip             string `json:"ip"`
	Prefix         string `json:"prefix"`
	Vni            string `json:"vni"`
	Dividers       string `json:"dividers"`
	Status         string `json:"status"`
	CreateTime     string `json:"createtime"`
	ProvisionDelay string `json:"provisiondelay"`
}

// +genclient
// +k8s:deepcopy-gen:interfaces=k8s.io/apimachinery/pkg/runtime.Object

// Vpc describes a Vpc custom resource.
type Vpc struct {
	metav1.TypeMeta   `json:",inline"`
	metav1.ObjectMeta `json:"metadata,omitempty"`

	Spec   VpcSpec `json:"spec"`
	Status string  `json:"status"`
}

// +k8s:deepcopy-gen:interfaces=k8s.io/apimachinery/pkg/runtime.Object

// VpcList is a list of Vpc resources.
type VpcList struct {
	metav1.TypeMeta `json:",inline"`
	metav1.ListMeta `json:"metadata"`

	Items []Vpc `json:"items"`
}

