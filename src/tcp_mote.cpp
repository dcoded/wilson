#include <tcp_mote.h>




/**
 * Application Message Handlers
 */
void tcp_mote::recv (message msg, const std::string event_name) {
    // last arg is a percentage from 0.0 to 1.0
    //create_interference (msg, 0.0);
    if (not_interfered (msg, 0/*0.0001*/))
    {
        if (msg.dest == uuid () || msg.next == uuid ()) {
            bytes_recv += sizeof (tcp_lite_header) - sizeof (transmission);
            msgs_recv ++;
        }

        if (msg.dest == uuid ()) {

        }
        else if (msg.next == uuid ()) {

            send (msg, msg.dest);
        }
    }
}




void tcp_mote::send (message msg, const int destination) {
    msg.source = uuid ();
    msg.dest   = destination;
    msg.next   = next_hop (msg.dest);
    msg.prev   = uuid ();

    if (msg.next == -1) { // cant calculate path

    } else {
        bytes_sent += sizeof (tcp_lite_header) - sizeof (transmission);
        msgs_sent ++;
        publish (msg);
    }
}


void tcp_mote::create_interference (message& msg, double probability) {
    uint8_t* data = reinterpret_cast <uint8_t*> (&msg);
    size_t   len  = sizeof (message);

    std::random_device rd;
    std::default_random_engine e1 (rd ());   
    std::uniform_int_distribution <uint32_t> dist (0, 0xFFFFFFFF);

    for (int i = 0; i < len; i++)
    for (int j = 0; j < 8; j++)
    {
        uint32_t random_number = dist (e1);
        if (random_number < probability * 0xFFFFFFFF)
            data[i] ^= (1 << j);
    }
}

bool tcp_mote::not_interfered (message& msg, double probability) {
    std::random_device rd;
    std::default_random_engine e1 (rd ());   
    std::uniform_int_distribution <uint32_t> dist (0, 0xFFFFFFFF);

    for (int i = 0; i < sizeof (msg) * 8; i++)
    {
        uint32_t random_number = dist (e1);
        if (random_number < probability * 0xFFFFFFFF)
            return false;
    }
    return true;
}


std::future<void> tcp_mote::test (message msg, const int destination) {
    return std::async (std::launch::async, &tcp_mote::send, this, msg, destination);
}

