#pragma once
#ifndef INCLUDE_POINT_H
#define INCLUDE_POINT_H

#include <initializer_list>
#include <type_traits>
#include <vector>
#include <sstream>
#include <string>
#include <cassert>
#include <cmath>

class point {
private:
    std::vector <int> coordinates_;

public:
    explicit point ();
    explicit point (std::initializer_list <int> coordinates);

    template <typename Iter>
    point (Iter begin, Iter end)
    : coordinates_ (begin, end) {}

    size_t size () const;

    const double distance (const point& other) const;

    std::vector<int>::const_iterator begin () const;
    std::vector<int>::const_iterator end () const;

    point* operator= (point* rhs);
    const int operator[] (size_t index) const;
    bool operator< (const point& rhs) const;

    void operator+= (point& rhs);
    void operator-= (point& rhs);
    void operator*= (point& rhs);
    void operator/= (point& rhs);
    void operator%= (point& rhs);

    operator std::string () const;
};

point operator+ (const point& lhs, const point& rhs);
point operator- (const point& lhs, const point& rhs);
point operator* (const point& lhs, const point& rhs);
point operator/ (const point& lhs, const point& rhs);
point operator% (const point& lhs, const point& rhs);

std::ostream& operator << (std::ostream& s, const std::string& str);

#endif