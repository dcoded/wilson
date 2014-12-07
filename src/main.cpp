#include <iostream>
#include <set>

#include <mote.h>
#include <event.h>

using Edge = std::tuple <int, int, double>;


std::set<Edge> discover_edges (std::vector <mote>& motes);

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
			std::cout << "[info] " << motes[i].id () << " in range of " << motes[j].id () << "\n";
		//	std::cout << "[info] " << std::string(a) << " <> " << std::string(b) << "\n";
			motes[i].subscribe (reinterpret_cast<mote::listener*> (&motes[j]));
		}
	}

	std::set<Edge> edges = discover_edges (motes);
	for (mote& m : motes)
		m.update_routes (motes, edges);

	motes[1].send ({ 0x0001, -1, 0x004, "test"});

	std::cout << std::endl;
}

std::set<Edge> discover_edges (std::vector <mote>& motes)
{
	std::set <Edge> edges;

	for (mote m : motes)
	for (mote* n : m.get_listeners ()) {

		edges.emplace (std::make_tuple (
			m.id (),
			n->id (),
			m.location (). distance (n->location ())
		));
	}
	

	for (Edge e : edges)
	{
		int u,v,w;

		std::tie (u,v,w) = e;
		std::cout << u << " <- " << v << "\n";
	}

	return edges;
}