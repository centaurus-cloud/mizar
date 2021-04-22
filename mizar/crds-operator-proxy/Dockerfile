FROM golang:1.14 as clone
WORKDIR /
RUN git clone --branch v0.17.0 https://github.com/kubernetes/code-generator 

# Generate crd api code 
FROM golang:1.14 as generatecode
ENV GO111MODULE=on
COPY --from=clone /code-generator /code-generator
COPY ./ /crds-operator-proxy

WORKDIR /code-generator
RUN go mod edit -replace=mizar.com/crds-operator-proxy=/crds-operator-proxy
RUN ./generate-groups.sh all mizar.com/crds-operator-proxy/crds/vpcs/apis/generated mizar.com/crds-operator-proxy/crds/vpcs apis:v1 --go-header-file ./hack/boilerplate.go.txt --output-base ..
RUN cp -r /mizar.com/crds-operator-proxy/crds/vpcs/apis/* /crds-operator-proxy/crds/vpcs/apis/ 

RUN ./generate-groups.sh all mizar.com/crds-operator-proxy/crds/bouncers/apis/generated mizar.com/crds-operator-proxy/crds/bouncers apis:v1 --go-header-file ./hack/boilerplate.go.txt --output-base ..
RUN cp -r /mizar.com/crds-operator-proxy/crds/bouncers/apis/* /crds-operator-proxy/crds/bouncers/apis/

RUN ./generate-groups.sh all mizar.com/crds-operator-proxy/crds/dividers/apis/generated mizar.com/crds-operator-proxy/crds/dividers apis:v1 --go-header-file ./hack/boilerplate.go.txt --output-base ..
RUN cp -r /mizar.com/crds-operator-proxy/crds/dividers/apis/* /crds-operator-proxy/crds/dividers/apis/

RUN ./generate-groups.sh all mizar.com/crds-operator-proxy/crds/droplets/apis/generated mizar.com/crds-operator-proxy/crds/droplets apis:v1 --go-header-file ./hack/boilerplate.go.txt --output-base ..
RUN cp -r /mizar.com/crds-operator-proxy/crds/droplets/apis/* /crds-operator-proxy/crds/droplets/apis/

RUN ./generate-groups.sh all mizar.com/crds-operator-proxy/crds/endpoints/apis/generated mizar.com/crds-operator-proxy/crds/endpoints apis:v1 --go-header-file ./hack/boilerplate.go.txt --output-base ..
RUN cp -r /mizar.com/crds-operator-proxy/crds/endpoints/apis/* /crds-operator-proxy/crds/endpoints/apis/

RUN ./generate-groups.sh all mizar.com/crds-operator-proxy/crds/nets/apis/generated mizar.com/crds-operator-proxy/crds/nets apis:v1 --go-header-file ./hack/boilerplate.go.txt --output-base ..
RUN cp -r /mizar.com/crds-operator-proxy/crds/nets/apis/* /crds-operator-proxy/crds/nets/apis/
RUN rm -r /mizar.com

# Run custom resource controller. Requires KUBECONFIG env and a running Kubernetes cluster.
FROM golang:1.14
ENV GO111MODULE=on
ENV KUBECONFIG=~/.kube/config 
COPY --from=generatecode /crds-operator-proxy /crds-operator-proxy
WORKDIR /crds-operator-proxy/cmd
ENTRYPOINT [ "go", "run", "." ]


