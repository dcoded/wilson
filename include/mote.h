#pragma once
#ifndef INCLUDE_MOTE_H
#define INCLUDE_MOTE_H

#include <iostream>
#include <functional>
#include <map>
#include <atomic>

#include <event.h>
#include <point.h>
#include <listener.h>


struct routing {
    int id;
    std::map <int, double> routes;
};

template <class Protocol>
class mote : public listener <Protocol>, public event <Protocol>
           , public listener <routing> , public event <routing> {

private:
    point location_;
    int   id_;

    std::map <int, std::atomic<int> >    next_hop_;
    std::map <int, std::atomic<double> > distance_;

public:
    /* Prevent ambiguity due to multiple inheritance */
    using event <Protocol>::publish;
    using event <Protocol>::subscribe;

    using event <routing>::publish;
    using event <routing>::subscribe;


    virtual void recv (const Protocol msg, const std::string event_name) = 0;
    virtual void send (Protocol msg, const int destination) = 0;

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

protected:
    const int next_hop (const int address);

private:
    // listen for routing table updates
    void recv (const routing msg, const std::string event_name);
};



/**
 * Constructors
 */
template <class Protocol>
mote<Protocol>::mote () {}




/**
 * Location getter/setter
 */
template <class Protocol>
const point mote<Protocol>::location () const { return location_; }

template <class Protocol>
void mote<Protocol>::location (const point location) {
    location_ = location;
}




/**
 * UUID getter/setter
 */
template <class Protocol>
const int mote<Protocol>::uuid () const { return id_; }

template <class Protocol>
void mote<Protocol>::uuid (const int id) {
    id_ = id;
    this->event<Protocol>::name (std::string("Channel [protocol,") + std::to_string(id) + "]");
    this->event<routing>::name (std::string("Channel [routing,") + std::to_string(id) + "]");
    //this->event<confirm>::name (std::string("Channel [confirm,") + std::to_string(id) + "]");
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

        for (auto& d : distance_)
            forward.routes[d.first] = d.second;
        
        publish (forward);
    }
}




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
        mote* neighbor = dynamic_cast <mote*> (l);
        
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
    
    for (auto& d : distance_)
        update.routes[d.first] = d.second;

    publish (update);
}



/**
 * Helper class for child implementation to retrieve next hop
 */
template <class Protocol>
const int mote<Protocol>::next_hop (const int address) {
    if (next_hop_.find (address) != next_hop_.end ()) 
        return next_hop_[address];
    else
        return -1;
}

#endif