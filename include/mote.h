#pragma once
#include <functional>
#include <random>
#include <map>

#include <event.h>
#include <point.h>
#include <listener.h>

// Seed with a real random value, if available
std::random_device rd;
std::default_random_engine e1 (rd ());

//using message = std::string;
struct message
{
	int source;
	int next;
	int dest;

	std::string data;
};


class mote : public listener <message>
           , public event <message> {
using Edge = std::tuple <int, int, double>;
private:
	point location_;
	int id_;

	std::map <int, int> next_;
public:
	mote ();
	mote (point location);

	const point location () const;

	const int id () const;
	void id (const int id);

	void send (message msg);
	void recv (const message msg, const std::string event_name);

	const std::vector<mote*> get_listeners () const;

	void update_routes (std::vector<mote>& motes, std::set<Edge> edges);
};

mote::mote ()
{
	std::uniform_int_distribution <int> dist (0, 32);
	
	int x = dist (e1);
	int y = dist (e1);

	location_ = point {x,y};	
}

mote::mote (point location) 
: location_ (location) {}

const point mote::location () const {
	return location_;
}

const int mote::id () const {
	return id_;
}

void mote::id (const int id) {
	id_ = id;
	name (std::string("Channel [") + std::to_string(id) + "]");
}

void mote::recv (message msg, const std::string event_name)
{
	if (msg.dest == id ()) {
		std::cout
			<< "[mote[" << id_ << "]::recv||" << event_name << "] "
			<< "recieved data: " << msg.data << "\n";
	}
	else if (msg.next == id ()) {
		std::cout 
			<< "[mote[" << id_ << "]::recv||" << event_name << "] "
			<< "forwarding to dest " << msg.dest << "\n";
		publish (msg);
	}
	//add_noise (&bytes[0], bytes.size (), 0.5);
}

void mote::send (message msg) {

	msg.next = next_[msg.dest];

	if (msg.next < 0)
		std::cout
			<< "[mote[" << id_ << "]::send] "
			<< " cannot compute hop to destination " << msg.dest << "\n";
	else {
		std::cout
			<< "[mote[" << id_ << "]::send] "
			<< "(" << msg.source << "," << msg.next << "," << msg.dest << ")\n";

		publish (msg); 
	}
}

const std::vector<mote*> mote::get_listeners () const {
	std::vector <mote*> motes;

	for (auto* listener : listeners_)
	{
		mote* mptr = reinterpret_cast <mote*> (listener);
		motes.push_back (mptr);
	}

	return motes;
}

void mote::update_routes (std::vector<mote>& motes, std::set<Edge> edges) {
	std::cout << "[" << id () << "::update_routes]\n";

	std::map<int,double> dist;

	for (int i = 0; i < motes.size (); i++)
	{
		int nid = motes[i].id ();
		if (nid == id ())
			dist[nid] = 0;
		else
			dist[nid] = 0x0FFFFFFE;

		next_[nid] = -1;
	}

	for (const mote& m : motes)
	for (const Edge& e : edges)
	{
		int u,v,w;
		std::tie (u,v,w) = e;

		if (dist[u] + w < dist[v]) {
			dist[v]  = dist[u] + w;
			next_[v] = u;
		}
	}

	for (auto& entry : next_)
	{
		if (entry.second == id ())
			entry.second = entry.first;

		std::cout << "next[" << entry.first << "] = " << entry.second << "\n";
	}
}

// void add_noise (uint8_t* data, size_t len, double probability)
// {	
// 	std::uniform_int_distribution <int> dist (0, 0xFFFFFFFF);

// 	for (int i = 0; i < len; i++)
// 	for (int j = 0; j < 8; j++)
// 	{
// 		if (dist (e1) <= probability * 0xFFFFFFFF)
// 		{
// 			data[i] ^= (1 << j);
// 		}
// 	}
// }