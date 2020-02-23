import ipaddress
import heapq
import subprocess

class Cidr:
	def __init__(self, prefixlen, ip):
		"""
		Defines an IPv4 CIDR block
		"""
		self.prefixlen = prefixlen
		self.ip = ip
		self.ipnet = ipaddress.ip_network("{}/{}".format(self.ip, self.prefixlen))
		self.hosts = list(self.ipnet.hosts())
		self.gw = self.hosts.pop(0)
		self.allocated = set()
		heapq.heapify(self.hosts)

	def get_ip(self, idx):
		return self.ipnet[idx]

	def get_hosts(self):
		return self.hosts

	def allocate_ip(self):
		if not len(self.hosts):
			return None
		ip = heapq.heappop(self.hosts)
		# TODO: bad hack, search the list and remove it!!
		while ip in self.allocated:
			ip = heapq.heappop(self.hosts)
		self.allocated.add(ip)
		return ip

	def mark_ip_as_allocated(self, ip):
		self.allocated.add(ipaddress.IPv4Address(ip))

	def deallocate_ip(self, ip):
		self.allocated.remove(ip)
		heapq.heappush(self.hosts,ip)


def run_cmd(cmd):
	result = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
	text = result.stdout.read().decode()
	returncode = result.returncode
	return (returncode, text)