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

#include <ariajanke/cul/VectorTraits.hpp>

namespace sf {

template <typename T> class Vector2;

template <typename T> class Vector3;

} // end of sf namespace

namespace cul {

template <typename T>
struct VectorTraits<sf::Vector2<T>> {
    static constexpr const bool k_is_vector_type          = true;
    static constexpr const bool k_should_define_operators = false;

    using ScalarType = T;

    template <int kt_idx, typename = int>
    struct Get final {};

    template <typename U>
    struct Get<0, U> final {
        ScalarType operator () (const sf::Vector2<T> & r) const
            { return r.x; }
    };

    template <typename U>
    struct Get<1, U> final {
        ScalarType operator () (const sf::Vector2<T> & r) const
            { return r.y; }
    };

    struct Make final {
        sf::Vector2<T> operator () (const T & x, const T & y) const
            { return sf::Vector2<T>{x, y}; }
    };

    template <typename U>
    using ChangeScalarType = sf::Vector2<U>;

    static constexpr const int k_dimension_count = 2;
};

template <typename T>
struct VectorTraits<sf::Vector3<T>> {
    static constexpr const bool k_is_vector_type          = true;
    static constexpr const bool k_should_define_operators = false;

    using ScalarType = T;

    template <int kt_idx, typename = int>
    struct Get final {};

    template <typename U>
    struct Get<0, U> final {
        ScalarType operator () (const sf::Vector3<T> & r) const
            { return r.x; }
    };

    template <typename U>
    struct Get<1, U> final {
        ScalarType operator () (const sf::Vector3<T> & r) const
            { return r.y; }
    };

    template <typename U>
    struct Get<2, U> final {
        ScalarType operator () (const sf::Vector3<T> & r) const
            { return r.z; }
    };


    struct Make final {
        sf::Vector3<T> operator () (const T & x, const T & y) const
            { return sf::Vector3<T>{x, y}; }
    };

    template <typename U>
    using ChangeScalarType = sf::Vector3<U>;

    static constexpr const int k_dimension_count = 3;
};

} // end of cul namespace
