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

    int sent_bits_total = 0;
    int recv_bits_total = 0;

    int sent_msgs_total = 0;
    int recv_msgs_total = 0;

    for (mote m : motes) sent_bits_total += m.metric ("sent_bits");
    for (mote m : motes) recv_bits_total += m.metric ("recv_bits");

    for (mote m : motes) sent_msgs_total += m.metric ("sent_messages");
    for (mote m : motes) recv_msgs_total += m.metric ("recv_messages");



    std::cout << "Bits Sent: " << sent_bits_total << "\n";
    std::cout << "Bits Recv: " << recv_bits_total << "\n";

    std::cout << "Msgs Sent: " << sent_msgs_total << "\n";
    std::cout << "Msgs Recv: " << recv_msgs_total << "\n";

    std::cout << std::endl;
}