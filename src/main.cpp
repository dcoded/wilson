#include <iostream>
#include <future>
#include <random>
#include <tcp_mote.h>
#include <udp_mote.h>

// Seed with a real random value, if available

template <typename mote_type, typename pckt_type>
void simulate (const int COUNT_MOTES, const int SPACE_SIZE, const int RADIO_STRENGTH);

template <typename Iter>
void init_network (const Iter& begin, const Iter& end, const int size);

template <typename T>
void setup_pubsub (std::vector <T>& motes, const int RADIO_STRENGTH);

template <typename T>
void setup_routes (std::vector <T>& motes);

template <class Iter, class Job>
void parallel_job (const Iter& begin, const Iter& end, Job fn);

int main(int argc, char** argv)
{
    if (argc < 5) {
        std::cout << "Usage: " << argv[0] << " { tcp | udp } <size> <motes> <radio strength>\n";
        return 0;
    }
    /*
        SIMULATION SETTINGS

        Note: Networks are randomly generated and those with a high SPACE_SIZE
              and/or low COUNT_MOTES or RADIO_STRENGTH will increase the
              likelyhood of an isolated node.
    */
    /** 10,5,4 showed that sent==recv */
    const int SPACE_SIZE     = atoi (argv[2]); // size of area sensors can be placed (NxN)
    const int COUNT_MOTES    = atoi (argv[3]); // # of sensors around
    const int RADIO_STRENGTH = atoi (argv[4]); // how far a sensor can communicate


    if (std::string(argv[1]) == std::string ("tcp")) {
        std::cout << "Simulating TCP\n";
        simulate <tcp_mote, tcp_message> (COUNT_MOTES, SPACE_SIZE, RADIO_STRENGTH);
    } else {
        std::cout << "Simulating UDP\n";
        simulate <udp_mote, udp_message> (COUNT_MOTES, SPACE_SIZE, RADIO_STRENGTH);
    }


}


template <typename mote_type, typename pckt_type>
void simulate (const int COUNT_MOTES, const int SPACE_SIZE, const int RADIO_STRENGTH) {

    std::vector <mote_type> motes (COUNT_MOTES);



    // assign IDs/addresses & spatial locations
    init_network (motes.begin (), motes.end (), SPACE_SIZE);

    /* create pub/sub channel between two motes if they have a
       Euclidian distance < RADIO_STRENGTH.
    */
    setup_pubsub (motes, RADIO_STRENGTH);

    /* Perform distributed distance vector routing to create
       routing tables for each mote.
    */
    setup_routes (motes);

    // parallel_job (motes.begin (), motes.end (),
    //     std::bind (&mote_type::init, std::placeholders::_1));

    int connections = 0;

    for (size_t i = 0; i < motes.size (); i++)
    for (size_t j = 0; j < motes.size (); j++)
    {
        pckt_type msg;
        msg.data = "foo";
        motes[i].connect (motes[j].uuid ());
        
        if (motes[i].connected (motes[j].uuid ())) {
            if (motes[i].send (msg, motes[j].uuid ())) {
                connections++;
            }
            motes[i].close (motes[j].uuid ());
        }
    }


    // /*
    //     Below are some basic metrics to identify transmission performance
    // */
    int bytes_sent = 0;
    int bytes_recv = 0;
    int msgs_sent = 0;
    int msgs_recv = 0;

    for (mote_type& m : motes) {
        bytes_sent += m.bytes_sent;
        bytes_recv += m.bytes_recv;
        msgs_sent += m.msgs_sent;
        msgs_recv += m.msgs_recv;
    }

    std::cout << "Bytes Sent   : " << bytes_sent << "\n";
    std::cout << "Bytes Recv   : " << bytes_recv << "\n";
    std::cout << "\n";
    std::cout << "Messages Sent: " << msgs_sent << "\n";
    std::cout << "Messages Recv: " << msgs_recv << "\n";
    std::cout << "Connections  : " << connections << "\n";
    std::cout << "\n";
    std::cout << "Bytes / Conn : " << int (bytes_sent / connections) << " bytes per connection\n";
    std::cout << "Avg. Distance: " << int (msgs_sent / connections)  << " hops per connection\n";

    std::cout << std::endl;
}




template <typename Iter>
void init_network (const Iter& begin, const Iter& end, const int size) {
    std::random_device rd;
    std::default_random_engine e1 (rd ());
    std::uniform_int_distribution <int> dist (0, size);

    int id = 0;
    for (Iter it = begin; it != end; it++) {
        int x = dist (e1);
        int y = dist (e1);

        point location {x,y};

        it->uuid (id);
        it->location (location);


        std::cout << "[init] " << id << " @ " << location << "\n";
        id++;
    }
}



template <typename T>
void setup_pubsub (std::vector <T>& motes, const int RADIO_STRENGTH) {
    // initialize pub/sub for two motes if Euclidian distance < RADIO_STRENGTH
    for (size_t i = 0; i < motes.size (); i++)
    for (size_t j = 0; j < motes.size (); j++)
    {
        if (i == j) continue;

        point a = motes[i].location ();
        point b = motes[j].location ();

        if (a.distance (b) <= RADIO_STRENGTH) {
            std::cout
                << "[info] " << motes[i].uuid () << " in range of "
                << motes[j].uuid () << "\n";

            motes[i].subscribe (motes[j]);
        }
    }
}

template <typename T>
void setup_routes (std::vector <T>& motes) {
    for (auto& mote : motes) mote.discover ();
    for (auto& mote : motes) mote.invocate ();
}


template <class Iter, class Job>
void parallel_job (const Iter& begin, const Iter& end, Job fn) {
    std::vector <std::future<void>> jobs;

    for (Iter it = begin; it != end; it++)
        jobs.push_back (fn (*it));

    for (auto& job : jobs) job.get ();
}