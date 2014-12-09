#pragma once
#include <mote.h>

struct message : public transmission {
    std::string data;
};


class tcp_mote : public mote <message> {
private:
public:
    using mote <message>::mote;
    // send/recv an application message to/from another mote in the network
    void send (message msg, const int destination);

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

    if (msg.dest == id () || msg.next == id ()) {
        std::cout
            << "[mote[" << id () << "]::recv(" << event_name << ")] "
            << "recieved data: " << msg.data << "\n";

        /* (sizeof (message) + msg.data.size ()) * 8 */
        metric ("recv_bits")     += msg.data.size () * 8;
        metric ("recv_messages") ++;

        // send (ack, ack.dest);
    }
}

void tcp_mote::send (message msg, const int destination) {
    msg.source = id ();
    msg.dest   = destination;
    msg.next   = next_hop (msg.dest);
    msg.prev   = id ();

    if (msg.next == -1) {
        // we cannot find a neighbor who knows of the destination
        std::cout
            << "[mote[" << id () << "]::send] "
            << " cannot compute hop to destination " << msg.dest << "\n";
    } else {
        // update the message to notify correct neighbor (next)
        std::cout
            << "[mote[" << id () << "]::send] "
            << "(" << msg.source << "," << msg.next << "," << msg.dest << ")"
            << "\n";

        // pass it on
        metric ("sent_bits")     += msg.data.size () * 8;
        metric ("sent_messages") ++;
        publish (msg);
    }
}

