package v1

import metav1 "k8s.io/apimachinery/pkg/apis/meta/v1"

// These const variables are used in our custom controller.
const (
	GroupName string = "mizar.com"
	Kind      string = "Endpoint"
	Version   string = "v1"
	Plural    string = "endpoints"
	Singluar  string = "endpoint"
	ShortName string = "ep"
	Name      string = Plural + "." + GroupName
)

// Vpcspec specifies the 'spec' of Endpoint CRD.
// filed_name type tag (e.g: `json:"ip"`)
type EndpointSpec struct {
	Type           string `json:"type"`
	Mac            string `json:"mac"`
	Ip             string `json:"ip"`
	Gw             string `json:"gw"`
	Status         string `json:"status"`
	Network        string `json:"network"`
	Vpc            string `json:"vpc"`
	Vni            string `json:"vni"`
	Droplet        string `json:"droplet"`
	Interface      string `json:"interface"`
	Veth           string `json:"veth"`
	HostIp         string `json:"hostip"`
	HostMac        string `json:"hostmac"`
	CreateTime     string `json:"createtime"`
	ProvisionDelay string `json:"provisiondelay"`
	CniDelay       string `json:"cnidelay"`
}

// +genclient
// +k8s:deepcopy-gen:interfaces=k8s.io/apimachinery/pkg/runtime.Object

// Endpoint describes a Endpoint custom resource.
type Endpoint struct {
	metav1.TypeMeta   	`json:",inline"`
	metav1.ObjectMeta 	`json:"metadata,omitempty"`

	Spec   EndpointSpec `json:"spec"`
	Status string  		`json:"status"`
}

// +k8s:deepcopy-gen:interfaces=k8s.io/apimachinery/pkg/runtime.Object

// EndpointList is a list of Endpoint resources.
type EndpointList struct {
	metav1.TypeMeta `json:",inline"`
	metav1.ListMeta `json:"metadata"`

	Items []Endpoint `json:"items"`
}

