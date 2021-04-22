package main

import (
	"context"
	"fmt"
	"log"
	"net"

	pbbouncers "mizar.com/crds-operator-proxy/grpc_server/bouncers"
	pbdividers "mizar.com/crds-operator-proxy/grpc_server/dividers"
	pbdroplets "mizar.com/crds-operator-proxy/grpc_server/droplets"
	pbendpoints "mizar.com/crds-operator-proxy/grpc_server/endpoints"
	pbnets "mizar.com/crds-operator-proxy/grpc_server/nets"
	pbvpcs "mizar.com/crds-operator-proxy/grpc_server/vpcs"

	"google.golang.org/grpc"
)

const (
	port = ":50051"
)

// server is used to implement Vpcs_proto.VpcsServer.
type server struct {
	pbvpcs.UnimplementedVpcsServiceServer
	pbbouncers.UnimplementedBouncersServiceServer
	pbdividers.UnimplementedDividersServiceServer
	pbdroplets.UnimplementedDropletsServiceServer
	pbendpoints.UnimplementedEndpointsServiceServer
	pbnets.UnimplementedNetsServiceServer
}

//Vpcs
// services - CreateVpc
func (s *server) CreateVpc(ctx context.Context, in *pbvpcs.Vpc) (*pbvpcs.Empty, error) {
	log.Printf("Received Request - Create vpc: %v", in.GetIp())
	return &pbvpcs.Empty{}, nil
}

// services - UpdateVpc
func (s *server) UpdateVpc(ctx context.Context, in *pbvpcs.Vpc) (*pbvpcs.Empty, error) {
	log.Printf("Received Request - Update vpc: %v", in.GetIp())
	return &pbvpcs.Empty{}, nil
}

// services - ReadVpc
func (s *server) ReadVpc(ctx context.Context, in *pbvpcs.VpcId) (*pbvpcs.VpcsResponse, error) {
	log.Printf("Received Request - Read vpc: %v", in.GetId())
	return &pbvpcs.VpcsResponse{JsonReturncode: "{Vpcs Response: " + in.GetId() + "}"}, nil
}

// services - DeleteVpc
func (s *server) DeleteVpc(ctx context.Context, in *pbvpcs.VpcId) (*pbvpcs.Empty, error) {
	log.Printf("Received Request - Delete vpc: %v", in.GetId())
	return &pbvpcs.Empty{}, nil
}

// services - ResumeVpc
func (s *server) ResumeVpc(ctx context.Context, in *pbvpcs.VpcId) (*pbvpcs.Empty, error) {
	log.Printf("Received Request - Resume vpc: %v", in.GetId())
	return &pbvpcs.Empty{}, nil
}

//Bouncers
// services - CreateBouncer
func (s *server) CreateBouncer(ctx context.Context, in *pbbouncers.Bouncer) (*pbbouncers.Empty, error) {
	log.Printf("Received Request - Create bouncer: %v", in.GetIp())
	return &pbbouncers.Empty{}, nil
}

// services - UpdateBouncer
func (s *server) UpdateBouncer(ctx context.Context, in *pbbouncers.Bouncer) (*pbbouncers.Empty, error) {
	log.Printf("Received Request - Update bouncer: %v", in.GetIp())
	return &pbbouncers.Empty{}, nil
}

// services - ReadBouncer
func (s *server) ReadBouncer(ctx context.Context, in *pbbouncers.BouncerId) (*pbbouncers.BouncersResponse, error) {
	log.Printf("Received Request - Read bouncer: %v", in.GetId())
	return &pbbouncers.BouncersResponse{JsonReturncode: "{Bouncer Response: " + in.GetId() + "}"}, nil
}

// services - DeleteBouncer
func (s *server) DeleteBouncer(ctx context.Context, in *pbbouncers.BouncerId) (*pbbouncers.Empty, error) {
	log.Printf("Received Request - Delete bouncer: %v", in.GetId())
	return &pbbouncers.Empty{}, nil
}

// services - ResumeBouncer
func (s *server) ResumeBouncer(ctx context.Context, in *pbbouncers.BouncerId) (*pbbouncers.Empty, error) {
	log.Printf("Received Request - Resume bouncer: %v", in.GetId())
	return &pbbouncers.Empty{}, nil
}

//Dividers
// services - CreateDivider
func (s *server) CreateDivider(ctx context.Context, in *pbdividers.Divider) (*pbdividers.Empty, error) {
	log.Printf("Received Request - Create divider: %v", in.GetIp())
	return &pbdividers.Empty{}, nil
}

// services - UpdateDivider
func (s *server) UpdateDivider(ctx context.Context, in *pbdividers.Divider) (*pbdividers.Empty, error) {
	log.Printf("Received Request - Update divider: %v", in.GetIp())
	return &pbdividers.Empty{}, nil
}

// services - ReadDivider
func (s *server) ReadDivider(ctx context.Context, in *pbdividers.DividerId) (*pbdividers.DividersResponse, error) {
	log.Printf("Received Request - Read divider: %v", in.GetId())
	return &pbdividers.DividersResponse{JsonReturncode: "{Divider Response: " + in.GetId() + "}"}, nil
}

// services - DeleteDivider
func (s *server) DeleteDivider(ctx context.Context, in *pbdividers.DividerId) (*pbdividers.Empty, error) {
	log.Printf("Received Request - Delete divider: %v", in.GetId())
	return &pbdividers.Empty{}, nil
}

