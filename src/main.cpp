#include <iostream>
#include <mote.h>

int main(int argc, char** argv)
{
    const int RADIO_STRENGTH = 15;

    std::vector <mote> motes (8);

    for (int i = 0; i < motes.size (); i++)
    {
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

    // std::set<Edge> edges = discover_edges (motes);
    // for (mote& m : motes)
    //     m.update_routes (motes, edges);


    for (mote& m : motes) m.discover ();
    for (mote& m : motes) m.invocate ();

    motes[1].send ({ 0x0001, -1, 0x004, "test"});

    std::cout << std::endl;
}