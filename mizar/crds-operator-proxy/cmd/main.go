package main

import (
	"fmt"
	"os"

	"k8s.io/klog"
        config "mizar.com/crds-operator-proxy/config"
	vpccontroller "mizar.com/crds-operator-proxy/crds/vpcs/controller"
	bouncercontroller "mizar.com/crds-operator-proxy/crds/bouncers/controller"
	dividercontroller "mizar.com/crds-operator-proxy/crds/dividers/controller"
	dropletcontroller "mizar.com/crds-operator-proxy/crds/droplets/controller"
	endpointcontroller "mizar.com/crds-operator-proxy/crds/endpoints/controller"
	netcontroller "mizar.com/crds-operator-proxy/crds/nets/controller"
)

func main() {
	args := os.Args[1:]
        config.Server_addr = args[0]
        fmt.Println(config.Server_addr)
	//vpcs
	vpcproxy := vpccontroller.NewController()
	vpcproxy.CreateCRD()
	if err := vpcproxy.CreateObject(); err != nil {
		klog.Infoln(err)
	}
	fmt.Print("vpc proxy starts"
	
	//bouncers
	bouncerproxy := bouncercontroller.NewController()
	bouncerproxy.CreateCRD()
	if err := bouncerproxy.CreateObject(); err != nil {
		klog.Infoln(err)
	}
	fmt.Print("bouncer proxy starts")
	
	//dividers
	dividerproxy := dividercontroller.NewController()
	dividerproxy.CreateCRD()
	if err := dividerproxy.CreateObject(); err != nil {
		klog.Infoln(err)
	}
	fmt.Print("divider proxy starts")
	
	//droplets
	dropletproxy := dropletcontroller.NewController()
	dropletproxy.CreateCRD()
	if err := dropletproxy.CreateObject(); err != nil {
		klog.Infoln(err)
	}
	fmt.Print("droplet proxy starts")
        
	//endpoints
	endpointproxy := endpointcontroller.NewController()
	endpointproxy.CreateCRD()
	if err := endpointproxy.CreateObject(); err != nil {
		klog.Infoln(err)
	}
	fmt.Print("endpoint proxy starts")
	
	//nets
	netproxy := netcontroller.NewController()
	netproxy.CreateCRD()
	if err := netproxy.CreateObject(); err != nil {
		klog.Infoln(err)
	}
	fmt.Print("net proxy starts")
        go vpcproxy.Run()
        go bouncerproxy.Run()
        go dividerproxy.Run()
  	go netproxy.Run()
	go endpointproxy.Run()
 	go dropletproxy.Run()
        fmt.Scanln()        	
}

