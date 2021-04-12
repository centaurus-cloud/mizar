cd $HOME
wget -qO- https://github.com/futurewei-cloud/containerd/releases/download/tenant-cni-args/containerd.zip | zcat > /tmp/containerd
sudo chmod +x /tmp/containerd
sudo systemctl stop containerd
sudo mv /usr/bin/containerd /usr/bin/containerd.bak
sudo mv /tmp/containerd /usr/bin/
sudo systemctl restart containerd
sudo systemctl start docker
export CONTAINER_RUNTIME_ENDPOINT="containerRuntime,container,/run/containerd/containerd.sock"
sleep 15
cd $HOME/go/src/k8s.io/arktos
./hack/arktos-up.sh
