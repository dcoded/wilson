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
	int dest;

	std::string data;
};


class mote : public listener <message>
           , public event <message> {
private:
	point location_;
	int id_;

	std::map <int, int> routing_;
public:
	mote ();
	mote (point location);

	const point location () const;

	const int id () const;
	void id (const int id);

	void send (const message msg);
	void recv (const message msg, const std::string event_name);
};

mote::mote ()
{
	std::uniform_int_distribution <int> dist (0, 100);
	
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
	std::cout << "[mote[" << id_ << "]::recv||" << event_name << "] recieved data: " << msg.data << "\n";
	//add_noise (&bytes[0], bytes.size (), 0.5);
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