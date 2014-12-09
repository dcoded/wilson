#include <iostream>
#include <tcp_mote.h>



int main(int argc, char** argv)
{
    /*
        SIMULATION SETTINGS

        Note: Networks are randomly generated and those with a high SPACE_SIZE
              and/or low COUNT_MOTES or RADIO_STRENGTH will increase the
              likelyhood of an isolated node.
    */
    /** 10,5,4 showed that sent==recv */
    const int SPACE_SIZE     = 128; // size of area sensors can be placed (NxN)
    const int COUNT_MOTES    = 64; // # of sensors around
    const int RADIO_STRENGTH = 30; // how far a sensor can communicate


    std::vector <tcp_mote> motes;

    // place sensors in random locations
    for (int i = 0; i < COUNT_MOTES; i++) {
        motes.push_back (tcp_mote(SPACE_SIZE));
    }

    // assign IDs/addresses
    for (int i = 0; i < motes.size (); i++) {
        motes[i].id (i);
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
                << "[info] " << motes[i].id () << " in range of "
                << motes[j].id () << "\n";

            motes[i].subscribe (motes[j]);
        }
    }

    // create routing tables
    std::cout << "Detecting neighbors...\n";
    for (auto& mote : motes) mote.discover ();

    std::cout << "Setting up DV routing tables...\n";
    for (auto& mote : motes) mote.invocate ();

    // make a test message!

    // 0x0004 is the destination address
    for (int i = 0; i < motes.size (); i++)
    for (int j = 0; j < motes.size (); j++) {
        if (i == j) continue;

        std::stringstream ss;
        ss << "Hi " << j << " I'm  " << i;

        message test;
        test.data = ss.str ();

        motes[i].send (test, motes[j].id ());
    }



    /*
        Below are some basic metrics to identify transmission performance
    */
    int sent_bits = 0;
    int recv_bits = 0;
    int sent_msgs = 0;
    int recv_msgs = 0;

    auto summation = [] (const int a, const int b) { return a + b; };

    aggregate_metric<int> (motes.begin (), motes.end (), sent_bits, "sent_bits", summation);
    aggregate_metric<int> (motes.begin (), motes.end (), recv_bits, "recv_bits", summation);
    aggregate_metric<int> (motes.begin (), motes.end (), sent_msgs, "sent_messages", summation);
    aggregate_metric<int> (motes.begin (), motes.end (), recv_msgs, "recv_messages", summation);

    std::cout << "Bits Sent: " << sent_bits << "\n";
    std::cout << "Bits Recv: " << recv_bits << "\n";

    std::cout << "Msgs Sent: " << sent_msgs << "\n";
    std::cout << "Msgs Recv: " << recv_msgs << "\n";

    std::cout << std::endl;
}