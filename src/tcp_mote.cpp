#include <tcp_mote.h>


std::future<bool> tcp_mote::connect (const int destination) {
    std::future <bool> future;

    if (state_.find (destination) == state_.end ())
        state_[destination] = TCP_STATE_CLOSED;

    if (state_[destination] == TCP_STATE_CLOSED) {
        tcp_message msg;
        {
            msg.flags &= TCP_NOFLAGS;
            msg.flags |= TCP_SYN;
            msg.source = uuid ();
            msg.dest   = destination;
        }
        state_[destination] = TCP_STATE_SYN_SENT;
        std::cout << "[" << uuid () << ":" << destination << "] SYN_SENT\n";

        future = std::async (std::launch::async, &tcp_mote::send, this, msg, destination);
    } else {
        future = std::async ([]() -> bool { return false; });
        std::cout << "connect(): error, dest is already open\n";
    }

    return future;
    // while != DATA?
}

std::future <bool> tcp_mote::close (const int destination) {

    if (state_.find (destination) == state_.end ())
        state_[destination] = TCP_STATE_CLOSED;

    if (state_[destination] != TCP_STATE_CLOSED) {
        tcp_message msg;
        {
            msg.flags &= TCP_NOFLAGS;
            msg.flags |= TCP_FIN;
            msg.source = uuid ();
            msg.dest   = destination;
        }
        state_[destination] = TCP_STATE_FIN_WAIT_1;
        std::cout << "[" << uuid () << ":" << destination << "] FIN_WAIT_1\n";

        return std::async (std::launch::async, &tcp_mote::send, this, msg, destination);
    }    
    return std::async ([]() -> bool { return false; });
}




/**
 * Application Message Handlers
 */
void tcp_mote::recv (tcp_message msg, const std::string event_name) {
    // last arg is a percentage from 0.0 to 1.0
    //create_interference (msg, 0.0);
    if (not_interfered (msg, 0/*0.0001*/))
    {
        if (msg.dest == uuid () || msg.next == uuid ()) {
            bytes_recv += sizeof (tcp_header) - sizeof (transmission);
            msgs_recv ++;
        }

             if (msg.dest == uuid ()) respond (msg);
        else if (msg.next == uuid ()) send (msg, msg.dest);
    }
}


