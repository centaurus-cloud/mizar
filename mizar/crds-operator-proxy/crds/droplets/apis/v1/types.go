package v1

import metav1 "k8s.io/apimachinery/pkg/apis/meta/v1"

// These const variables are used in our custom controller.
const (
	GroupName string = "mizar.com"
	Kind      string = "Droplet"
	Version   string = "v1"
	Plural    string = "droplets"
	Singluar  string = "droplet"
	ShortName string = "drp"
	Name      string = Plural + "." + GroupName
)

// DropletSpec specifies the 'spec' of Droplet CRD.
// filed_name type tag (e.g: `json:"ip"`)
type DropletSpec struct {
	Mac             string `json:"mac"`
	Ip             	string `json:"ip"`
	Status         	string `json:"status"`
	Interface       string `json:"interface"`
	CreateTime     	string `json:"createtime"`
	ProvisionDelay 	string `json:"provisiondelay"`
}

// +genclient
// +k8s:deepcopy-gen:interfaces=k8s.io/apimachinery/pkg/runtime.Object

// Vpc describes a Vpc custom resource.
type Droplet struct {
	metav1.TypeMeta   	`json:",inline"`
	metav1.ObjectMeta 	`json:"metadata,omitempty"`

	Spec   DropletSpec 	`json:"spec"`
	Status string  		`json:"status"`
}

// +k8s:deepcopy-gen:interfaces=k8s.io/apimachinery/pkg/runtime.Object

// VpcList is a list of Droplet resources.
type DropletList struct {
	metav1.TypeMeta `json:",inline"`
	metav1.ListMeta `json:"metadata"`

	Items []Droplet `json:"items"`
}
