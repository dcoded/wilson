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

struct rupdate
{
	int id;
	std::map <int, double> routes;
};


class mote : public listener <message> , public event <message>
           , public listener <rupdate> , public event <rupdate> {
using Edge = std::tuple <int, int, double>;
private:
	point location_;
	int   id_;

	std::map <int, int>    next_hop_;
	std::map <int, double> distance_;
public:
	using event <message>::subscribe;
	using event <rupdate>::subscribe;

	using event <message>::publish;
	using event <rupdate>::publish;

	mote ();
	mote (point location);

	const point location () const;

	const int id () const;
	void id (const int id);

	void send (message msg);
	void recv (const message msg, const std::string event_name);
	
	void recv (const rupdate msg, const std::string event_name);
	
	void discover ();
	void forward () const;
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
	this->event<message>::name (std::string("Channel [message,") + std::to_string(id) + "]");
	this->event<rupdate>::name (std::string("Channel [rupdate,") + std::to_string(id) + "]");
}





void mote::recv (message msg, const std::string event_name)
{
	if (msg.dest == this->mote::id ()) {
		std::cout
			<< "[mote[" << id_ << "]::recv(" << event_name << ")] "
			<< "recieved data: " << msg.data << "\n";
	}
	else if (msg.next == id ()) {
		std::cout 
			<< "[mote[" << id_ << "]::recv(" << event_name << ")] "
			<< "forwarding to dest " << msg.dest << "\n";
		send (msg);
	}
	//add_noise (&bytes[0], bytes.size (), 0.5);
}









void mote::send (message msg) {

	if (next_hop_.find (msg.dest) == next_hop_.end ())
	{
		std::cout
			<< "[mote[" << id_ << "]::send] "
			<< " cannot compute hop to destination " << msg.dest << "\n";
	}
	else
	{
		msg.next = next_hop_[msg.dest];
		std::cout
			<< "[mote[" << id_ << "]::send] "
			<< "(" << msg.source << "," << msg.next << "," << msg.dest << ")\n";
		publish (msg);
	}
}

void mote::recv (rupdate update, const std::string event_name) {
	bool updated = false;

	for (auto route : update.routes)
	{
		   int neighbor = update.id;
		   int endpoint = route.first;  // target ID
		double distance = route.second; // distance from neighbor

		if (distance_.find (endpoint) == distance_.end ()) {
			// this mote doesn't know if this peer yet
			distance_[endpoint] = distance + distance_[neighbor];
			next_hop_[endpoint] = neighbor;
			updated = true;

			std::cout
				<< "[+] " << id_ << " @ " << location_ << " -> " << endpoint
				<< " via " << neighbor
				<< " = "
				<< distance_[neighbor] << "+" << distance << " = " << distance_[endpoint]
				<< "\n";
		}
		else if (distance_[neighbor] + distance < distance_[endpoint]) {
			// neighbor has a shorter path
			distance_[endpoint] = distance + distance_[neighbor];
			next_hop_[endpoint] = neighbor;
			updated = true;

			std::cout
				<< "[^] " << id_ << " @ " << location_ << " -> " << endpoint
				<< " via " << neighbor
				<< " = "
				<< distance_[neighbor] << "+" << distance << " = " << distance_[endpoint]
				<< "\n";
		}
	}

	if (updated)
	{
		update.id = id_;
		update.routes = distance_;
		publish (update);
	}
}









void mote::discover () {
	// create list of neighbors
	distance_.clear ();
	next_hop_.clear ();

	for (listener<message>* l : this->event<message>::listeners ())
	{
		mote* neighbor = static_cast <mote*> (l);
		
		distance_[neighbor->id ()] = location ().distance (neighbor->location ());
		next_hop_[neighbor->id ()] = neighbor->id ();
	}

	distance_[id_] = 0;
	next_hop_[id_] = id_;
}

void mote::forward () const {
	// copy routing table
	rupdate update;
	update.id     = id_;
	update.routes = distance_;

	publish (update);
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