#pragma once
#include <future>
#include <thread>
#include <random>

#include <mote.h>

struct transmission {
    int next;
    int prev;
};

struct tcp_lite_header : public transmission {
    uint16_t source;
    uint16_t dest;
    uint16_t size;
    uint16_t checksum;
};

struct message : public tcp_lite_header {
    std::string data;
};


class tcp_mote : public mote <message>, public std::thread {
private:
public:
    std::atomic <int> bytes_sent;
    std::atomic <int> bytes_recv;
    
    std::atomic <int> msgs_sent;
    std::atomic <int> msgs_recv;

    // send/recv an application message to/from another mote in the network
    virtual void send (message msg, const int destination);

    static void create_interference (message& msg, double probability);
    static bool not_interfered (message& msg, double probability);

    std::future<void> test (message msg, const int destination);

private:
    virtual void recv (const message msg, const std::string event_name);
};