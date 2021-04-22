package v1

import metav1 "k8s.io/apimachinery/pkg/apis/meta/v1"

// These const variables are used in our custom controller.
const (
	GroupName string = "mizar.com"
	Kind      string = "Bouncer"
	Version   string = "v1"
	Plural    string = "bouncers"
	Singluar  string = "bouncer"
	ShortName string = "bncr"
	Name      string = Plural + "." + GroupName
)

// BouncerSpec specifies the 'spec' of Bouncer CRD.
// filed_name type tag (e.g: `json:"ip"`)
type BouncerSpec struct {
	Vpc            string `json:"vpc"`
	Net            string `json:"net"`
	Ip             string `json:"ip"`
	Mac            string `json:"mac"`
	Droplet        string `json:"droplet"`
	Status         string `json:"status"`
	CreateTime     string `json:"createtime"`
	ProvisionDelay string `json:"provisiondelay"`
}

// +genclient
// +k8s:deepcopy-gen:interfaces=k8s.io/apimachinery/pkg/runtime.Object

// Vpc describes a Vpc custom resource.
type Bouncer struct {
	metav1.TypeMeta   `json:",inline"`
	metav1.ObjectMeta `json:"metadata,omitempty"`

	Spec   BouncerSpec `json:"spec"`
	Status string      `json:"status"`
}

// +k8s:deepcopy-gen:interfaces=k8s.io/apimachinery/pkg/runtime.Object

// BouncerList is a list of Bouncer resources.
type BouncerList struct {
	metav1.TypeMeta `json:",inline"`
	metav1.ListMeta `json:"metadata"`

	Items []Bouncer `json:"items"`
}
