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
	vpcv1 "mizar.com/crds-operator-proxy/crds/vpcs/apis/v1"
	pb "mizar.com/crds-operator-proxy/grpc/vpcs"
	config "mizar.com/crds-operator-proxy/config"
)

const (
	//address1      = "localhost:50051"
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
	resource := object.(*vpcv1.Vpc)
	klog.Infof("Added: %v", resource.Spec)
	// If the object is in the desired state, end callback.
	if resource.Status == statusMessage {
		return
	}

	// If the object is not handled yet, handle it by modifying its Status.
	copy := resource.DeepCopy()
	copy.Status = statusMessage
	_, err := c.vpcclientset.MizarV1().Vpcs(corev1.NamespaceDefault).Update(copy)
	if err != nil {
		klog.Errorf(err.Error())
		return
	}
	gRPCRequest(CREATE, object)

	c.recorder.Event(copy, corev1.EventTypeNormal, "ObjectHandled", "Object is handled by custom controller.")
}

func (c *Controller) objectUpdatedCallback(oldObject, newObject interface{}) {
	//klog.Infof("Updated: %v", object)
	resource := newObject.(*vpcv1.Vpc)
	klog.Infof("Updated: %v", resource.Spec)
	gRPCRequest(UPDATE, newObject)
	// If the object is in the desired state, end callback.
	if resource.Status == statusMessage {
		return
	}

	// If the object is not handled yet, handle it by modifying its Status.
	copy := resource.DeepCopy()
	copy.Status = statusMessage
	_, err := c.vpcclientset.MizarV1().Vpcs(corev1.NamespaceDefault).Update(copy)
	if err != nil {
		klog.Errorf(err.Error())
		return
	}

	c.recorder.Event(copy, corev1.EventTypeNormal, "ObjectHandled", "Object is handled by custom controller.")
}

func (c *Controller) objectDeletedCallback(object interface{}) {
	//klog.Infof("Deleted: %v", object)
	resource := object.(*vpcv1.Vpc)
	klog.Infof("Deleted: %v", resource)
	// If the object is in the desired state, end callback.
	if resource.Status == statusMessage {
		return
	}

	// If the object is not handled yet, handle it by modifying its Status.
	copy := resource.DeepCopy()
	copy.Status = statusMessage
	_, err := c.vpcclientset.MizarV1().Vpcs(corev1.NamespaceDefault).Update(copy)
	if err != nil {
		klog.Errorf(err.Error())
		return
	}
	gRPCRequest(DELETE, object)

	c.recorder.Event(copy, corev1.EventTypeNormal, "ObjectHandled", "Object is handled by custom controller.")
}

func gRPCRequest(command int, object interface{}) {
	// Set up a connection to the server.
	fmt.Println("command=%v", command)
	conn, err := grpc.Dial(config.Server_addr, grpc.WithInsecure(), grpc.WithBlock())
	if err != nil {
                fmt.Println("ccc")
		log.Fatalf("did not connect: %v", err)

	}
	defer conn.Close()
	clientcon := pb.NewVpcsServiceClient(conn)
	// Contact the server and request crd services.
	ctx, cancel := context.WithTimeout(context.Background(), time.Second)
	defer cancel()
	switch command {
	case 1:
		vpcin := object.(*vpcv1.Vpc)		
		vpcspec := vpcin.Spec
		var resource pb.Vpc
		resource = pb.Vpc{Ip: string(vpcspec.Ip), Prefix: string(vpcspec.Prefix), Vni: string(vpcspec.Vni), Dividers: string(vpcspec.Dividers), Status: string(vpcspec.Status), CreateTime: string(vpcspec.CreateTime), ProvisionDelay: string(vpcspec.ProvisionDelay)}
		//clientcon.CreateVpc(ctx, &pb.Vpc{Ip: string(vpcspec.Ip), Prefix: "10.0.0.0", Vni: "16777210", Dividers: "2", Status: "active", CreateTime: "2020-07-20", ProvisionDelay: "20"})
		_, err = clientcon.CreateVpc(ctx, &resource)
	case 2:
		vpcin := object.(*vpcv1.Vpc)
		vpcspec := vpcin.Spec
                fmt.Println("vpcspec test =%v", vpcspec)
		var resource pb.Vpc
		resource = pb.Vpc{Ip: string(vpcspec.Ip), Prefix: string("vpcspec.Prefix"), Vni: string("vpcspec.Vni"), Dividers: string("vpcspec.Dividers"), Status: string(vpcspec.Status), CreateTime: string("vpcspec.CreateTime"), ProvisionDelay: string("vpcspec.ProvisionDelay")}
		_, err = clientcon.UpdateVpc(ctx, &resource)
		//clientcon.UpdateVpc(ctx, &pb.Vpc{Ip: "10.0.0.1", Prefix: "10.0.0.0/24", Vni: "16777210", Dividers: "2", Status: "active", CreateTime: "2020-07-20", ProvisionDelay: "20"})
		//resource := pb.Vpc{vpcspec.Ip, vpcspec.Prefix, vpcspec.Vni, vpcspec.Dividers, vpcspec.Status, vpcspec.CreateTime, vpcspec.ProvisionDelay}
		//_, err = clientcon.UpdateVpc(ctx, resource)
	case 3:
		//vpc_in := object.(*vpcv1.Vpc)
		//vpcspec := vpc_in.Spec
		//id := vpcspec.Ip
		_, err := clientcon.DeleteVpc(ctx, &pb.VpcId{Id: "10.0.0.1"})
		if err != nil {
			log.Fatalf("%v", err)
			return
		}
		//_, err = clientcon.DeleteVpc(ctx, id)
	case 4:
		//vpc_in := object.(*vpcv1.Vpc)
		//vpcspec := vpc_in.Spec
		//id := vpcspec.Ip
		//var response *pb.VpcsResponse
		//response, err = clientcon.ReadVpc(ctx, id)
		r, err := clientcon.ReadVpc(ctx, &pb.VpcId{Id: "10.0.0.1"})
		log.Printf("Read VPC: %v", r)
                if err != nil {
			log.Fatalf("%v", err)
			return
		}
	case 5:
		//vpc_in := object.(*vpcv1.Vpc)
		//vpcspec := vpc_in.Spec
		//id := vpcspec.Ip
		//_, err = clientcon.ResumeVpc(ctx, id)
		_, err := clientcon.ResumeVpc(ctx, &pb.VpcId{Id: "10.0.0.1"})
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

