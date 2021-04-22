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

	bouncerclientset "mizar.com/crds-operator-proxy/crds/bouncers/apis/generated/clientset/versioned"
	bouncerlisters "mizar.com/crds-operator-proxy/crds/bouncers/apis/generated/listers/apis/v1"
	bouncerscheme "mizar.com/crds-operator-proxy/crds/bouncers/apis/generated/clientset/versioned/scheme"
	bouncerinformers "mizar.com/crds-operator-proxy/crds/bouncers/apis/generated/informers/externalversions"
	bouncerv1 "mizar.com/crds-operator-proxy/crds/bouncers/apis/v1"
)

type Controller struct {
	kubeclientset          kubernetes.Interface
	apiextensionsclientset apiextensionsclientset.Interface
	bouncerclientset       bouncerclientset.Interface
	informer               cache.SharedIndexInformer
	lister                 bouncerlisters.BouncerLister
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
	testClient := bouncerclientset.NewForConfigOrDie(config)

	informerFactory := bouncerinformers.NewSharedInformerFactory(testClient, time.Minute*1)
	informer := informerFactory.Mizar().V1().Bouncers()

	utilruntime.Must(bouncerv1.AddToScheme(bouncerscheme.Scheme))
	eventBroadcaster := record.NewBroadcaster()
	eventBroadcaster.StartLogging(klog.Infof)
	eventBroadcaster.StartRecordingToSink(&typedcorev1.EventSinkImpl{Interface: kubeClient.CoreV1().Events("")})
	recorder := eventBroadcaster.NewRecorder(bouncerscheme.Scheme, corev1.EventSource{Component: "bouncer-controller"})

	workqueue := workqueue.NewRateLimitingQueue(workqueue.DefaultControllerRateLimiter())

	c := &Controller{
		kubeclientset:          kubeClient,
		apiextensionsclientset: apiextensionsClient,
		bouncerclientset:       testClient,
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
