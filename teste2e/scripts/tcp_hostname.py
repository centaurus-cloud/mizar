import socket

HOST = '0.0.0.0'
PORT = 9001

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
s.bind((HOST, PORT))
s.listen(1)
host = bytes(socket.gethostname(), 'utf-8')

while True:
    conn, addr = s.accept()

    while True:
        data = conn.recv(1024)
        if not data:
            break
        conn.sendall(host)

    conn.close()
