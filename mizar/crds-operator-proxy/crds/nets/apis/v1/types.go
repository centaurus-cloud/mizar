package v1

import metav1 "k8s.io/apimachinery/pkg/apis/meta/v1"

// These const variables are used in our custom controller.
const (
	GroupName string = "mizar.com"
	Kind      string = "Net"
	Version   string = "v1"
	Plural    string = "nets"
	Singluar  string = "net"
	ShortName string = "net"
	Name      string = Plural + "." + GroupName
)

// NetSpec specifies the 'spec' of Net CRD.
// filed_name type tag (e.g: `json:"ip"`)
type NetSpec struct {
	Ip             string `json:"ip"`
	Prefix         string `json:"prefix"`
	Vni            string `json:"vni"`
	Vpc            string `json:"dividers"`
	Status         string `json:"status"`
	Bouncers       string `json:"bouncers"`
	CreateTime     string `json:"createtime"`
	ProvisionDelay string `json:"provisiondelay"`
}

// +genclient
// +k8s:deepcopy-gen:interfaces=k8s.io/apimachinery/pkg/runtime.Object

// Net describes a Net custom resource.
type Net struct {
	metav1.TypeMeta   `json:",inline"`
	metav1.ObjectMeta `json:"metadata,omitempty"`

	Spec   NetSpec `json:"spec"`
	Status string  `json:"status"`
}

// +k8s:deepcopy-gen:interfaces=k8s.io/apimachinery/pkg/runtime.Object

// NetList is a list of Net resources.
type NetList struct {
	metav1.TypeMeta `json:",inline"`
	metav1.ListMeta `json:"metadata"`

	Items []Net `json:"items"`
}
