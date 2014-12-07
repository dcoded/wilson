import os
import struct

SOURCE_ADDRESS = '127.1.1.5'
DEST_ADDRESS = '172.53.55.81'

class TCP_lite(object):
    def __init__(self, source_port, dest_port):
        self.source_port = source_port
        self.dest_port = dest_port
        self.window_size = socket.htons(5840)
        self.checksum = 0
        self.payload = ""

    def pack(self,source,dest):
        data_offset = 5 << 4
        lite_header = struct.pack('!HHHH', self.source_port, self.dest_port,
                                  self.window_size, self.checksum)
        return lite_header + self.payload

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
                                protocol, total_length)
        check_header = check_header + tcp_header + self.payload
        tcp_checksum = checksum(check_header)
        tcp_header = struct.pack('!HHLLBBH', self.source_port,
                                 self.dest_port, self.sequence_num,
                                 self.ack_num, data_offset, flags, self.window)
        tcp_header += struct.pack('H',tcp_checksum) + struct.pack('!H', self.urgp)
        return tcp_header



def convert_lite_to_TCP():
    data = s.recvfrom(5999)
    source_port, dest_port, payload = parse_lite(data)

    tcp = TCP(source_port, dest_port)
    tcp.payload = payload
    data = tcp.pack(SOURCE_ADDRESS, DEST_ADDRESS)



def convert_TCP_to_lite(data):
    data = s.recvfrom(5999)
    source_port, dest_port, data = parse_TCP(data)

    lite = TCP_lite(source_port, dest_port)
    lite.payload = data
    data = lite.pack(SOURCE_ADDRESS, DEST_ADDRESS)


def parse_IP_header(packet):
    ip_header = packet[:20]
    ip_header = struct.unpack('!BBHHHBBH4s4s',ip_header)
    header_length = ((ip_header[0] >> 4) & 0xF) * 4
    return header_length

def parse_lite(packet):
    header_length = parse_IP_header(packet)
    lite_header = packet[header_length:header_length + 8]

    lite_header = struct.unpack('!HHHH', lite_header)
    source_port = lite_header[0]
    dest_port = lite_header[1]
    window_size = lite_header[2]
    checksum = lite_header[3]

    data = packet[header_length + 8:]
    return source_port, dest_port, data

def parse_TCP(packet):
    header_length = parse_IP_header(packet)
    tcp_header = packet[header_length:header_length+20]

    tcp_header = struct.unpack('!HHLLBBHHH' , tcp_header)

    source_port = tcp_header[0]
    dest_port = tcp_header[1]
    sequence = tcp_header[2]
    acknowl = tcp_header[3]
    offset_reserved = tcp_header[4]
    tcp_header_length = offset_reserved >> 4

    data = data[tcp_header_length:]
    return source_port, dest_port, data
