/****************************************************************************

    File: Util.hpp
    Author: Andrew Janke
    License: GPLv3

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*****************************************************************************/

#pragma once

#include <iterator>
#include <type_traits>

#include <stdexcept>
#include <iosfwd>

#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Rect.hpp>

#include <common/TypeList.hpp>
#include <common/ConstString.hpp>

namespace util {

// imperfect protection, other includes can still disurpt it
// This problem so far exist on GCC
// Will renaming fix it for all platforms? That is the question which is the
// reason why I just don't rename it and call it done.
#ifdef major
#   undef major
#endif

// <---------------------------- Public Interface ---------------------------->
// <------------------------------- iteration -------------------------------->

template <typename T, typename Func, typename Iter>
void quad_range(Iter beg, Iter end, Func && func);

template <typename T, typename Func, typename Cont>
void quad_range(Cont & container, Func && func);

template <typename T, typename Func>
void quad_range(std::initializer_list<T> && ilist, Func && f);

// <------------------------------ vector math ------------------------------->

float  square_root(float );
double square_root(double);

float  sine(float );
double sine(double);

float  cosine(float );
double cosine(double);

float  arc_cosine(float );
double arc_cosine(double);

template <typename T>
T magnitude(const sf::Vector2<T> & v)
    { return square_root(v.x*v.x + v.y*v.y); }

template <typename T>
    typename std::enable_if<std::is_arithmetic<T>::value, T>::type
    magnitude(T t)
    { return (t < T(0)) ? -t : t; }

template <typename T> T sign(T t) { return (t < T(0)) ? T(-1) : T(1); }

template <typename T>
sf::Vector2<T> normalize(const sf::Vector2<T> & v);

template <typename T>
    typename std::enable_if<std::is_arithmetic<T>::value, T>::type
    normalize(T t);

template <typename T>
    constexpr typename std::enable_if<std::is_arithmetic<T>::value, T>::type
    pi() { return T(3.141592653589793238462643383279); }

template <typename T>
sf::Vector2<T> rectangle_location(const sf::Rect<T> & rect)
    { return sf::Vector2<T>(rect.left, rect.top); }

template <typename T>
T right_of(const sf::Rect<T> & rect) { return rect.left + rect.width; }

template <typename T>
T bottom_of(const sf::Rect<T> & rect) { return rect.top + rect.height; }

template <typename T, typename U>
sf::Vector2<T> cast_vector2_to(const sf::Vector2<U> & v)
    { return sf::Vector2<T>(T(v.x), T(v.y)); }

template <typename T, typename U>
sf::Vector2<T> cast_vector2_to(U x, U y)
    { return sf::Vector2<T>(T(x), T(y)); }

template <typename T>
sf::Vector2<T> center_of(const sf::Rect<T> & rect);

template <typename T>
sf::Rect<T> and_rectangles(const sf::Rect<T> & a, const sf::Rect<T> & b);

template <typename T>
T area_of(const sf::Rect<T> & a) { return a.width*a.height; }

// required to doc, the unit circle (frame of reference) for this function...
template <typename T>
sf::Vector2<T> rotate_vector(sf::Vector2<T> r, T rot);

template <typename T>
T dot(const sf::Vector2<T> & v, const sf::Vector2<T> & u)
    { return v.x*u.x + v.y*u.y; }

template <typename T>
T angle_between(const sf::Vector2<T> & v, const sf::Vector2<T> & u);

template <typename T>
sf::Vector2<T> project_unto(const sf::Vector2<T> & a, const sf::Vector2<T> & b);

template <typename T>
sf::Vector2<T> major(const sf::Vector2<T> & v);

/** @note Unsafe float equals, use for sentinel values only!
 *
 */
template <typename T>
    constexpr typename std::enable_if<std::is_floating_point<T>::value, bool>::type
    float_equals(T a, T b)
{ return !(a < b) && !(b < a); }

template <typename T>
    typename std::enable_if<std::is_floating_point<T>::value, bool>::type
    is_nan(T a)
{ return a != a; }

// <-------------------------------- debugging ------------------------------->

void message_assert(const char * msg, bool cond);

} // end of util namespace

