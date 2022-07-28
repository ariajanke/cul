/****************************************************************************

    MIT License

    Copyright (c) 2022 Aria Janke

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.

*****************************************************************************/

#pragma once

#include <common/Vector2.hpp>

namespace cul {

template <typename T>
using EnableRectangle = std::enable_if_t<std::is_arithmetic_v<T>, Rectangle<T>>;

template <typename T>
std::enable_if_t<std::is_arithmetic_v<T>, Size2<T>>
    make_size(T width_, T height_);

template <typename T>
void set_top_left_of(EnableRectangle<T> & rect, T left, T top);

template <typename T>
void set_size_of(EnableRectangle<T> & rect, T width, T height);

template <typename T>
inline void set_top_left_of(EnableRectangle<T> & rect, const Vector2<T> & r)
    { set_top_left_of(rect, r.x, r.y); }

template <typename T>
void set_size_of(EnableRectangle<T> & rect, const Size2<T> & r)
    { set_size_of(rect, r.width, r.height); }

/** @returns top left location of a rectangle as a vector. */
template <typename T>
Vector2<T> top_left_of(const Rectangle<T> & rect)
    { return Vector2<T>(rect.left, rect.top); }

/** @returns top right location of a rectangle as a vector. */
template <typename T>
Vector2<T> top_right_of(const Rectangle<T> & rect)
    { return Vector2<T>(rect.left + rect.width, rect.top); }

/** @returns bottom left location of a rectangle as a vector. */
template <typename T>
Vector2<T> bottom_left_of(const Rectangle<T> & rect)
    { return Vector2<T>(rect.left, rect.top + rect.height); }

/** @returns bottom right location of a rectangle as a vector. */
template <typename T>
Vector2<T> bottom_right_of(const Rectangle<T> & rect)
    { return Vector2<T>(rect.left + rect.width, rect.top + rect.height); }

template <typename T>
EnableArithmetic<T>
    right_of(const Rectangle<T> & rect) { return rect.left + rect.width; }

template <typename T>
EnableArithmetic<T>
    bottom_of(const Rectangle<T> & rect) { return rect.top + rect.height; }

template <typename T>
std::enable_if_t<std::is_arithmetic_v<T>, Size2<T>>
    size_of(const Rectangle<T> &);

template <typename T>
std::enable_if_t<std::is_arithmetic_v<T>, Vector2<T>>
    center_of(const Rectangle<T> &);

template <typename T>
EnableRectangle<T> find_rectangle_intersection
    (const Rectangle<T> &, const Rectangle<T> &);

template <typename T>
std::enable_if_t<std::is_arithmetic_v<T>, bool>
    overlaps(const Rectangle<T> &, const Rectangle<T> &);

template <typename T>
EnableArithmetic<T> area_of(const Rectangle<T> & a)
    { return a.width*a.height; }

template <typename T>
EnableRectangle<T> compose(const Vector2<T> & top_left, const Size2<T> &);

template <typename T>
std::enable_if_t<std::is_arithmetic_v<T>, Tuple<Vector2<T>, Size2<T>>>
    decompose(const Rectangle<T> & rect)
{ return std::make_tuple(top_left_of(rect), size_of(rect)); }

// ----------------------- Implementation Details -----------------------------

#ifndef DOXYGEN_SHOULD_SKIP_THIS

template <typename T>
std::enable_if_t<std::is_arithmetic_v<T>, Size2<T>>
    make_size(T width_, T height_)
{
    Size2<T> rv;
    rv.width  = width_ ;
    rv.height = height_;
    return rv;
}

template <typename T>
void set_top_left_of(EnableRectangle<T> & rect, T left, T top) {
    rect.left = left;
    rect.top  = top ;
}

template <typename T>
void set_size_of(EnableRectangle<T> & rect, T width, T height) {
    rect.width  = width ;
    rect.height = height;
}

template <typename T>
std::enable_if_t<std::is_arithmetic_v<T>, Size2<T>>
    size_of(const Rectangle<T> & rect)
{
    Size2<T> size;
    size.width  = rect.width ;
    size.height = rect.height;
    return size;
}

template <typename T>
std::enable_if_t<std::is_arithmetic_v<T>, Vector2<T>>
    center_of(const Rectangle<T> & rect)
{
    return Vector2<T>(rect.left + rect.width  / T(2),
                      rect.top  + rect.height / T(2));
}

template <typename T>
EnableRectangle<T> find_rectangle_intersection
    (const Rectangle<T> & a, const Rectangle<T> & b)
{
    using TVec = Vector2<T>;
    auto high_a = TVec(right_of(a), bottom_of(a));
    auto high_b = TVec(right_of(b), bottom_of(b));

    auto low_rv  = TVec(std::max(a.left, b.left), std::max(a.top, b.top));
    auto high_rv = TVec(std::min(high_a.x, high_b.x), std::min(high_a.y, high_b.y));
    if (low_rv.x >= high_rv.x || low_rv.y >= high_rv.y) {
        return Rectangle<T>();
    } else {
        return Rectangle<T>(low_rv.x, low_rv.y,
                            high_rv.x - low_rv.x, high_rv.y - low_rv.y);
    }
}

template <typename T>
std::enable_if_t<std::is_arithmetic_v<T>, bool>
    overlaps(const Rectangle<T> & a, const Rectangle<T> & b)
{
    return    right_of (a) > b.left && right_of (b) > a.left
           && bottom_of(a) > b.top  && bottom_of(b) > a.top ;
}

template <typename T>
EnableRectangle<T> compose(const Vector2<T> & top_left, const Size2<T> & size) {
    return Rectangle<T>(top_left.x, top_left.y, size.width, size.height);
}

#endif // DOXYGEN_SHOULD_SKIP_THIS

} // end of cul namespace
