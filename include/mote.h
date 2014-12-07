#pragma once
#ifndef INCLUDE_MOTE_H
#define INCLUDE_MOTE_H

#include <iostream>
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

    mote (); // creates a random location
    mote (point location);

    const point location () const;

    // get and set mote id (a.k.a address)
    const int id () const;
    void      id (const int id);

    // allows neighbors to "hear" radio broadcasts
    void subscribe (mote& neighbor);

    // send/recv an application message to/from another mote in the network
    void send (message msg);
    void recv (const message msg, const std::string event_name);

    // listen for routing table updates
    void recv (const rupdate msg, const std::string event_name);
    
    // initialize routing table
    void discover ();

    // broadcast routing table to network
    void invocate () const;
};






/**
 * Constructors
 */
mote::mote () {
    std::uniform_int_distribution <int> dist (0, 32);
    
    int x = dist (e1);
    int y = dist (e1);

    location_ = point {x,y};    
}

mote::mote (point location) 
: location_ (location) {}





/**
 * Field Access
 */
const point mote::location () const { return location_; }
const int   mote::id () const { return id_; }

void mote::id (const int id) {
    id_ = id;
    this->event<message>::name (std::string("Channel [message,") + std::to_string(id) + "]");
    this->event<rupdate>::name (std::string("Channel [rupdate,") + std::to_string(id) + "]");
}




/**
 * Subscribe neighbors (listeners) to both an application message handler
 * and a routing message handler.
 */
void mote::subscribe (mote& neighbor) {
    subscribe (static_cast<listener<message>*>(&neighbor));
    subscribe (static_cast<listener<rupdate>*>(&neighbor));
}





/**
 * Application Message Handlers
 */
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
    if (next_hop_.find (msg.dest) == next_hop_.end ()) {
        // we cannot find a neighbor who knows of the destination
        std::cout
            << "[mote[" << id_ << "]::send] "
            << " cannot compute hop to destination " << msg.dest << "\n";
    } else {
        // update the message to notify correct neighbor (next)
        msg.next = next_hop_[msg.dest];
        std::cout
            << "[mote[" << id_ << "]::send] "
            << "(" << msg.source << "," << msg.next << "," << msg.dest << ")"
            << "\n";

        // pass it on
        publish (msg);
    }
}





/**
 * Routing Layer Message Handlers
 */
void mote::recv (rupdate update, const std::string event_name) {
    bool updated = false;

    for (auto route : update.routes)
    {
           int neighbor = update.id;    // neighbor who sent it here
           int endpoint = route.first;  // destination target id
        double distance = route.second; // distance from neighbor

        if (distance_.find (endpoint) == distance_.end ()) {
            // this mote doesn't know if this peer yet
            distance_[endpoint] = distance + distance_[neighbor];
            next_hop_[endpoint] = neighbor;
            updated = true;
        }
        else if (distance_[neighbor] + distance < distance_[endpoint]) {
            // neighbor has a shorter path
            distance_[endpoint] = distance + distance_[neighbor];
            next_hop_[endpoint] = neighbor;
            updated = true;
        }
    }

    // if a change has occurred, propagate table to neighbors
    if (updated) {
        update.id = id_;
        update.routes = distance_;
        publish (update);
    }
}








/**
 * Initialize routing table with neighbors
 */
void mote::discover () {
    // clear any old table (or leftover memory)
    distance_.clear ();
    next_hop_.clear ();

    // find neighbors as listeners and add to table
    for (listener<message>* l : this->event<message>::listeners ())
    {
        mote* neighbor = static_cast <mote*> (l);
        
        distance_[neighbor->id ()] = location ().distance (neighbor->location ());
        next_hop_[neighbor->id ()] = neighbor->id ();
    }

    distance_[id_] = 0;   // route to self has no distance
    next_hop_[id_] = id_; // route to self via self...duh
}

/**
 * Start network propigation of this motes routing table
 */
void mote::invocate () const {
    // copy routing table
    rupdate update;
    update.id     = id_;
    update.routes = distance_;

    publish (update);
}

#endif






// void add_noise (uint8_t* data, size_t len, double probability)
// {    
//     std::uniform_int_distribution <int> dist (0, 0xFFFFFFFF);

//     for (int i = 0; i < len; i++)
//     for (int j = 0; j < 8; j++)
//     {
//         if (dist (e1) <= probability * 0xFFFFFFFF)
//         {
//             data[i] ^= (1 << j);
//         }
//     }
// }