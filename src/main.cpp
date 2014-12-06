#include <iostream>
#include <mote.h>
#include <event.h>





int main(int argc, char** argv)
{
	const int RADIO_STRENGTH = 5;

	std::vector <mote> motes (32);

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
			// std::cout << "[info] " << motes[i].name () << " in range of " << motes[j].name () << "\n";
			// std::cout << "[info] " << std::string(a) << " <> " << std::string(b) << "\n";
			motes[i].subscribe (reinterpret_cast<mote::listener*> (&motes[j]));
		}
	}



	motes[0].publish ({ 0x0015, 0x0023, "test"});


	std::cout << std::endl;
}

