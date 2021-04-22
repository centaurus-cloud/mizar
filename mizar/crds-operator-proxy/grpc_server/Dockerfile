#gRPC 
FROM golang:1.14 as server
ENV GO111MODULE=on
WORKDIR /
COPY . /crds-operator-proxy/grpc_server
RUN pwd
RUN ls -al /crds-operator-proxy
EXPOSE 50051
WORKDIR /crds-operator-proxy/grpc_server
RUN ls -al
CMD go run ./server/main.go
#ENTRYPOINT [ "go", "run", "." ]
