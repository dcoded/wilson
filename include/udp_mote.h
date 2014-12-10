#pragma once
#include <future>
#include <thread>
#include <random>

#include <mote.h>
#include <header/udp.h>

struct udp_message : public udp_header {
    std::string data;
};


class udp_mote : public mote <udp_message>, public std::thread {
private:
public:
    std::atomic <int> bytes_sent;
    std::atomic <int> bytes_recv;
    
    std::atomic <int> msgs_sent;
    std::atomic <int> msgs_recv;

    // send/recv an application message to/from another mote in the network
    virtual void send (udp_message msg, const int destination);

    static void create_interference (udp_message& msg, double probability);
    static bool not_interfered (udp_message& msg, double probability);

    std::future<void> test (udp_message msg, const int destination);

    std::future <void> connect (int destination);
    std::future <void> close (int destination);

    bool connected (const int destination);

private:
    virtual void recv (const udp_message msg, const std::string event_name);
};