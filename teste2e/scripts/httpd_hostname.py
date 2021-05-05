#!/usr/bin/env python3
import socket
from http.server import HTTPServer, BaseHTTPRequestHandler


class SimpleHTTPRequestHandler(BaseHTTPRequestHandler):

    def do_GET(self):
        self.send_response(200)
        self.end_headers()
        host = bytes(socket.gethostname(), 'utf-8')
        self.wfile.write(host)


httpd = HTTPServer(('0.0.0.0', 7000), SimpleHTTPRequestHandler)
httpd.serve_forever()
