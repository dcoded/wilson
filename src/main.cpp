#include <iostream>
#include <mote.h>

int main(int argc, char** argv)
{
    const int RADIO_STRENGTH = 15;

    std::vector <mote> motes (8);

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

    for (mote& m : motes) m.discover ();
    for (mote& m : motes) m.invocate ();

    message test;
    test.data = "Hello World!";

    motes[1].send (test, 0x0004);

    int sent_bits = 0;
    int recv_bits = 0;
    int sent_msgs = 0;
    int recv_msgs = 0;

    auto summation = [] (const int a, const int b) { return a + b; };

    aggregate_metric<int> (sent_bits, motes.begin (), motes.end (), "sent_bits", summation);
    aggregate_metric<int> (recv_bits, motes.begin (), motes.end (), "recv_bits", summation);
    aggregate_metric<int> (sent_msgs, motes.begin (), motes.end (), "sent_messages", summation);
    aggregate_metric<int> (recv_msgs, motes.begin (), motes.end (), "recv_messages", summation);

    std::cout << "Bits Sent: " << sent_bits << "\n";
    std::cout << "Bits Recv: " << recv_bits << "\n";

    std::cout << "Msgs Sent: " << sent_msgs << "\n";
    std::cout << "Msgs Recv: " << recv_msgs << "\n";

    std::cout << std::endl;
}