// services - ResumeDivider
func (s *server) ResumeDivider(ctx context.Context, in *pbdividers.DividerId) (*pbdividers.Empty, error) {
	log.Printf("Received Request - Resume divider: %v", in.GetId())
	return &pbdividers.Empty{}, nil
}

//Droplet
// services - CreateDroplet
func (s *server) CreateDroplet(ctx context.Context, in *pbdroplets.Droplet) (*pbdroplets.Empty, error) {
	log.Printf("Received Request - Create droplet: %v", in.GetIp())
	return &pbdroplets.Empty{}, nil
}

// services - UpdateDroplet
func (s *server) UpdateDroplet(ctx context.Context, in *pbdroplets.Droplet) (*pbdroplets.Empty, error) {
	log.Printf("Received Request - Update droplet: %v", in.GetIp())
	return &pbdroplets.Empty{}, nil
}

// services - ReadDroplet
func (s *server) ReadDroplet(ctx context.Context, in *pbdroplets.DropletId) (*pbdroplets.DropletsResponse, error) {
	log.Printf("Received Request - Read droplet: %v", in.GetId())
	return &pbdroplets.DropletsResponse{JsonReturncode: "{Droplet Response: " + in.GetId() + "}"}, nil
}

// services - DeleteDroplet
func (s *server) DeleteDroplet(ctx context.Context, in *pbdroplets.DropletId) (*pbdroplets.Empty, error) {
	log.Printf("Received Request - Delete droplet: %v", in.GetId())
	return &pbdroplets.Empty{}, nil
}

// services - ResumeDroplet
func (s *server) ResumeDroplet(ctx context.Context, in *pbdroplets.DropletId) (*pbdroplets.Empty, error) {
	log.Printf("Received Request - Resume droplet: %v", in.GetId())
	return &pbdroplets.Empty{}, nil
}

//Endpoint
// services - CreateEndpoint
func (s *server) CreateEndpoint(ctx context.Context, in *pbendpoints.Endpoint) (*pbendpoints.Empty, error) {
	log.Printf("Received Request - Create ep: %v", in.GetIp())
	return &pbendpoints.Empty{}, nil
}

// services - UpdateEndpoint
func (s *server) UpdateEndpoint(ctx context.Context, in *pbendpoints.Endpoint) (*pbendpoints.Empty, error) {
	log.Printf("Received Request - Update ep: %v", in.GetIp())
	return &pbendpoints.Empty{}, nil
}

// services - ReadEndpoint
func (s *server) ReadEndpoint(ctx context.Context, in *pbendpoints.EndpointId) (*pbendpoints.EndpointsResponse, error) {
	log.Printf("Received Request - Read ep: %v", in.GetId())
	return &pbendpoints.EndpointsResponse{JsonReturncode: "{Endpoint Response: " + in.GetId() + "}"}, nil
}

// services - DeleteEndpoint
func (s *server) DeleteEndpoint(ctx context.Context, in *pbendpoints.EndpointId) (*pbendpoints.Empty, error) {
	log.Printf("Received Request - Delete ep: %v", in.GetId())
	return &pbendpoints.Empty{}, nil
}

// services - ResumeEndpoint
func (s *server) ResumeEndpoint(ctx context.Context, in *pbendpoints.EndpointId) (*pbendpoints.Empty, error) {
	log.Printf("Received Request - Resume ep: %v", in.GetId())
	return &pbendpoints.Empty{}, nil
}

//Net
// services - CreateNet
func (s *server) CreateNet(ctx context.Context, in *pbnets.Net) (*pbnets.Empty, error) {
	log.Printf("Received Request - Create net: %v", in.GetIp())
	return &pbnets.Empty{}, nil
}

// services - UpdateNet
func (s *server) UpdateNet(ctx context.Context, in *pbnets.Net) (*pbnets.Empty, error) {
	log.Printf("Received Request - Update net: %v", in.GetIp())
	return &pbnets.Empty{}, nil
}

// services - ReadNet
func (s *server) ReadNet(ctx context.Context, in *pbnets.NetId) (*pbnets.NetsResponse, error) {
	log.Printf("Received Request - Read net: %v", in.GetId())
	return &pbnets.NetsResponse{JsonReturncode: "{Net Response: " + in.GetId() + "}"}, nil
}

// services - DeleteNet
func (s *server) DeleteNet(ctx context.Context, in *pbnets.NetId) (*pbnets.Empty, error) {
	log.Printf("Received Request - Delete net: %v", in.GetId())
	return &pbnets.Empty{}, nil
}

// services - ResumeNet
func (s *server) ResumeNet(ctx context.Context, in *pbnets.NetId) (*pbnets.Empty, error) {
	log.Printf("Received Request - Resume net: %v", in.GetId())
	return &pbnets.Empty{}, nil
}

func main() {
	fmt.Print("Server started, Port: " + port)
	lis, err := net.Listen("tcp", port)
	if err != nil {
		log.Fatalf("failed to listen: %v", err)
	}
	s := grpc.NewServer()
	pbvpcs.RegisterVpcsServiceServer(s, &server{})
	pbbouncers.RegisterBouncersServiceServer(s, &server{})
	pbdividers.RegisterDividersServiceServer(s, &server{})
	pbdroplets.RegisterDropletsServiceServer(s, &server{})
	pbendpoints.RegisterEndpointsServiceServer(s, &server{})
	pbnets.RegisterNetsServiceServer(s, &server{})
	if err := s.Serve(lis); err != nil {
		log.Fatalf("failed to serve: %v", err)
	}
}

