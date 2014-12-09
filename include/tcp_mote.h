#pragma once
#include <future>
#include <thread>
#include <random>

#include <mote.h>


void add_noise (uint8_t* data, size_t len, double probability);

struct message : public transmission {
    std::string data;
};


class tcp_mote : public mote <message>, public std::thread {
private:
public:
    std::atomic <int> bits_sent;
    std::atomic <int> bits_recv;
    
    std::atomic <int> msgs_sent;
    std::atomic <int> msgs_recv;

    using mote <message>::mote;
    // send/recv an application message to/from another mote in the network
    void send (message msg, const int destination);
    std::future<void> test (message msg, const int destination) {
        return std::async (std::launch::async, &tcp_mote::send, this, msg, destination);
    }

    std::future<void> init () {
        // create routing tables
        return std::async (std::launch::async, &tcp_mote::invocate, this);
    }

private:
    void recv (const message msg, const std::string event_name);
};


/**
 * Application Message Handlers
 */
void tcp_mote::recv (message msg, const std::string event_name) {
    // last arg is a percentage from 0.0 to 1.0
    add_noise (reinterpret_cast <uint8_t*> (&msg), sizeof (transmission), 0.0);

    // create (but dont send yet) acknowledgement/confirmation
    // confirm ack;
    // {
    //     ack.source = id ();
    //     ack.dest   = msg.prev;
    //     ack.next   = next_hop (msg.source);
    //     ack.prev   = id ();
    // }

    if (msg.dest == uuid ()) {
        // std::cout
        //     << "[mote[" << id () << "]::recv(" << event_name << ")] "
        //     << "recieved data: " << msg.data << "\n";

        bits_recv += msg.data.size () * 8;
        msgs_recv ++;

        // send (ack, ack.dest);
    }
    else if (msg.next == uuid ()) {
        // std::cout 
        //     << "[mote[" << id () << "]::recv(" << event_name << ")] "
        //     << "forwarding to dest " << msg.dest << "\n";

        bits_recv += msg.data.size () * 8;
        msgs_recv ++;

        send (msg, msg.dest);
        // send (ack, ack.dest);
    }
}

void tcp_mote::send (message msg, const int destination) {
    msg.source = uuid ();
    msg.dest   = destination;
    msg.next   = next_hop (msg.dest);
    msg.prev   = uuid ();

    if (msg.next == -1) {
        // we cannot find a neighbor who knows of the destination
        // std::cout
        //     << "[mote[" << id () << "]::send] "
        //     << " cannot compute hop to destination " << msg.dest << "\n";
    } else {
        // update the message to notify correct neighbor (next)
        // std::cout
        //     << "[mote[" << id () << "]::send] "
        //     << "(" << msg.source << "," << msg.next << "," << msg.dest << ")"
        //     << "\n";

        // pass it on
        bits_sent += msg.data.size () * 8;
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
