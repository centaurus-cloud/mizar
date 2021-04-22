package controller

import (
	"context"
	"fmt"
	"log"
	"time"

	"google.golang.org/grpc"

	corev1 "k8s.io/api/core/v1"
	utilruntime "k8s.io/apimachinery/pkg/util/runtime"
	"k8s.io/client-go/tools/cache"
	"k8s.io/klog"
	endpointv1 "mizar.com/crds-operator-proxy/crds/endpoints/apis/v1"
	pb "mizar.com/crds-operator-proxy/grpc/endpoints"
)

const (
	address1      = "localhost:50051"
	statusMessage = "HANDLED"
	CREATE        = 1
	UPDATE        = 2
	DELETE        = 3
	READ          = 4
	RESUME        = 5
)

// Run first sync client-go cache by calling cache.WaitforCacheSync,
// then block the main thread forever.
// Event handler will run in another Goroutine,
// generated in c.NewController() function.
func (c *Controller) Run() {
	defer utilruntime.HandleCrash()
	defer c.workqueue.ShutDown()

	klog.Infoln("Waiting cache to be synced.")
	// Handle timeout for syncing.
	timeout := time.NewTimer(time.Second * 30)
	timeoutCh := make(chan struct{})
	go func() {
		<-timeout.C
		timeoutCh <- struct{}{}
	}()
	if ok := cache.WaitForCacheSync(timeoutCh, c.informer.HasSynced); !ok {
		klog.Fatalln("Timeout expired during waiting for caches to sync.")
	}

	klog.Infoln("Starting custom controller.")
	select {}
}

func (c *Controller) objectAddedCallback(object interface{}) {
	resource := object.(*endpointv1.Endpoint)
	klog.Infof("Added: %v", resource)
	// If the object is in the desired state, end callback.
	if resource.Status == statusMessage {
		return
	}

	// If the object is not handled yet, handle it by modifying its Status.
	copy := resource.DeepCopy()
	copy.Status = statusMessage
	_, err := c.endpointclientset.MizarV1().Endpoints(corev1.NamespaceDefault).Update(copy)
	if err != nil {
		klog.Errorf(err.Error())
		return
	}
	gRPCRequest(CREATE, object)

	c.recorder.Event(copy, corev1.EventTypeNormal, "ObjectHandled", "Object is handled by custom controller.")
}

func (c *Controller) objectUpdatedCallback(oldObject, newObject interface{}) {
	//klog.Infof("Updated: %v", object)
	resource := newObject.(*endpointv1.Endpoint)
	klog.Infof("Updated: %v", resource)
	gRPCRequest(UPDATE, newObject)
	// If the object is in the desired state, end callback.
	if resource.Status == statusMessage {
		return
	}

	// If the object is not handled yet, handle it by modifying its Status.
	copy := resource.DeepCopy()
	copy.Status = statusMessage
	_, err := c.endpointclientset.MizarV1().Endpoints(corev1.NamespaceDefault).Update(copy)
	if err != nil {
		klog.Errorf(err.Error())
		return
	}

	c.recorder.Event(copy, corev1.EventTypeNormal, "ObjectHandled", "Object is handled by custom controller.")
}

func (c *Controller) objectDeletedCallback(object interface{}) {
	//klog.Infof("Deleted: %v", object)
	resource := object.(*endpointv1.Endpoint)
	klog.Infof("Deleted: %v", resource)
	// If the object is in the desired state, end callback.
	if resource.Status == statusMessage {
		return
	}

	// If the object is not handled yet, handle it by modifying its Status.
	copy := resource.DeepCopy()
	copy.Status = statusMessage
	_, err := c.endpointclientset.MizarV1().Endpoints(corev1.NamespaceDefault).Update(copy)
	if err != nil {
		klog.Errorf(err.Error())
		return
	}
	gRPCRequest(DELETE, object)

	c.recorder.Event(copy, corev1.EventTypeNormal, "ObjectHandled", "Object is handled by custom controller.")
}

func gRPCRequest(command int, object interface{}) {
	// Set up a connection to the server.
	fmt.Println("command=%d", command)
	conn, err := grpc.Dial(address1, grpc.WithInsecure(), grpc.WithBlock())
	if err != nil {
		log.Fatalf("did not connect: %v", err)
	}
	defer conn.Close()
	clientcon := pb.NewEndpointsServiceClient(conn)
	// Contact the server and request crd services.
	ctx, cancel := context.WithTimeout(context.Background(), time.Second)
	defer cancel()
	switch command {
	case 1:
		objectin := object.(*endpointv1.Endpoint)
		objectspec := objectin.Spec
		var resource pb.Endpoint
		resource = pb.Endpoint{Type: string(objectspec.Type), 
                        Mac: string(objectspec.Mac), 
                        Ip: string(objectspec.Ip), 
                        Gw: string(objectspec.Gw), 
			Status: string(objectspec.Status), 
			Network: string(objectspec.Network), 
			Vpc: string(objectspec.Vpc), 
			Vni: string(objectspec.Vni), 
			Droplet: string(objectspec.Droplet), 
			Interface: string(objectspec.Interface), 
			Veth: string(objectspec.Veth), 
			HostIp: string(objectspec.HostIp), 
			HostMac: string(objectspec.HostMac), 
			CreateTime: string(objectspec.CreateTime), 
                        ProvisionDelay: string(objectspec.ProvisionDelay),
			CniDelay: string(objectspec.CniDelay), }
		_, err = clientcon.CreateEndpoint(ctx, &resource)
	case 2:
		objectin := object.(*endpointv1.Endpoint)
		objectspec := objectin.Spec
		var resource pb.Endpoint
		resource = pb.Endpoint{Type: string(objectspec.Type), 
                        Mac: string(objectspec.Mac), 
                        Ip: string(objectspec.Ip), 
                        Gw: string(objectspec.Gw), 
			Status: string(objectspec.Status), 
			Network: string(objectspec.Network), 
			Vpc: string(objectspec.Vpc), 
			Vni: string(objectspec.Vni), 
			Droplet: string(objectspec.Droplet), 
			Interface: string(objectspec.Interface), 
			Veth: string(objectspec.Veth), 
			HostIp: string(objectspec.HostIp), 
			HostMac: string(objectspec.HostMac), 
			CreateTime: string(objectspec.CreateTime), 
                        ProvisionDelay: string(objectspec.ProvisionDelay),
			CniDelay: string(objectspec.CniDelay), }
		_, err = clientcon.UpdateEndpoint(ctx, &resource)
	case 3:
		objectin := object.(*endpointv1.Endpoint)
		objectName := objectin.Name
		_, err := clientcon.DeleteEndpoint(ctx, &pb.EndpointId{Id: objectName})
		if err != nil {
			log.Fatalf("%v", err)
			return
		}
	case 4:
		objectin := object.(*endpointv1.Endpoint)
		objectName := objectin.Name
		r, err := clientcon.ReadEndpoint(ctx, &pb.EndpointId{Id: objectName})
		log.Printf("Read Endpoint: %v", r)
		if err != nil {
			log.Fatalf("%v", err)
			return
		}
		if err != nil {
			log.Fatalf("%v", err)
			return
		}
	case 5:
		objectin := object.(*endpointv1.Endpoint)
		objectName := objectin.Name
		_, err := clientcon.ResumeEndpoint(ctx, &pb.EndpointId{Id: objectName})
		if err != nil {
			log.Fatalf("%v", err)
			return
		}
	default:
		log.Println("command is not correct")
	}
	//errorMessage = err
	if err != nil {
		log.Fatalf("could not create: %v", err)
	}
	log.Printf("Returned Client")
}