void tcp_mote::respond (tcp_message& msg) {

    bool reset = false;

    if (state_.find (msg.source) == state_.end ()) {
        state_[msg.source] = TCP_STATE_CLOSED;
    }

    tcp_message rst;
    {
        rst.source = msg.dest;
        rst.dest   = msg.source;
        rst.flags &= (TCP_NOFLAGS | TCP_RST);
    }

    switch(state_[msg.source])
    {
        case TCP_STATE_CLOSED:
            if (msg.flags & TCP_SYN) {
                state_[msg.source] = TCP_STATE_SYN_RECV;
                std::cout << "[" << msg.source << ":" << uuid () << "] SYN_RECV\n";

                std::swap (msg.dest, msg.source);

                msg.flags &= TCP_NOFLAGS;
                msg.flags |= (TCP_SYN | TCP_ACK);

                msg.size = 0;
                msg.seq  = rand ();
                send (msg, msg.dest);
            }
            else reset = true;
        break;

        case TCP_STATE_SYN_SENT: // SENDER MOTE
            if (msg.flags & (TCP_SYN | TCP_ACK)) {
                std::cout << "[" << uuid () << ":" << msg.source << "] SENDER ESTABLISHED\n";
                state_[msg.source] = TCP_STATE_CLIENT_ESTABLISHED;
                
                std::swap (msg.dest, msg.source);

                msg.flags &= TCP_NOFLAGS;
                msg.flags |= TCP_ACK;
                msg.ack  = rand ();
                msg.size = 0;
                send (msg, msg.dest);
            } else reset = true;
        break;

        case TCP_STATE_SYN_RECV: // RECEIVER MOTE
            if (msg.flags & TCP_ACK) {
                std::cout << "[" << msg.source << ":" << uuid () << "] RECEIVER ESTABLISHED\n";
                state_[msg.source] = TCP_STATE_SERVER_ESTABLISHED;
            } else reset = true;
        break;

        case TCP_STATE_SERVER_ESTABLISHED:
            if (msg.flags & TCP_FIN) {
                std::cout << "[" << msg.source << ":" << uuid () << "] CLIENT CLOSING\n";
                state_[msg.source] = TCP_STATE_LAST_ACK;

                std::swap (msg.source, msg.dest);

                msg.flags &= TCP_NOFLAGS;
                msg.flags |= TCP_ACK;

                send (msg, msg.dest);

                msg.flags &= TCP_NOFLAGS;
                msg.flags |= TCP_FIN;

                send (msg, msg.dest);
            } else reset = true;
        break;

        case TCP_STATE_CLIENT_ESTABLISHED:
        break;

        case TCP_STATE_LAST_ACK:
            if (msg.flags & TCP_ACK) {
                std::cout << "[" << msg.source << ":" << uuid () << "] RECEIVER CLOSED\n";
                state_[msg.source] = TCP_STATE_CLOSED;
            } else reset = true;
        break;

        case TCP_STATE_FIN_WAIT_1:
            if (msg.flags & TCP_ACK) {
                state_[msg.source] = TCP_STATE_FIN_WAIT_2;
            } else reset = true;
        break;

        case TCP_STATE_FIN_WAIT_2:
            if (msg.flags & TCP_FIN) {
                std::cout << "[" << uuid () << ":" << msg.source << "] SENDER CLOSED\n";
                state_[msg.source] = TCP_STATE_CLOSED;

                std::swap (msg.source, msg.dest);

                msg.flags &= TCP_NOFLAGS;
                msg.flags |= TCP_ACK;

                send (msg, msg.dest);
            } else reset = true;
        break;
    }

    if (reset)
    {
        msg = rst;
        state_[rst.dest] = TCP_STATE_CLOSED;
        std::cout << "[" << msg.source << ":" << uuid () << "] RESET / CLOSED\n";
        send (msg, msg.dest);
    }
}



bool tcp_mote::send (tcp_message msg, const int destination) {
    msg.next   = next_hop (destination);
    msg.prev   = uuid ();
    msg.dest   = destination;

    if (msg.next == -1) { // cant calculate path
        std::cout << "no path to destination\n";
    } else {
        std::cout << msg.source << " -> (" << uuid () << "," << next_hop(msg.dest) << ") -> " << msg.dest << "\n";
            
        bytes_sent += sizeof (tcp_header) - sizeof (transmission);
        msgs_sent ++;
        publish (msg);
    }

    return (msg.next != -1);
}


void tcp_mote::create_interference (tcp_message& msg, double probability) {
    uint8_t* data = reinterpret_cast <uint8_t*> (&msg);
    size_t   len  = sizeof (tcp_message);

    std::random_device rd;
    std::default_random_engine e1 (rd ());   
    std::uniform_int_distribution <uint32_t> dist (0, 0xFFFFFFFF);

    for (size_t i = 0; i < len; i++)
    for (size_t j = 0; j < 8; j++)
    {
        uint32_t random_number = dist (e1);
        if (random_number < probability * 0xFFFFFFFF)
            data[i] ^= (1 << j);
    }
}

bool tcp_mote::not_interfered (tcp_message& msg, double probability) {
    std::random_device rd;
    std::default_random_engine e1 (rd ());   
    std::uniform_int_distribution <uint32_t> dist (0, 0xFFFFFFFF);

    for (size_t i = 0; i < sizeof (msg) * 8; i++)
    {
        uint32_t random_number = dist (e1);
        if (random_number < probability * 0xFFFFFFFF)
            return false;
    }
    return true;
}

bool tcp_mote::connected (const int destination) {
    if (state_.find (destination) == state_.end ())
        return false;

    return (state_[destination] == TCP_STATE_CLIENT_ESTABLISHED);
}
