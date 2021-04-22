package config

var Server_addr string

func setServerAddress(addr string) error {
	Server_addr = addr
	return nil
}

