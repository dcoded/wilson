#include <iostream>
#include <future>
#include <random>
#include <tcp_mote.h>

// Seed with a real random value, if available

const point random_location (int max);

template <class Iter, class Job>
void parallel_job (const Iter& begin, const Iter& end, Job fn);

int main(int argc, char** argv)
{
    /*
        SIMULATION SETTINGS

        Note: Networks are randomly generated and those with a high SPACE_SIZE
              and/or low COUNT_MOTES or RADIO_STRENGTH will increase the
              likelyhood of an isolated node.
    */
    /** 10,5,4 showed that sent==recv */
    const int SPACE_SIZE     = 100; // size of area sensors can be placed (NxN)
    const int COUNT_MOTES    = 32; // # of sensors around
    const int RADIO_STRENGTH = 15; // how far a sensor can communicate

    using mote_type = tcp_mote;

    std::vector <mote_type> motes (COUNT_MOTES);

    // assign IDs/addresses & locations
    for (int i = 0; i < motes.size (); i++) {
        motes[i].uuid (i);
        motes[i].location (random_location (SPACE_SIZE));
    }

    // initialize pub/sub for two motes if Euclidian distance < RADIO_STRENGTH
    for (int i = 0; i < motes.size (); i++)
    for (int j = 0; j < motes.size (); j++)
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

    for (auto& mote : motes) mote.discover ();

    parallel_job (motes.begin (), motes.end (),
        std::bind (&mote_type::init, std::placeholders::_1));



    std::vector <std::future<void>> jobs;
    // send messages 
    for (int i = 0; i < motes.size (); i++)
    for (int j = 0; j < motes.size (); j++) {
        if (i == j) continue;

        std::stringstream ss;
        ss << "Hi " << j << " I'm  " << i;

        message test;
        test.data = ss.str ();

        jobs.push_back (motes[i].test(test, motes[j].uuid ()));
    }

    for (auto& job : jobs) job.get ();




    // /*
    //     Below are some basic metrics to identify transmission performance
    // */
    int sent_bits = 0;
    int recv_bits = 0;
    int sent_msgs = 0;
    int recv_msgs = 0;

    for (mote_type& m : motes) {
        sent_bits += m.bits_sent;
        recv_bits += m.bits_recv;
        sent_msgs += m.msgs_sent;
        recv_msgs += m.msgs_recv;
    }

    std::cout << "Bits Sent: " << sent_bits << "\n";
    std::cout << "Bits Recv: " << recv_bits << "\n";

    std::cout << "Msgs Sent: " << sent_msgs << "\n";
    std::cout << "Msgs Recv: " << recv_msgs << "\n";

    std::cout << std::endl;
}




const point random_location (int max) {
    std::random_device rd;
    std::default_random_engine e1 (rd ());
    std::uniform_int_distribution <int> dist (0, max);
    
    int x = dist (e1);
    int y = dist (e1);

    return point {x,y};   
}

template <class Iter, class Job>
void parallel_job (const Iter& begin, const Iter& end, Job fn) {
    std::vector <std::future<void>> jobs;

    for (Iter it = begin; it != end; it++)
        jobs.push_back (fn (*it));

    for (auto& job : jobs) job.get ();
}