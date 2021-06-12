/****************************************************************************

    MIT License

    Copyright 2021 Aria Janke

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.

*****************************************************************************/

#pragma once

#include <common/Vector2.hpp>

#include <tuple>

namespace cul {

template <typename T, typename Func, typename ... Types>
void for_bezier_points
    (const std::tuple<cul::Vector2<T>, Types...> &, T step, Func &&);

template <typename T, typename Func, typename ... Types>
void for_bezier_lines
    (const std::tuple<cul::Vector2<T>, Types...> &, T step, Func &&);

template <typename T, typename ... Types>
cul::Vector2<T> compute_bezier_point
    (T t, const std::tuple<cul::Vector2<T>, Types...> &);

// ----------------------------------------------------------------------------

template <typename T>
class BezierCurveDetails {

    template <typename U, typename Func, typename ... Types>
    friend void for_bezier_points
        (const std::tuple<cul::Vector2<U>, Types...> &, U step, Func &&);

    template <typename U, typename Func, typename ... Types>
    friend void for_bezier_lines
        (const std::tuple<cul::Vector2<U>, Types...> &, U step, Func &&);

    template <typename U, typename ... Types>
    friend cul::Vector2<U> compute_bezier_point
        (U t, const std::tuple<cul::Vector2<U>, Types...> &);

    using VecT = cul::Vector2<T>;

    template <typename ... Types>
    using Tuple = std::tuple<Types...>;

    template <typename Func, typename ... Types>
    static void for_points(const Tuple<Types...> & tuple, T step, Func && f) {
        verify_step(step, "for_points");
        for (T v = 0; v < 1; v += step) {
            f(compute_point_tuple(v, tuple));
        }
        f(compute_point_tuple(1, tuple));
    }

    template <typename Func, typename ... Types>
    static void for_lines(const Tuple<Types...> & tuple, T step, Func && f) {
        verify_step(step, "for_lines");
        for (T v = 0; v < 1; v += step) {
            T next = std::min(T(1), v + step);
            f(compute_point_tuple(v, tuple), compute_point_tuple(next, tuple));
        }
    }

    template <typename ... Types>
    static VecT compute_point_tuple(T t, const Tuple<Types...> & tuple) {
        return compute_point_tuple<sizeof...(Types)>(t, tuple, std::index_sequence_for<Types...>());
    }


    static void verify_step(T t, const char * caller) {
        if (t >= 0 && t <= 1) return;
        throw std::invalid_argument(std::string(caller)
                + ": step must be in [0 1].");
    }

    template <std::size_t k_tuple_size, typename TupleT, std::size_t ... kt_indicies>
    static VecT compute_point_tuple(T t, const TupleT & tuple, std::index_sequence<kt_indicies...>) {
        return compute_point<k_tuple_size>(t, std::get<kt_indicies>(tuple)...);
    }

    template <std::size_t k_tuple_size, typename ... Types>
    static VecT compute_point(T, Types ...)
        { return VecT(); }

    template <std::size_t k_tuple_size, typename ... Types>
    static VecT compute_point(T t, const VecT & r, Types ... args) {
        static_assert(k_tuple_size > sizeof...(Types), "");

        static constexpr const auto k_degree = k_tuple_size - 1;
        static constexpr const auto k_0p_degree = ( k_degree - sizeof...(Types) );
        static constexpr const auto k_1m_degree = k_degree - k_0p_degree;

        static constexpr const T k_scalar
            = ( k_0p_degree == k_degree || k_1m_degree == k_degree )
            ? T(1) : T(k_degree);

        return k_scalar*interpolate<k_1m_degree, k_0p_degree>(t)*r
               + compute_point<k_tuple_size>(t, std::forward<Types>(args)...);
    }

    template <std::size_t k_degree>
    static T interpolate_1m([[maybe_unused]] T t) {
        if constexpr (k_degree == 0)
            { return 1; }
        else
            { return (1 - t)*interpolate_1m<k_degree - 1>(t); }
    }

    template <std::size_t k_degree>
    static T interpolate_0p([[maybe_unused]] T t) {
        if constexpr (k_degree == 0)
            { return 1; }
        else
            { return t*interpolate_0p<k_degree - 1>(t); }
    }

    template <std::size_t k_m1_degree, std::size_t k_0p_degree>
    static T interpolate(T t) {
        return interpolate_1m<k_m1_degree>(t)*interpolate_0p<k_0p_degree>(t);
    }
};

template <typename T, typename Func, typename ... Types>
void for_bezier_points
    (const std::tuple<cul::Vector2<T>, Types...> & tuple, T step, Func && f)
{ return BezierCurveDetails<T>::for_points(tuple, step, std::move(f)); }

template <typename T, typename Func, typename ... Types>
void for_bezier_lines
    (const std::tuple<cul::Vector2<T>, Types...> & tuple, T step, Func && f)
{ return BezierCurveDetails<T>::for_lines(tuple, step, std::move(f)); }

template <typename T, typename ... Types>
cul::Vector2<T> compute_bezier_point
    (T t, const std::tuple<cul::Vector2<T>, Types...> & tuple)
{ return BezierCurveDetails<T>::compute_point_tuple(t, tuple); }

// computes n points on a bezier curve according to a given tuple
template <std::size_t k_count, typename T, typename ... Types>
std::array<cul::Vector2<T>, k_count> make_bezier_array
    (const std::tuple<cul::Vector2<T>, Types...> & tuple)
{
    static constexpr const T k_step = T(1) / T(k_count);
    std::array<cul::Vector2<T>, k_count> arr;
    T t = T(0);
    for (auto & v : arr) {
        v = compute_bezier_point(std::min(T(1), t), tuple);
        t += k_step;
    }
    return arr;
}

} // end of cul namespace
