#pragma once
#include <vector>
#include <functional>

#include <listener.h>

template <typename T>
class event {
private:
	std::string name_;
	std::vector <listener<T>*> listeners_;
public:
	event ();
	event (const std::string name);
	~event ();

	const std::string name () const;
	void name (const std::string name);

	void subscribe (listener<T>* ln);
	void publish (const T& message) const;
};

template <typename T>
event<T>::event () {}

template <typename T>
event<T>::event (const std::string name)
: name_ (name) {}

template <typename T>
event<T>::~event () {}

template <typename T>
const std::string event<T>::name () const {
	return name_;
}

template <typename T>
void event<T>::name (const std::string name) {
	name_ = name;
}

template <typename T>
void event<T>::subscribe (listener<T>* ln) {
	listeners_.push_back (ln);
}

template <typename T>
void event<T>::publish (const T& message) const {
	for (auto& listener : listeners_)
		listener->recv (message, name_);
}