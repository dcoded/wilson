#pragma once
#include <future>
#include <thread>
#include <random>
#include <mote.h>

#include <header/tcp.h>

enum tcp_flag {
    TCP_FIN = 0x01,
    TCP_SYN = 0x02,
    TCP_RST = 0x04,
    TCP_ACK = 0x10,
    TCP_NOFLAGS = 0xFE00
};

enum tcp_state {
    TCP_STATE_CLOSED,
    TCP_STATE_SYN_SENT,
    TCP_STATE_SYN_RECV,
    TCP_STATE_SERVER_ESTABLISHED,
    TCP_STATE_CLIENT_ESTABLISHED,
    TCP_STATE_FIN_WAIT_1,
    TCP_STATE_FIN_WAIT_2,
    TCP_STATE_LAST_ACK

};

struct tcp_message : public tcp_header {
    std::string data;
};


class tcp_mote : public mote <tcp_message>, public std::thread {
private:
    std::map <int, tcp_state> state_;
public:
    std::atomic <int> bytes_sent;
    std::atomic <int> bytes_recv;
    
    std::atomic <int> msgs_sent;
    std::atomic <int> msgs_recv;

    // send/recv an application message to/from another mote in the network
    virtual bool send (tcp_message msg, const int destination);

    static void create_interference (tcp_message& msg, double probability);
    static bool not_interfered (tcp_message& msg, double probability);

    std::future <bool> connect (const int destination);
    std::future <bool> close (const int destination);

    bool connected (const int destination);


private:
    virtual void recv (const tcp_message msg, const std::string event_name);

    void respond (tcp_message& msg);
};