import os
from tcp_lite import *

from twisted.internet import reactor
from twisted.internet import protocol
from twisted.internet.tcp import Client
from twisted.protocols import basic

DEST_IP = '127.0.0.1'

class TCP(basic.LineReceiver):
    def dataReceived(self,data):
        print data
        source_port, dest_port, data = parse_TCP(data)
        tcp = TCP_lite(source_port, dest_port)
        tcp.payload = data
        data = tcp.pack('127.0.0.1','127.0.0.1')
        
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect((DEST_IP, 8000))
        sock.send(data)
        print data
        print "DONE"

def main():
    factory = protocol.ServerFactory()
    factory.protocol = TCP
    reactor.listenTCP(8001,factory)
    reactor.run()

if __name__ == '__main__':
    main()
