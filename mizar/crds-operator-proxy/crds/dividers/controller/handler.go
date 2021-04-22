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
	dividerv1 "mizar.com/crds-operator-proxy/crds/dividers/apis/v1"
	pb "mizar.com/crds-operator-proxy/grpc/dividers"
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
	resource := object.(*dividerv1.Divider)
	klog.Infof("Added: %v", resource)
	// If the object is in the desired state, end callback.
	if resource.Status == statusMessage {
		return
	}

	// If the object is not handled yet, handle it by modifying its Status.
	copy := resource.DeepCopy()
	copy.Status = statusMessage
	_, err := c.dividerclientset.MizarV1().Dividers(corev1.NamespaceDefault).Update(copy)
	if err != nil {
		klog.Errorf(err.Error())
		return
	}
	gRPCRequest(CREATE, object)

	c.recorder.Event(copy, corev1.EventTypeNormal, "ObjectHandled", "Object is handled by custom controller.")
}

func (c *Controller) objectUpdatedCallback(oldObject, newObject interface{}) {
	//klog.Infof("Updated: %v", object)
	resource := newObject.(*dividerv1.Divider)
	klog.Infof("Updated: %v", resource)
	gRPCRequest(UPDATE, newObject)
	// If the object is in the desired state, end callback.
	if resource.Status == statusMessage {
		return
	}

	// If the object is not handled yet, handle it by modifying its Status.
	copy := resource.DeepCopy()
	copy.Status = statusMessage
	_, err := c.dividerclientset.MizarV1().Dividers(corev1.NamespaceDefault).Update(copy)
	if err != nil {
		klog.Errorf(err.Error())
		return
	}

	c.recorder.Event(copy, corev1.EventTypeNormal, "ObjectHandled", "Object is handled by custom controller.")
}

func (c *Controller) objectDeletedCallback(object interface{}) {
	//klog.Infof("Deleted: %v", object)
	resource := object.(*dividerv1.Divider)
	klog.Infof("Deleted: %v", resource)
	// If the object is in the desired state, end callback.
	if resource.Status == statusMessage {
		return
	}

	// If the object is not handled yet, handle it by modifying its Status.
	copy := resource.DeepCopy()
	copy.Status = statusMessage
	_, err := c.dividerclientset.MizarV1().Dividers(corev1.NamespaceDefault).Update(copy)
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
	clientcon := pb.NewDividersServiceClient(conn)
	// Contact the server and request crd services.
	ctx, cancel := context.WithTimeout(context.Background(), time.Second)
	defer cancel()
	switch command {
	case 1:
		objectin := object.(*dividerv1.Divider)
		objectspec := objectin.Spec
		//fmt.Println("spec=%v", objectspec)
		var resource pb.Divider
		resource = pb.Divider{Vpc: string(objectspec.Vpc), Ip: string(objectspec.Ip), Mac: string(objectspec.Mac), Droplet: string(objectspec.Droplet), Status: string(objectspec.Status), CreateTime: string(objectspec.CreateTime), ProvisionDelay: string(objectspec.ProvisionDelay)}
		//clientcon.CreateVpc(ctx, &pb.Vpc{Ip: string(vpcspec.Ip), Prefix: "10.0.0.0", Vni: "16777210", Dividers: "2", Status: "active", CreateTime: "2020-07-20", ProvisionDelay: "20"})
		_, err = clientcon.CreateDivider(ctx, &resource)
	case 2:
		objectin := object.(*dividerv1.Divider)
		objectspec := objectin.Spec
		var resource pb.Divider
		resource = pb.Divider{Vpc: string(objectspec.Vpc), Ip: string(objectspec.Ip), Mac: string(objectspec.Mac), Droplet: string(objectspec.Droplet), Status: string(objectspec.Status), CreateTime: string(objectspec.CreateTime), ProvisionDelay: string(objectspec.ProvisionDelay)}
		_, err = clientcon.UpdateDivider(ctx, &resource)
	case 3:
		objectin := object.(*dividerv1.Divider)
		objectName := objectin.Name
		_, err := clientcon.DeleteDivider(ctx, &pb.DividerId{Id: objectName})
		if err != nil {
			log.Fatalf("%v", err)
			return
		}
	case 4:
		objectin := object.(*dividerv1.Divider)
		objectName := objectin.Name
		r, err := clientcon.ReadDivider(ctx, &pb.DividerId{Id: objectName})
		log.Printf("Read Divider: %v", r)
		if err != nil {
			log.Fatalf("%v", err)
			return
		}
		if err != nil {
			log.Fatalf("%v", err)
			return
		}
	case 5:
		objectin := object.(*dividerv1.Divider)
		objectName := objectin.Name
		_, err := clientcon.ResumeDivider(ctx, &pb.DividerId{Id: objectName})
		if err != nil {
			log.Fatalf("%v", err)
			return
		}
	default:
		log.Println("command is not correct")
	}
	if err != nil {
		log.Fatalf("could not create: %v", err)
	}
	log.Printf("Returned Client")
}
