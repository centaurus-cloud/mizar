package controller

import (
	"os"
	"time"

	corev1 "k8s.io/api/core/v1"
	apiextensionsclientset "k8s.io/apiextensions-apiserver/pkg/client/clientset/clientset"
	utilruntime "k8s.io/apimachinery/pkg/util/runtime"
	"k8s.io/apimachinery/pkg/util/wait"
	"k8s.io/client-go/kubernetes"
	typedcorev1 "k8s.io/client-go/kubernetes/typed/core/v1"
	"k8s.io/client-go/tools/cache"
	"k8s.io/client-go/tools/clientcmd"
	"k8s.io/client-go/tools/record"
	"k8s.io/client-go/util/workqueue"
	"k8s.io/klog"

	dropletv1 "mizar.com/crds-operator-proxy/crds/droplets/apis/v1"
	dropletclientset "mizar.com/crds-operator-proxy/crds/droplets/apis/generated/clientset/versioned"
	dropletscheme "mizar.com/crds-operator-proxy/crds/droplets/apis/generated/clientset/versioned/scheme"
	dropletinformers "mizar.com/crds-operator-proxy/crds/droplets/apis/generated/informers/externalversions"
	dropletlisters "mizar.com/crds-operator-proxy/crds/droplets/apis/generated/listers/apis/v1"
)

type Controller struct {
	kubeclientset          kubernetes.Interface
	apiextensionsclientset apiextensionsclientset.Interface
	dropletclientset       dropletclientset.Interface
	informer               cache.SharedIndexInformer
	lister                 dropletlisters.DropletLister
	recorder               record.EventRecorder
	workqueue              workqueue.RateLimitingInterface
}

func NewController() *Controller {
	kubeconfig := os.Getenv("KUBECONFIG")

	config, err := clientcmd.BuildConfigFromFlags("", kubeconfig)
	if err != nil {
		klog.Fatalf(err.Error())
	}

	kubeClient := kubernetes.NewForConfigOrDie(config)
	apiextensionsClient := apiextensionsclientset.NewForConfigOrDie(config)
	testClient := dropletclientset.NewForConfigOrDie(config)

	informerFactory := dropletinformers.NewSharedInformerFactory(testClient, time.Minute*1)
	informer := informerFactory.Mizar().V1().Droplets()

	utilruntime.Must(dropletv1.AddToScheme(dropletscheme.Scheme))
	eventBroadcaster := record.NewBroadcaster()
	eventBroadcaster.StartLogging(klog.Infof)
	eventBroadcaster.StartRecordingToSink(&typedcorev1.EventSinkImpl{Interface: kubeClient.CoreV1().Events("")})
	recorder := eventBroadcaster.NewRecorder(dropletscheme.Scheme, corev1.EventSource{Component: "droplet-controller"})

	workqueue := workqueue.NewRateLimitingQueue(workqueue.DefaultControllerRateLimiter())

	c := &Controller{
		kubeclientset:          kubeClient,
		apiextensionsclientset: apiextensionsClient,
		dropletclientset:       testClient,
		informer:               informer.Informer(),
		lister:                 informer.Lister(),
		recorder:               recorder,
		workqueue:              workqueue,
	}

	informer.Informer().AddEventHandler(cache.ResourceEventHandlerFuncs{
		AddFunc:    c.objectAddedCallback,
		UpdateFunc: c.objectUpdatedCallback,
		DeleteFunc: c.objectDeletedCallback,
	})
	informerFactory.Start(wait.NeverStop)

	return c
}

