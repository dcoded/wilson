#include <iterator>
#include <iostream>
#include <point.h>
#include <algorithm>

point::point() {}

point::point (std::initializer_list <int> coordinates)
: coordinates_ (coordinates) {}

const int point::operator[] (size_t index) const {
    return coordinates_[index];
}

size_t point::size () const {
    return coordinates_.size ();
}

const double point::distance (const point& other) const {
    int sum = 0;
    for (size_t i = 0; i < size (); i++)
        sum += pow (coordinates_[i] - other[i], 2);
    
    return sqrt(sum);
}

std::vector<int>::const_iterator point::begin () const {
    return coordinates_.begin ();
}

std::vector<int>::const_iterator point::end () const {
    return coordinates_.end ();
}

point* point::operator= (point* rhs) {
    coordinates_ = std::move (rhs->coordinates_);
    return this;
}

bool point::operator< (const point& rhs) const {

    assert (size () == rhs.size ());

    for (size_t i = 0; i < size (); i++) {
        if (coordinates_[i] < rhs[i]) return true;
        if (coordinates_[i] > rhs[i]) return false;
    }
    return false;
}

void point::operator+= (point& rhs) { *this = *this + rhs; }
void point::operator-= (point& rhs) { *this = *this - rhs; }
void point::operator*= (point& rhs) { *this = *this * rhs; }
void point::operator/= (point& rhs) { *this = *this / rhs; }
void point::operator%= (point& rhs) { *this = *this % rhs; }

point::operator std::string () const{
    std::string formatted;
    std::string delimiter = ", ";

    std::ostringstream imploded;
    std::copy(
        coordinates_.begin (),
        coordinates_.end(),
        std::ostream_iterator<int> (imploded, delimiter.c_str ()));

    formatted = imploded.str ();
    formatted.resize (formatted.size () - delimiter.size ());

    return "Point(" + formatted + ')';
}



















std::ostream& operator << (std::ostream& s, const std::string& str) {
        s << str.c_str();
        return s;
}

point operator+ (const point& lhs, const point& rhs) {
    assert (lhs.size () == rhs.size ());

    std::vector <int> res;
    std::transform (
        lhs.begin (), lhs.end (), rhs.begin (),
        std::back_inserter (res),
        std::plus <int> ());

    return point (res.begin (), res.end ());
}

point operator- (const point& lhs, const point& rhs) {
    assert (lhs.size () == rhs.size ());
    
    std::vector <int> res;
    std::transform (
        lhs.begin (), lhs.end (), rhs.begin (),
        std::back_inserter (res),
        std::minus <int> ());

    return point (res.begin (), res.end ());
}

point operator* (const point& lhs, const point& rhs) {
    assert (lhs.size () == rhs.size ());
    
    std::vector <int> res;
    std::transform (
        lhs.begin (), lhs.end (), rhs.begin (),
        std::back_inserter (res),
        std::multiplies <int> ());

    return point (res.begin (), res.end ());
}

point operator/ (const point& lhs, const point& rhs) {
    assert (lhs.size () == rhs.size ());
    
    std::vector <int> res;
    std::transform (
        lhs.begin (), lhs.end (), rhs.begin (),
        std::back_inserter (res),
        std::divides <int> ());

    return point (res.begin (), res.end ());
}

point operator% (const point& lhs, const point& rhs) {
    assert (lhs.size () == rhs.size ());
    
    std::vector <int> res;
    std::transform (
        lhs.begin (), lhs.end (), rhs.begin (),
        std::back_inserter (res),
        std::modulus <int> ());

    return point (res.begin (), res.end ());
}
