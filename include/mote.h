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


void add_noise (uint8_t* data, size_t len, double probability) { 
    std::random_device rd;
    std::default_random_engine e1 (rd ());   
    std::uniform_int_distribution <int> dist (0, 0x7FFFFFFF);

    for (int i = 0; i < len; i++)
    for (int j = 0; j < 8; j++)
    {
        int random_number = dist (e1);
        if (random_number < probability * 0xFFFFFFFF)
            data[i] ^= (1 << j);
    }
}

//using message = std::string;
struct transmission {
    int source;
    int dest;

    int next;
    int prev;
};

// r update = routing update
struct routing {
    int id;
    std::map <int, double> routes;
};

// struct message : public transmission {
//     std::string data;
// };

// struct tcplite {
//     uint16_t source;
//     uint16_t dest;
//     uint16_t length;
//     uint16_t checksum;

//     std::string data;
// };

// struct tcp {
//     uint32_t source;
//     uint32_t dest;
//     uint32_t length;
//     uint32_t checksum;

//     std::string data;
// };

template <class Protocol>
class mote : public listener <Protocol>, public event <Protocol>
           , public listener <routing> , public event <routing>
           //, public listener <confirm> , public event <confirm>
{

private:
    point location_;
    int   id_;

    std::map <int, int>    next_hop_;
    std::map <int, double> distance_;
public:
    mote (mote const&) = delete;

    using event <Protocol>::subscribe;
    using event <routing>::subscribe;
    //using event <confirm>::subscribe;

    using event <Protocol>::publish;
    using event <routing>::publish;
    //using event <confirm>::publish;
    mote ();

    const point location () const;
    void location (const point location);

    // get and set mote id (a.k.a address)
    const int uuid () const;
    void      uuid (const int id);

    // allows neighbors to "hear" radio broadcasts
    void subscribe (mote& neighbor);

    // initialize routing table
    void discover ();

    // broadcast routing table to network
    void invocate () const;

    const int next_hop (const int address);

private:
    // listen for routing table updates
    virtual void recv (const routing msg, const std::string event_name);

    // listen for transmission acknowledgements
    // virtual void recv (const confirm msg, const std::string event_name);
};






/**
 * Constructors
 */
template <class Protocol>
mote<Protocol>::mote () {}


/**
 * Field Access
 */
template <class Protocol>
const point mote<Protocol>::location () const { return location_; }

template <class Protocol>
const int   mote<Protocol>::uuid () const { return id_; }

template <class Protocol>
void mote<Protocol>::uuid (const int id) {
    id_ = id;
    this->event<Protocol>::name (std::string("Channel [message,") + std::to_string(id) + "]");
    this->event<routing>::name (std::string("Channel [routing,") + std::to_string(id) + "]");
    //this->event<confirm>::name (std::string("Channel [confirm,") + std::to_string(id) + "]");
}

template <class Protocol>
void mote<Protocol>::location (const point location) {
    location_ = location;
}




/**
 * Subscribe neighbors (listeners) to both an application message handler
 * and a routing message handler.
 */
template <class Protocol>
void mote<Protocol>::subscribe (mote& neighbor) {
    subscribe (static_cast<listener<Protocol>*>(&neighbor));
    subscribe (static_cast<listener<routing>*>(&neighbor));
    //subscribe (static_cast<listener<confirm>*>(&neighbor));
}








/**
 * Routing Layer Message Handlers
 */
template <class Protocol>
void mote<Protocol>::recv (const routing update, const std::string event_name) {
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
        routing forward;
        forward.id     = id_;
        forward.routes = distance_;
        publish (forward);
    }
}


// template <class Protocol>
// void mote<Protocol>::recv (const confirm msg, const std::string event_name) {
//     if (msg.dest == id ())
//         std::cout
//             << "[mote[" << id_ << "]::recv(" << event_name << ")] "
//             << "recieved confirm: " << msg.source << "\n";
// }





/**
 * Initialize routing table with neighbors
 */
template <class Protocol>
void mote<Protocol>::discover () {
    // clear any old table (or leftover memory)
    distance_.clear ();
    next_hop_.clear ();

    // find neighbors as listeners and add to table
    for (listener<routing>* l : this->event<routing>::listeners ())
    {
        mote* neighbor = static_cast <mote*> (l);
        
        distance_[neighbor->uuid ()] = location ().distance (neighbor->location ());
        next_hop_[neighbor->uuid ()] = neighbor->uuid ();
    }

    distance_[id_] = 0;   // route to self has no distance
    next_hop_[id_] = id_; // route to self via self...duh
}

/**
 * Start network propigation of this motes routing table
 */
template <class Protocol>
void mote<Protocol>::invocate () const {
    // copy routing table
    routing update;
    update.id     = id_;
    update.routes = distance_;

    publish (update);
}


template <class Protocol>
const int mote<Protocol>::next_hop (const int address) {
    if (next_hop_.find (address) != next_hop_.end ()) 
        return next_hop_[address];
    else
        return -1;
}

#endif