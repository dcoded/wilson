#pragma once

template <typename T>
class listener {
public:
	listener () {}
	~listener () {}
	virtual void recv (const T message, const std::string event_name) = 0;
};

/* METRICS: bits sent, bits recv'd, # of retransmissions */