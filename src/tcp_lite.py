import os
import struct
import socket

SOURCE_ADDRESS = '127.1.1.5'
DEST_ADDRESS = '172.53.55.81'

class TCP_lite(object):
    def __init__(self, source_port, dest_port):
        self.source_port = source_port
        self.dest_port = dest_port
        self.window_size = 0
        self.checksum = 1
        self.payload = ""

    def pack(self,source,dest):
        data_offset = 5 << 4
        self.window_size = len(self.payload)
        lite_header = struct.pack('!HHHH', self.source_port, self.dest_port,
                                  self.window_size, 
                                  self.checksum)
        return lite_header + self.payload


def main():
    test = TCP_lite(8000,8000)
    test.payload = 'hithere'
    data = test.pack('0.0.0.0','127.0.0.1')
    print data
    HOST = socket.gethostbyname(socket.gethostname())
    # CREATE RAW SOCKET, BIND TO PUBLIC INTERFACE
    s = socket.socket(socket.AF_INET, socket.SOCK_RAW, socket.IPPROTO_IP)
    s.bind((HOST,0))
    # INCLUDE IP HEADERS
    s.setsockopt(socket.IPPROTO_IP, socket.IP_HDRINCL, 1)
    
    # RECEIVE ALL PACKAGES
    s.ioctl(socket.SIO_RCVALL, socket.RCVALL_ON)
    s.recvfrom(8000)

if __name__ == '__main__':
    pass
    #main()

class TCP(object):
    def __init__(self, source_port, dest_port):
        self.source_port = source_port
        self.dest_port = dest_port
        self.sequence_num = 0
        self.ack_num = 0
        self.offset = 5
        self.reserved = 0
        self.org = 0
        self.ack = 0
        self.psh = 1
        self.rst = 0
        self.syn = 0
        self.fin = 0
        self.urg = 0
        self.window = socket.htons(5840)
        self.checksum = 0
        self.urgp = 0
        self.payload = ""

    def pack(self, source, dest):
        data_offset = self.offset << 4
        flags = (self.fin) + (self.syn << 1) + (self.rst << 2) + \
                (self.psh << 3) + (self.ack << 4) + (self.urg << 5)
        tcp_header = struct.pack('!HHLLBBHHH', self.source_port,
                                 self.dest_port, self.sequence_num,
                                 self.ack_num, data_offset, flags,
                                 self.window, self.checksum, self.urgp)
        
        reserved = 0
        protocol = socket.IPPROTO_TCP
        total_len = len(tcp_header) + len(self.payload)

        check_header = struct.pack('!4s4sBBH', source, dest, reserved,
                                protocol, total_len)
        check_header = check_header + tcp_header + self.payload
        tcp_checksum = 0
        tcp_header = struct.pack('!HHLLBBH', self.source_port,
                                 self.dest_port, self.sequence_num,
                                 self.ack_num, data_offset, flags, self.window)
        tcp_header += struct.pack('H',tcp_checksum) + struct.pack('!H', self.urgp)
        return tcp_header

def parse_lite(packet):
    header_length = 0
    lite_header = packet[header_length:header_length + 8]

    lite_header = struct.unpack('!HHHH', lite_header)
    source_port = lite_header[0]
    dest_port = lite_header[1]
    window_size = lite_header[2]
    checksum = lite_header[3]
    
    data = packet[header_length + 8:]
    return source_port, dest_port, data

def parse_TCP(packet):
    header_length = 0
    tcp_header = packet[header_length:header_length+20]

    tcp_header = struct.unpack('!HHLLBBHHH' , tcp_header)

    source_port = tcp_header[0]
    dest_port = tcp_header[1]
    sequence = tcp_header[2]
    acknowl = tcp_header[3]
    offset_reserved = tcp_header[4]
    tcp_header_length = offset_reserved >> 4

    data = packet[tcp_header_length:]
    return source_port, dest_port, data
    
    
