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

#include <common/VectorTraits.hpp>

namespace cul {

/** Defines a 3D vector for a given scalar type.
 *
 *  Types defined with this template are useable with many of the utility
 *  functions.
 *
 *  When defined, many vector operators are also (as per its Traits defined
 *  later). They are as follows:
 *  - unary negation
 *  - vector addition (and compound)
 *  - vector subtraction (and compound)
 *  - scalar multiplication (and compound)
 *  - scalar division (and compound)
 *  - equality
 *  - inequality
 */
template <typename T>
struct Vector3 final {
    constexpr Vector3() {}

    constexpr Vector3(T x_, T y_, T z_): x(x_), y(y_), z(z_) {}

    template <typename U>
    constexpr explicit Vector3(U x_, U y_, U z_): x(T(x_)), y(T(y_)), z(T(z_)) {}

    template <typename U>
    constexpr explicit Vector3(const Vector3<U> & r):
        x(T(r.x)), y(T(r.y)), z(T(r.z)) {}

    T x = 0, y = 0, z = 0;
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS

template <typename T>
struct VectorTraits<Vector3<T>> final {
    static constexpr const bool k_is_vector_type = true;

    static constexpr const bool k_should_define_operators = true;

    using ScalarType = T;

    template <int kt_idx, typename=int>
    struct Get final {};

    template <typename U>
    struct Get<0, U> final {
        constexpr ScalarType operator () (const Vector3<T> & r) const
            { return r.x; }
    };

    template <typename U>
    struct Get<1, U> final {
        constexpr ScalarType operator () (const Vector3<T> & r) const
            { return r.y; }
    };

    template <typename U>
    struct Get<2, U> final {
        constexpr ScalarType operator () (const Vector3<T> & r) const
            { return r.z; }
    };

    struct Make final {
        constexpr Vector3<T> operator () (const T & x, const T & y, const T & z) const
            { return Vector3<T>{x, y, z}; }
    };

    template <typename U>
    using ChangeScalarType = Vector3<U>;

    static constexpr const int k_dimension_count = 3;
};

#endif // DOXYGEN_SHOULD_SKIP_THIS

} // end of cul namespace
