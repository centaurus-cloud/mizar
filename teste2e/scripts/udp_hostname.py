import socket

HOST = '0.0.0.0'
PORT = 5001
s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.bind((HOST, PORT))

host = bytes(socket.gethostname(), 'utf-8')

while 1:
    data, address = s.recvfrom(1024)
    if not data:
        conn.sendall(b'broken')
    s.sendto(host, address)
