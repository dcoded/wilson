#pragma once
#include <future>
#include <thread>
#include <random>

#include <mote.h>


void add_noise (uint8_t* data, size_t len, double probability);

struct transmission {
    int next;
    int prev;
};

struct udp_lite_header : public transmission {
    uint16_t source;
    uint16_t dest;
    uint16_t size;
    uint16_t checksum;
};

struct message : public udp_lite_header {
    std::string data;
};


class udp_mote : public mote <message>, public std::thread {
private:
public:
    std::atomic <int> bytes_sent;
    std::atomic <int> bytes_recv;
    
    std::atomic <int> msgs_sent;
    std::atomic <int> msgs_recv;

    // send/recv an application message to/from another mote in the network
    virtual void send (message msg, const int destination);
    
    std::future<void> test (message msg, const int destination) {
        return std::async (std::launch::async, &udp_mote::send, this, msg, destination);
    }

private:
    virtual void recv (const message msg, const std::string event_name);
};


/**
 * Application Message Handlers
 */
void udp_mote::recv (message msg, const std::string event_name) {
    // last arg is a percentage from 0.0 to 1.0
    add_noise (reinterpret_cast <uint8_t*> (&msg), sizeof (transmission), 0.0);

    if (msg.dest == uuid () || msg.next == uuid ()) {
        bytes_recv += sizeof (udp_lite_header) - sizeof (transmission);
        msgs_recv ++;
    }

    if (msg.dest == uuid ()) {

    }
    else if (msg.next == uuid ()) {

        send (msg, msg.dest);
    }
}

void udp_mote::send (message msg, const int destination) {
    msg.source = uuid ();
    msg.dest   = destination;
    msg.next   = next_hop (msg.dest);
    msg.prev   = uuid ();

    if (msg.next == -1) { // cant calculate path

    } else {
        bytes_sent += sizeof (udp_lite_header) - sizeof (transmission);
        msgs_sent ++;
        publish (msg);
    }
}



void add_noise (uint8_t* data, size_t len, double probability) { 
    std::random_device rd;
    std::default_random_engine e1 (rd ());   
    std::uniform_int_distribution <int> dist (0, 0x7FFFFFFF);

    for (int i = 0; i < len; i++)
    for (int j = 0; j < 8; j++)
    {
        int random_number = dist (e1);
        if (random_number < probability * 0xFFFFFFFF)
            data[i] ^= (1 << j);
    }
}