class Printable {
public:
    template <typename T>
    explicit Printable(const T * ptr):
        m_value(reinterpret_cast<const void *>(ptr))
    {}
    void print(std::ostream & out) const { out << m_value; }
private:
    const void * m_value;
};

inline std::ostream & operator << (std::ostream & out, const Printable & p)
    { p.print(out); return out; }

using RectD   = sf::Rect<double>;
using VectorD = sf::Vector2<double>;
using VectorI = sf::Vector2i;

// ----------------------------------------------------------------------------
// ----------------------- Implementation Details -----------------------------

namespace util {

template <typename T, typename Func, typename Iter>
void quad_range(Iter beg, Iter end, Func && func) {
    for (Iter itr = beg; itr != end; ++itr) {
    for (Iter jtr = beg; jtr != itr; ++jtr) {
        func(*itr, *jtr);
    }}
}

template <typename T, typename Func, typename Cont>
void quad_range(Cont & container, Func && func) {
    quad_range<T>(std::begin(container), std::end(container), std::move(func));
}

template <typename T, typename Func>
void quad_range(std::initializer_list<T> && ilist, Func && f) {
    quad_range<T>(ilist.begin(), ilist.end(), std::move(f));
}

// ----------------------------------------------------------------------------

template <typename T>
sf::Vector2<T> normalize(const sf::Vector2<T> & v) {
    if (v == sf::Vector2<T>())
        throw std::invalid_argument("Attempting to normalize the zero vector");
    return v*(T(1) / magnitude(v));
}

template <typename T>
    typename std::enable_if<std::is_arithmetic<T>::value, T>::type
    normalize(T t)
{
    if (t == T(0))
        throw std::invalid_argument("Attempting to normalize the zero vector.");
    return (t < T(0)) ? -T(1) : T(1);
}

// ----------------------------------------------------------------------------

template <typename T>
sf::Vector2<T> center_of(const sf::Rect<T> & rect) {
    return sf::Vector2<T>(rect.left - rect.width/T(2),
                          rect.top  - rect.height/T(2));
}

template <typename T>
sf::Rect<T> and_rectangles(const sf::Rect<T> & a, const sf::Rect<T> & b) {
    using TVec = sf::Vector2<T>;
    auto high_a = TVec(right_of(a), bottom_of(a));
    auto high_b = TVec(right_of(b), bottom_of(b));

    auto low_rv  = TVec(std::max(a.left, b.left), std::max(a.top, b.top));
    auto high_rv = TVec(std::min(high_a.x, high_b.x), std::min(high_a.y, high_b.y));
    if (low_rv.x >= high_rv.x or low_rv.y >= high_rv.y) {
        return sf::Rect<T>();
    } else {
        return sf::Rect<T>(low_rv.x, low_rv.y,
                           high_rv.x - low_rv.x, high_rv.y - low_rv.y);
    }
}

// required to doc, the unit circle (frame of reference) for this function...
template <typename T>
sf::Vector2<T> rotate_vector(sf::Vector2<T> r, T rot) {
    // [r.x] * [ cos(rot) sin(rot)]
    // [r.y]   [-sin(rot) cos(rot)]
    return sf::Vector2<T>
        ( r.x*cosine(rot) - r.y*sine  (rot),
          r.x*sine  (rot) + r.y*cosine(rot));
}

template <typename T>
T angle_between(const sf::Vector2<T> & v, const sf::Vector2<T> & u) {
    T frac = dot(v, u) / (magnitude(u)*magnitude(v));
    if (frac > T(1))
        frac = T(1);
    else if (frac < T(-1))
        frac = T(-1);
    return arc_cosine(frac);
}

template <typename T>
sf::Vector2<T> project_unto(const sf::Vector2<T> & a, const sf::Vector2<T> & b) {
    if (util::magnitude(a) < T(0.00005))
        throw std::invalid_argument("Cannot project unto the zero vector.");
    return (util::dot(a, b)/(a.x*a.x + a.y*a.y))*a;
}

template <typename T>
sf::Vector2<T> major(const sf::Vector2<T> & v) {
    if (magnitude(v.x) < magnitude(v.y))
        return sf::Vector2<T>(T(0), v.y);
    else
        return sf::Vector2<T>(v.x, T(0));
}

} // end of util namespace
