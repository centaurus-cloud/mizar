package v1

import metav1 "k8s.io/apimachinery/pkg/apis/meta/v1"

// These const variables are used in our custom controller.
const (
	GroupName string = "mizar.com"
	Kind      string = "Divider"
	Version   string = "v1"
	Plural    string = "dividers"
	Singluar  string = "divider"
	ShortName string = "divd"
	Name      string = Plural + "." + GroupName
)

// DividerSpec specifies the 'spec' of Divider CRD.
// filed_name type tag (e.g: `json:"ip"`)
type DividerSpec struct {
	Vpc            string `json:"vpc"`
	Ip             string `json:"ip"`
	Mac            string `json:"mac"`
	Droplet        string `json:"droplet"`
	Status         string `json:"status"`
	CreateTime     string `json:"createtime"`
	ProvisionDelay string `json:"provisiondelay"`
}

// +genclient
// +k8s:deepcopy-gen:interfaces=k8s.io/apimachinery/pkg/runtime.Object

// Divider describes a Divider custom resource.
type Divider struct {
	metav1.TypeMeta   `json:",inline"`
	metav1.ObjectMeta `json:"metadata,omitempty"`

	Spec   DividerSpec `json:"spec"`
	Status string      `json:"status"`
}

// +k8s:deepcopy-gen:interfaces=k8s.io/apimachinery/pkg/runtime.Object

// DividerList is a list of Divider resources.
type DividerList struct {
	metav1.TypeMeta `json:",inline"`
	metav1.ListMeta `json:"metadata"`

	Items []Divider `json:"items"`
}
