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

	dividerclientset "mizar.com/crds-operator-proxy/crds/dividers/apis/generated/clientset/versioned"
	dividerscheme "mizar.com/crds-operator-proxy/crds/dividers/apis/generated/clientset/versioned/scheme"
	dividerinformers "mizar.com/crds-operator-proxy/crds/dividers/apis/generated/informers/externalversions"
	dividerlisters "mizar.com/crds-operator-proxy/crds/dividers/apis/generated/listers/apis/v1"
	dividerv1 "mizar.com/crds-operator-proxy/crds/dividers/apis/v1"
)

type Controller struct {
	kubeclientset          kubernetes.Interface
	apiextensionsclientset apiextensionsclientset.Interface
	dividerclientset       dividerclientset.Interface
	informer               cache.SharedIndexInformer
	lister                 dividerlisters.DividerLister
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
	testClient := dividerclientset.NewForConfigOrDie(config)

	informerFactory := dividerinformers.NewSharedInformerFactory(testClient, time.Minute*1)
	informer := informerFactory.Mizar().V1().Dividers()

	utilruntime.Must(dividerv1.AddToScheme(dividerscheme.Scheme))
	eventBroadcaster := record.NewBroadcaster()
	eventBroadcaster.StartLogging(klog.Infof)
	eventBroadcaster.StartRecordingToSink(&typedcorev1.EventSinkImpl{Interface: kubeClient.CoreV1().Events("")})
	recorder := eventBroadcaster.NewRecorder(dividerscheme.Scheme, corev1.EventSource{Component: "divider-controller"})

	workqueue := workqueue.NewRateLimitingQueue(workqueue.DefaultControllerRateLimiter())

	c := &Controller{
		kubeclientset:          kubeClient,
		apiextensionsclientset: apiextensionsClient,
		dividerclientset:       testClient,
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

