/****************************************************************************

    MIT License

    Copyright 2022 Aria Janke

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

#include <common/Util.hpp>
#if 0
#include <common/Vector2.hpp>
#endif
#ifdef MACRO_NEW_20220728_VECTORS
#   include <common/VectorTraits.hpp>
#   include <common/VectorUtils.hpp>
#   include <common/BezierCurvesDetails.hpp>
#else
#   include <common/Vector2Util.hpp>
#endif

#include <tuple>

namespace cul {

#ifdef MACRO_NEW_20220728_VECTORS

/** @addtogroup bezierutils
 *  @{
 */

/** Computes a single point along a Bezier curve, whose control points are
 *  defined by a given tuple.
 *  @tparam T any arithmetic type
 *  @tparam Types every subsequent type must also be arithmetics
 *  @param t any real number between 0 and 1, where 0 represents being at the
 *         first control point, and 1 being at the last control point
 */
template <typename T, typename ... Types>
constexpr EnableIf<detail::k_are_arithmetic<T, Types...>, T>
    find_bezier_point
    (T t, const Tuple<T, Types...> &);

/** Computes a single point along a Bezier curve, whose control points are
 *  defined by a given tuple.
 *  @tparam Vec any vector type
 *  @tparam Types every subsequent type must also be (the same) vector type
 *  @param t any real number between 0 and 1, where 0 represents being at the
 *         first control point, and 1 being at the last control point
 */
template <typename Vec, typename ... Types>
constexpr EnableIf<detail::k_are_vector_types<Vec, Types...>, Vec>
    find_bezier_point
    (ScalarTypeOf<Vec> t, const Tuple<Vec, Types...> &);

/** Loops for some number of points along a Bezier curve whose control points
 *  are defined by a given tuple.
 *  @tparam Vec any vector type
 *  @tparam Func a function called for each point along the Bezier curve, which
 *          takes the following form: void(const Vec &). It is called
 *          "number_of_points" number of times.
 *  @tparam Types remaining types of the control point tuple, must all be
 *          the same vector type.
 *  @param number_of_points the number of points to generate
 */
template <typename Vec, typename Func, typename ... Types>
[[deprecated]] void for_bezier_points
    (const EnableIf<k_is_vector_type<Vec>, Tuple<Vec, Types...>> & tuple,
     int number_of_points, Func &&);

/** Loops for some number of lines along a Bezier curve whose control points
 *  are defined by a given tuple.
 *  @tparam Vec any vector type
 *  @tparam Func a function called for each (straight) line along the Bezier
 *          curve, which takes the following form:
 *          void(const Vec &, const Vec &). It is called
 *          "line_count" number of times.
 *  @tparam Types remaining types of the control point tuple, must all be
 *          the same vector type.
 *  @param line_count the number of lines to generate
 */
template <typename Vec, typename Func, typename ... Types>
[[deprecated]] void for_bezier_lines
    (const EnableIf<detail::k_are_vector_types<Vec, Types...>, Tuple<Vec, Types...>> &,
     int line_count, Func &&);

#if 0 // sorry, had to remove it so soon
template <typename Func, typename Vec, typename ... Types>
[[deprecated]] void for_bezier_triangles
    (const EnableIf<k_are_vector_types<Vec, Types...>, Tuple<Vec, Types...>> & tuple_a,
     const EnableIf<k_are_vector_types<Vec, Types...>, Tuple<Vec, Types...>> & tuple_b,
     ScalarTypeOf<Vec> area, ScalarTypeOf<Vec> error, Func && f);
#endif

/** @returns n points on a bezier curve according to a given tuple */
template <std::size_t k_count, typename Vec, typename ... Types>
[[deprecated]]
    EnableIf<detail::k_are_vector_types<Vec, Types...>, std::array<Vec, k_count>>
    make_bezier_array
    (const Tuple<Vec, Types...> & tuple);

/** @returns a view, as a means to iterate each straight line, approximating a
 *           bezier curve
 *  @tparam Vec any vector type
 *  @tparam Types every subsequent type must also be (the same) vector type
 *  @param tuple tuple used to define the control points of the curve
 *  @param number_of_points the number of points that make up these lines
 */
template <typename Vec, typename ... Types>
constexpr EnableIf<detail::k_are_vector_types<Vec, Types...>,
    View<detail::BezierLineIterator<Vec, Types...>, detail::BezierEndLineIterator>>
    make_bezier_line_view
    (const Tuple<Vec, Types...> & tuple, int number_of_points);

/** @returns a view, as a means to iterate each point along the bezier curve
 *  @tparam Vec any vector type
 *  @tparam Types every subsequent type must also be (the same) vector type
 *  @param tuple tuple used to define the control points of the curve
 *  @param number_of_points is the number of desired points to iterate
 */
template <typename Vec, typename ... Types>
constexpr EnableIf<detail::k_are_vector_types<Vec, Types...>,
    View<detail::BezierIterator<Vec, Types...>, detail::BezierEndIterator>>
    make_bezier_point_view
    (const Tuple<Vec, Types...> & tuple, int number_of_points);

/** @returns a view, as a means to iterate a set of triangles that fill the
 *           area between two bezier curves
 *  @tparam Vec any vector type
 *  @tparam Types every subsequent type must also be (the same) vector type
 *  @param lhs defines controls points for one bezier curve
 *  @param rhs defines controls points for the other bezier curve
 *  @param number_of_points_per_side
 */
template <typename Vec, typename ... Types>
constexpr EnableIf<detail::k_are_vector_types<Vec, Types...>,
    View<detail::BezierTriangleIterator<Vec, Types...>, detail::BezierTriangleEndIterator>>
    make_bezier_triangle_view
    (const Tuple<Vec, Types...> & lhs, const Tuple<Vec, Types...> & rhs,
     int number_of_points_per_side);

/** @} */

// ----------------------------------------------------------------------------

#ifndef DOXYGEN_SHOULD_SKIP_THIS

template <typename T>
constexpr EnableIf<!k_is_vector_type<T> &&
    std::is_arithmetic_v<T>, T>
    make_zero_vector() { return 0; }

namespace detail {

template <int kt_index, typename Vec, typename ... Types>
constexpr EnableIf<
    k_are_vector_types<Vec, Types...>,
    typename ScalarsTuple<Tuple<Vec, Types...>>::Type>
    make_distances_tuple_
//  I need a few more things...
//  line between extremes
    (const Tuple<Vec, Types...> & tup,
     const typename ScalarsTuple<Tuple<Vec, Types...>>::Type & res_so_far,
     ScalarTypeOf<Vec> distances_so_far)
{
    using ScTuple_ = typename ScalarsTuple<Tuple<Vec, Types...>>::Type;
    using std::get;

    auto sub = [](const Vec & a, const Vec & b)
        { return VecOpHelpers<Vec>::template sub<0>(a, b); };

    ScTuple_ source_tup = res_so_far;
    get<kt_index>(source_tup) = distances_so_far;

    if constexpr (kt_index == sizeof...(Types)) {
        return source_tup;
    } else {
        auto diff = sub(get<sizeof...(Types)>(tup), get<0>(tup));
        ScalarTypeOf<Vec> dist = sum_of_squares(sub(
            get<kt_index + 1>(tup),
            project_onto( get<kt_index + 1>(tup), diff )
        ));
        return make_distances_tuple_<kt_index + 1>
            (tup, source_tup, distances_so_far + dist);
    }
}

template <typename Vec, typename ... Types>
constexpr EnableIf<
    k_are_vector_types<Vec, Types...>,
    typename ScalarsTuple<Tuple<Vec, Types...>>::Type>
    make_distances_tuple(const Tuple<Vec, Types...> & tup)
{
    return make_distances_tuple_<0>(tup, typename ScalarsTuple<Tuple<Vec, Types...>>::Type{}, 0);
}

template <typename ScalarTypeT>
struct ScalarTraits final {

    static constexpr const bool k_is_vector_type = true;

    static constexpr const bool k_should_define_operators = false;

    using ScalarType = ScalarTypeT;

    template <int kt_idx, typename=int>
    struct Get final {};

    template <typename U>
    struct Get<0, U> final {
        constexpr ScalarType operator () (const ScalarTypeT & r) const
            { return r; }
    };

    struct Make final {
        constexpr ScalarTypeT operator () (const ScalarTypeT & x) const
            { return x; }
    };

    template <typename U>
    using ChangeScalarType = U;

    static constexpr const int k_dimension_count = 1;
};

template <typename T>
constexpr EnableIf<std::is_arithmetic_v<T>, T>
    plus(const T & lhs, const T & rhs)
{ return lhs + rhs; }

template <typename T>
constexpr EnableIf<k_is_vector_type<T> &&
                   !std::is_arithmetic_v<T>, T>
    plus(const T & lhs, const T & rhs)
{ return VecOpHelpers<T>::template plus<0>(lhs, rhs); }

template <typename T>
constexpr EnableIf<std::is_arithmetic_v<T>, T>
    mul(const T & lhs, const T & rhs)
{ return lhs*rhs; }

template <typename T>
constexpr EnableIf<k_is_vector_type<T> &&
                   !std::is_arithmetic_v<T>, T>
    mul(const T & lhs, const ScalarTypeOf<T> & rhs)
{ return VecOpHelpers<T>::template mul<0>(lhs, rhs); }

template <typename VecU, typename ... Types>
constexpr EnableIf<
    k_are_vector_types<VecU, Types...>,
    VecU>
find_bezier_point_
    (ScalarTypeOf<VecU> t, const std::tuple<VecU, Types...> &);

template <typename T, typename ... Types>
constexpr EnableIf<k_are_arithmetic<T, Types...>, T>
    find_bezier_point_
    (T t, const Tuple<T, Types...> & tup);

template <typename VecT, typename Traits = VectorTraits<VecT>>
class BezierCurveDetails {
    using T = typename Traits::ScalarType;

    template <typename VecU, typename ... Types>
    friend constexpr EnableIf<
        k_are_vector_types<VecU, Types...>,
        VecU>
    find_bezier_point_
        (ScalarTypeOf<VecU> t, const std::tuple<VecU, Types...> &);

    template <typename T, typename ... Types>
    friend constexpr EnableIf<k_are_arithmetic<T, Types...>, T>
        find_bezier_point_
        (T t, const Tuple<T, Types...> & tup);

    template <typename ... Types>
    constexpr static VecT compute_point_tuple(T t, const Tuple<Types...> & tuple) {
        return compute_point_tuple<sizeof...(Types)>(t, tuple, std::index_sequence_for<Types...>{});
    }

    template <std::size_t k_tuple_size, typename TupleT, std::size_t ... kt_indicies>
    constexpr static VecT compute_point_tuple(T t, const TupleT & tuple, std::index_sequence<kt_indicies...>) {
        return compute_point<k_tuple_size>(t, std::get<kt_indicies>(tuple)...);
    }

    template <std::size_t k_tuple_size, typename ... Types>
    constexpr static VecT compute_point(T, Types ...)
        { return make_zero_vector<VecT>(); }

    template <std::size_t k_tuple_size, typename ... Types>
    constexpr static VecT compute_point(T t, const VecT & r, Types ... args) {
        static_assert(k_tuple_size > sizeof...(Types), "");

        constexpr const auto k_degree = k_tuple_size - 1;
        constexpr const auto k_0p_degree = k_degree - sizeof...(Types);
        constexpr const auto k_1m_degree = k_degree - k_0p_degree;

        constexpr const T k_scalar
            = ( k_0p_degree == k_degree || k_1m_degree == k_degree )
            ? T(1) : T(k_degree);

        return plus(mul(
                r, k_scalar*interpolate<k_1m_degree, k_0p_degree>(t)),
            compute_point<k_tuple_size>(t, std::forward<Types>(args)...));
    }

    template <std::size_t k_degree>
    constexpr static T interpolate_1m([[maybe_unused]] T t) {
        if constexpr (k_degree == 0)
            { return 1; }
        else
            { return (1 - t)*interpolate_1m<k_degree - 1>(t); }
    }

    template <std::size_t k_degree>
    constexpr static T interpolate_0p([[maybe_unused]] T t) {
        if constexpr (k_degree == 0)
            { return 1; }
        else
            { return t*interpolate_0p<k_degree - 1>(t); }
    }

    template <std::size_t k_m1_degree, std::size_t k_0p_degree>
    constexpr static T interpolate(T t)
        { return interpolate_1m<k_m1_degree>(t)*interpolate_0p<k_0p_degree>(t); }
};

template <typename Vec>
constexpr EnableIf<
    VectorTraits<Vec>::k_dimension_count == 3,
    ScalarTypeOf<Vec>> area_of_triangle_sqrd
    (const Vec & a, const Vec & b, const Vec & c)
{
    auto sub = [](const Vec & a, const Vec & b)
        { return VecOpHelpers<Vec>::template sub<0>(a, b); };
    auto cprod = cross(sub(a, b), sub(c, b));
    return (sum_of_squares(cprod) / 4);
}

template <typename Vec>
constexpr EnableIf<
    VectorTraits<Vec>::k_dimension_count == 2,
    ScalarTypeOf<Vec>> area_of_triangle_sqrd
    (const Vec & a, const Vec & b, const Vec & c)
{
    auto sub = [](const Vec & a, const Vec & b)
        { return VecOpHelpers<Vec>::template sub<0>(a, b); };
    auto cprod = cross(sub(a, b), sub(c, b));
    return (cprod*cprod / 4);
}

template <typename ... Types>
auto get_last(const Tuple<Types...> & tup)
{ return std::get<sizeof...(Types) - 1>(tup); }

template <typename Vec, typename ... TupleTypes>
class BezierIterator final {
    struct Dummy final {};
public:
    using EnableVecTuple = std::conditional_t<
        k_are_vector_types<Vec, TupleTypes...>,
        Tuple<Vec, TupleTypes...>, Dummy>;

    constexpr BezierIterator
        (const EnableVecTuple & tup,
         const typename ScalarsTuple<EnableVecTuple>::Type & scalar_tuple,
         const ScalarTypeOf<Vec> & step):
        m_pos(0),
        m_step(step),
        m_tuple(tup),
        m_scalar_tuple(scalar_tuple)
    {}

    constexpr BezierIterator & operator ++ () {
        m_pos = next_position();
        return *this;
    }
#   if 0
    constexpr bool operator == (const BezierIterator & rhs) const
        { return is_equal(rhs); }

    constexpr bool operator != (const BezierIterator & rhs) const
        { return !is_equal(rhs); }
#   endif
    constexpr bool operator != (const BezierEndIterator &) const
        { return !is_end(); }

    constexpr bool operator == (const BezierEndIterator &) const
        { return is_end(); }

    constexpr Vec operator * () const {
        auto last = get_last(m_scalar_tuple);
        auto spt = find_bezier_point(m_pos, m_scalar_tuple);
        return find_bezier_point(spt / last, m_tuple);
    }

    constexpr ScalarTypeOf<Vec> next_position() const {
        if (m_pos < 1 && magnitude(m_pos + m_step - 1) <= m_step / 2) {
            return 1; // have to make sure we hit one
        }
        return m_pos + m_step;
    }

private:
    constexpr bool is_equal(const BezierIterator & rhs) const {
        // do_something();
        return m_pos > 1 && rhs.m_pos > 1;
    }

    constexpr bool is_end() const { return m_pos > 1; }

    struct End final {};
    constexpr BezierIterator(End): m_pos(2) {}

    ScalarTypeOf<Vec> m_pos, m_step;
    EnableVecTuple m_tuple;
    typename ScalarsTuple<EnableVecTuple>::Type m_scalar_tuple;
};

template <typename Vec, typename ... TupleTypes>
class BezierLineIterator final {
public:
    explicit BezierLineIterator(const BezierIterator<Vec, TupleTypes...> & beg):
        m_itr(beg) {}

    constexpr Tuple<Vec, Vec> operator * () const {
        auto t = m_itr;
        return std::make_tuple(*m_itr, *(++t));
    }
#   if 0
    constexpr bool operator == (const BezierLineIterator & rhs) const
        { return is_equal(rhs); }

    constexpr bool operator != (const BezierLineIterator & rhs) const
        { return !is_equal(rhs); }
#   endif
    constexpr bool operator == (const BezierEndLineIterator &) const
        { return is_end(); }

    constexpr bool operator != (const BezierEndLineIterator &) const
        { return !is_end(); }

    constexpr BezierLineIterator & operator ++ () {
        ++m_itr;
        return *this;
    }
private:
    constexpr bool is_equal(const BezierLineIterator & rhs) const
        { return m_itr.next_position() > 1 && rhs.m_itr.next_position() > 1; }

    constexpr bool is_end() const
        { return m_itr.next_position() > 1; }

    BezierIterator<Vec, TupleTypes...> m_itr;
};

template <typename Vec, typename ... TupleTypes>
class BezierTriangleIterator final {
public:
    using PtIterator = BezierIterator<Vec, TupleTypes...>;

    constexpr BezierTriangleIterator
        (PtIterator && ws_, PtIterator && os_):
        m_ws(std::move(ws_)),
        m_os(std::move(os_))
    {
        // tip skip
        if (magnitude(VecOpHelpers<Vec>::template sub<0>( *m_ws, *m_os )) < 0.005) {
            ++m_os;
        }
    }

    constexpr BezierTriangleIterator & operator ++ () {
        std::swap(m_os, m_ws);
        ++m_os;
        return *this;
    }

    constexpr Tuple<Vec, Vec, Vec> operator * () const {
        auto ws_cpy = m_ws;
        ++ws_cpy;
        auto rv = std::make_tuple( *m_ws, *m_os, *ws_cpy );
        return rv;
    }

    constexpr bool operator != (const BezierTriangleEndIterator &) const
        { return !is_end(); }

    constexpr bool operator == (const BezierTriangleEndIterator &) const
        { return is_end(); }

private:
    constexpr bool is_end() const {
        auto ws_cpy = m_ws;
        ++ws_cpy;
        return ws_cpy == BezierEndIterator{};
    }

    PtIterator m_ws, m_os;
};

#if 0
template <typename Vec, typename ... TupleTypes>
class BezierTriangleIterator final {
public:
    using TupleT = Tuple<Vec, TupleTypes...>;
    using T = ScalarTypeOf<Vec>;

    constexpr BezierTriangleIterator(
        const TupleT & tuple_a, const TupleT & tuple_b,
            T area_, T error_):
        work_side (&tuple_a),
        other_side(&tuple_b),
        error(error_),
        area(area_)
    {
        // pinch check and skip
        auto first_ws_pt = find_bezier_point(T(0), *work_side );
        auto first_os_pt = find_bezier_point(T(0), *other_side);
        if (are_within(first_ws_pt, first_os_pt, error)) {
            auto [os_pos_, ws_pos_] = progress_from_pinch(
                *work_side, *other_side, area, error); {}
            // need to put these values somewhere...
            // while tip point is pulled from work_side, other_side maybe used
            // as this is a "pinch" point
            os_pos = 0;
            ws_pos = ws_pos_;
            next_step = WbcStep{false, os_pos_};
        } else {
            // still need to get ready for the first dereference
            next_step = find_next_position(
                        *work_side, first_os_pt, first_ws_pt, 0, area, error);
        }
        // "dereference" should follow
    }

    // all triangles should have "non-adruptly" different normals
    // low_ws, low_os, next_ws
    constexpr Tuple<Vec, Vec, Vec> operator * () const {
        return std::make_tuple(
            find_bezier_point(ws_pos, *work_side),
            find_bezier_point(os_pos, *other_side),
            find_bezier_point(next_step.next_pos, *work_side));
    }

    constexpr BezierTriangleIterator & operator ++ () {
        if (next_step.on_last) {
            // end it here?
            ws_pos = os_pos = 1.1;
            return *this;
        }
        ws_pos = next_step.next_pos;
        std::swap(ws_pos, os_pos);
        std::swap(work_side, other_side);

        auto following = find_next_position(
            *work_side, find_bezier_point(os_pos, *other_side),
            find_bezier_point(ws_pos, *work_side), ws_pos, area, error);
        bool should_mark_last =following.finishes && next_step.finishes;
        next_step = following;
        next_step.on_last = should_mark_last;

        return *this;
    }
#   if 0
    constexpr bool operator == (const BezierTriangleIterator & rhs) const
        { return is_equal(rhs); }

    constexpr bool operator != (const BezierTriangleIterator & rhs) const
        { return !is_equal(rhs); }
#   endif
    constexpr bool operator == (const BezierTriangleEndIterator &) const
        { return is_end(); }

    constexpr bool operator != (const BezierTriangleEndIterator &) const
        { return !is_end(); }

private:
#   if 0
    constexpr bool is_equal(const BezierTriangleIterator & rhs) const
        { return (ws_pos > 1 && rhs.ws_pos > 1) || (os_pos > 1 && rhs.os_pos > 1); }
#   endif
    constexpr bool is_end() const
        { return ws_pos > 1 || os_pos > 1; }

    constexpr static Vec sub(const Vec & a, const Vec & b)
        { return VecOpHelpers<Vec>::template sub<0>(a, b); }

    template <typename ... Types>
    constexpr static Tuple<T, T> progress_from_pinch
        (const TupleT & work_side, const TupleT & other_side,
         T area, T error)
    {
        const auto & any_side = work_side;
        // I'm going to need a combination things for my
        // resolution criteria
        auto tip_pt = find_bezier_point(T(0), any_side);
        auto os_pt = find_bezier_point(T(1), other_side);
        auto ws_pt = find_bezier_point(T(1), work_side );

        if (detail::area_of_triangle_sqrd(tip_pt, os_pt, ws_pt) < area) {
            return std::make_tuple(T(1), T(1));
        }
        T low = 0, high = 1;
        while (true) {
            // isn't there a more direct way to find this?
            // for instance, how fast is the triangle area's growth?
            // linear perhaps?
            auto mid = (low + high) / T(2);
            auto os_pt = find_bezier_point(mid, other_side);
            auto ws_pt = find_bezier_point(mid, work_side );
            auto tri_area_sqrd = detail::area_of_triangle_sqrd(tip_pt, os_pt, ws_pt);
            if (magnitude(tri_area_sqrd - area) < error*error)
                { return std::make_tuple(mid, mid); }
            *((tri_area_sqrd > area) ? &high : &low) = mid;
        }
    }
#   if 0
    // different for 2D, this is the 3D version
    // will possibly have to move out of class
    static constexpr T area_of_triangle_sqrd
        (const Vec & a, const Vec & b, const Vec & c)
    {
        auto cprod = cross(sub(a, b), sub(c, b));
        return (sum_of_squares(cprod) / 4);
    }
#   endif
    struct WbcStep final { // walk bezier curve step
        constexpr WbcStep() {}
        constexpr WbcStep(bool fin_, T next_pos_):
            finishes(fin_), next_pos(next_pos_) {}
        bool finishes = false;
        bool on_last = false;
        T next_pos = 0;
    };

    constexpr static WbcStep find_next_position
        (const TupleT & work_side,
         const Vec & pt_low_os, const Vec & pt_low_ws,
         T ws_pos, T area, T error)
    {
        // so now on the work side, we find another point such that we're close
        // to the given area (which is the "resolution" argument)
        auto pt_con_ws = find_bezier_point(T(1), work_side);
        if (detail::area_of_triangle_sqrd(pt_low_os, pt_low_ws, pt_con_ws) < area*area) {
            return WbcStep{true, T(1)};
        }
        T low = ws_pos, high = 1;
        while (low != high) {
            pt_con_ws = find_bezier_point((low + high) / T(2), work_side);
            auto con_tri_area = detail::area_of_triangle_sqrd(pt_low_os, pt_low_ws, pt_con_ws);
            if (magnitude(con_tri_area - area*area) < error*error) {
                return WbcStep{false, (low + high) / T(2)};
            }
            auto & target = *((con_tri_area > area*area) ? &high : &low);
            if (target == (low + high) / 2) break;
            target = (low + high) / T(2);
            //*((con_tri_area > area*area) ? &high : &low) = (low + high) / T(2);
        }
        return WbcStep{false, low};
    }

    constexpr static WbcStep find_next_position_new
        (const TupleT & work_side,
         const Vec & pt_low_os, const Vec & pt_low_ws,
         T ws_pos, T area, T error)
    {
        // so now on the work side, we find another point such that we're close
        // to the given area (which is the "resolution" argument)
        auto pt_con_ws = find_bezier_point(T(1), work_side);
        if (detail::area_of_triangle_sqrd(pt_low_os, pt_low_ws, pt_con_ws) < area*area) {
            return WbcStep{true, T(1)};
        }
#       if 0
        // need extreme point for work_side
        // this is the source of most of my problem using this method
        auto ex_work_side_pt = std::get<sizeof...(TupleTypes)>(work_side);
        // note: cosine of the angle

        // if I want this to be constexpr... I'm going to have to break the "ct_sqrt"
        auto sine_alpha_sqrd = [] (const Vec & a, const Vec & b) {
            auto adj_sqrd   = dot(a, b)*dot(a, b);
            auto hypot_sqrd = sum_of_squares(a)*sum_of_squares(b);
            auto oppo_sqrd  = hypot_sqrd - adj_sqrd;
            return oppo_sqrd / hypot_sqrd;
        } ( sub(ex_work_side_pt, pt_low_ws), sub(pt_low_os, pt_low_ws) );
        auto side_ab_sqrd = sum_of_squares( sub( pt_low_os, pt_low_ws ) );

        auto side_ac_sqrd = ( 4*area*area ) / ( sine_alpha_sqrd*side_ab_sqrd );
        // I'd normalize, but not possible without sqrt
        auto toward_ws_ex = sub(ex_work_side_pt, pt_low_ws);
        // So... I just assumed linear... omg
        // Yes... we *are* generating a triangle
#       else
        T low = ws_pos, high = 1;
        while (low != high) {
            pt_con_ws = find_bezier_point((low + high) / T(2), work_side);
            auto con_tri_area = detail::area_of_triangle_sqrd(pt_low_os, pt_low_ws, pt_con_ws);
            if (magnitude(con_tri_area - area*area) < error*error) {
                return WbcStep{false, (low + high) / T(2)};
            }
            *((con_tri_area > area*area) ? &high : &low) = (low + high) / T(2);
        }
        return WbcStep{false, (low + high) / T(2)};
#       endif
    }

    const TupleT * work_side  ;
    const TupleT * other_side ;

    T os_pos = 0;
    T ws_pos = 0;
    WbcStep next_step;
    T error = 0;
    T area = 0;
};
#endif

template <typename VecU, typename ... Types>
constexpr EnableIf<
    k_are_vector_types<VecU, Types...>,
    VecU>
find_bezier_point_
    (ScalarTypeOf<VecU> t, const std::tuple<VecU, Types...> & tup)
{ return BezierCurveDetails<VecU>::compute_point_tuple(t, tup); }

template <typename T, typename ... Types>
constexpr EnableIf<k_are_arithmetic<T, Types...>, T>
    find_bezier_point_
    (T t, const Tuple<T, Types...> & tup)
{ return BezierCurveDetails<T, ScalarTraits<T>>::compute_point_tuple(t, tup); }

} // end detail namespace -> into ::cul

// --------------------- Implementations (beyond helpers) ---------------------

template <typename T, typename ... Types>
constexpr EnableIf<detail::k_are_arithmetic<T, Types...>, T>
    find_bezier_point
    (T t, const Tuple<T, Types...> & tup)
{ return detail::find_bezier_point_(t, tup); }

template <typename Vec, typename ... Types>
constexpr EnableIf<detail::k_are_vector_types<Vec, Types...>, Vec>
    find_bezier_point
    (ScalarTypeOf<Vec> t, const Tuple<Vec, Types...> & tup)
{ return detail::find_bezier_point_(t, tup); }

template <typename Vec, typename Func, typename ... Types>
[[deprecated]] void for_bezier_points
    (const EnableIf<k_is_vector_type<Vec>, Tuple<Vec, Types...>> & tuple,
     int number_of_points, Func && f_)
{
    for (auto pt : make_bezier_point_view(tuple, number_of_points)) {
        auto fc = adapt_to_flow_control_signal(f_, pt);
        if (fc == fc_signal::k_break) return;
    }
}

template <typename Vec, typename Func, typename ... Types>
[[deprecated]] void for_bezier_lines
    (const EnableIf<detail::k_are_vector_types<Vec, Types...>, Tuple<Vec, Types...>> & tup,
     int line_count, Func && f_)
{
    for (auto [a, b] : make_bezier_line_view(tup, line_count)) {
        auto fc = adapt_to_flow_control_signal(f_, a, b);
        if (fc == fc_signal::k_break) return;
    }
}

template <int k_count, typename Vec, typename ... Types>
[[deprecated]] constexpr EnableIf<
    detail::k_are_vector_types<Vec, Types...>,
    std::array<Vec, k_count>>
    make_bezier_array(const Tuple<Vec, Types...> & tuple)
{
    // constexpr with imperitive is so cursed
    std::array<Vec, k_count> rv{};
    auto itr = rv.begin();
    for (auto r : make_bezier_point_view(tuple, k_count)) {
        *itr++ = r;
    }
    return rv;
}

template <typename Vec, typename ... Types>
constexpr EnableIf<detail::k_are_vector_types<Vec, Types...>,
    View<detail::BezierLineIterator<Vec, Types...>, detail::BezierEndLineIterator>>
    make_bezier_line_view
    (const Tuple<Vec, Types...> & tuple, int number_of_points)
{
    // number of points must be at least two
    using namespace detail;
    using LineItr = BezierLineIterator<Vec, Types...>;
    auto pt_view = make_bezier_point_view(tuple, number_of_points);
    return View<LineItr, BezierEndLineIterator>{LineItr{pt_view.begin()}, BezierEndLineIterator{}};
}

template <typename Vec, typename ... Types>
constexpr EnableIf<detail::k_are_vector_types<Vec, Types...>,
    View<detail::BezierIterator<Vec, Types...>, detail::BezierEndIterator>>
    make_bezier_point_view
    (const Tuple<Vec, Types...> & tuple, int number_of_points)
{
    // number of points must be at least two
    using namespace detail;
    using BezItr = BezierIterator<Vec, Types...>;
    auto step = 1 / ScalarTypeOf<Vec>(number_of_points - 1);
    auto interpolation_tuple = detail::make_distances_tuple(tuple);
    return View<BezItr, BezierEndIterator>{BezItr{tuple, interpolation_tuple, step}, BezierEndIterator{}};
}

template <typename Vec, typename ... Types>
constexpr EnableIf<detail::k_are_vector_types<Vec, Types...>,
    View<detail::BezierTriangleIterator<Vec, Types...>, detail::BezierTriangleEndIterator>>
    make_bezier_triangle_view
    (const Tuple<Vec, Types...> & lhs, const Tuple<Vec, Types...> & rhs,
     int number_of_points_per_side)
{
    using namespace detail;
    return View<BezierTriangleIterator<Vec, Types...>, BezierTriangleEndIterator>{
        BezierTriangleIterator<Vec, Types...>{
            make_bezier_point_view(lhs, number_of_points_per_side).begin(),
            make_bezier_point_view(rhs, number_of_points_per_side).begin()
        },
        BezierTriangleEndIterator{}
    };
}
#if 0
template <typename Vec, typename ... Types>
constexpr EnableIf<k_are_vector_types<Vec, Types...>,
    View<BezierTriangleIterator<Vec, Types...>, BezierTriangleEndIterator>>
    make_bezier_triangle_view
    (const Tuple<Vec, Types...> & lhs, const Tuple<Vec, Types...> & rhs,
     ScalarTypeOf<Vec> area, ScalarTypeOf<Vec> error)
{
#   if 0
    using RtView = View<BezierTriangleIterator<Vec, Types...>, BezierTriangleEndIterator>;
    return RtView{
        BezierTriangleIterator<Vec, Types...>{lhs, rhs, area, error},
        BezierTriangleEndIterator{}
    };
#   endif
}
#endif
// ----------------------------------------------------------------------------

#endif // ifndef DOXYGEN_SHOULD_SKIP_THIS

#else

/** @addtogroup bezierutils
 *  @{
 */

/** Loops for some number of points along a Bezier curve whose control points
 *  are defined by a given tuple.
 *  @tparam T Scalar type of the vector
 *  @tparam Func a function called for each point along the Bezier curve, which
 *          takes the following form: void(cul::Vector2<T>). It is called
 *          "number_of_points" number of times.
 *  @tparam Types remaining types of the control point tuple, must all be
 *          the same vector type.
 *  @param number_of_points the number of points to generate
 */
template <typename T, typename Func, typename ... Types>
void for_bezier_points
    (const std::tuple<Vector2<T>, Types...> &, int number_of_points, Func &&);

/** Loops for some number of lines along a Bezier curve whose control points
 *  are defined by a given tuple.
 *  @tparam T Scalar type of the vector
 *  @tparam Func a function called for each (straight) line along the Bezier
 *          curve, which takes the following form:
 *          void(cul::Vector2<T>, cul::Vector2<T>). It is called
 *          "line_count" number of times.
 *  @tparam Types remaining types of the control point tuple, must all be
 *          the same vector type.
 *  @param line_count the number of lines to generate
 */
template <typename T, typename Func, typename ... Types>
void for_bezier_lines
    (const std::tuple<Vector2<T>, Types...> &, int line_count, Func &&);

/** Computes a single point along a Bezier curve, whose control points are
 *  defined by a given tuple.
 *  @tparam T Scalar type of the vector
 *  @param t any real number between 0 and 1, where 0 represents being at the
 *         first control point, and 1 being at the last control point
 */
template <typename T, typename ... Types>
Vector2<T> find_bezier_point
    (T t, const std::tuple<Vector2<T>, Types...> &);

/** @returns n points on a bezier curve according to a given tuple */
template <std::size_t k_count, typename T, typename ... Types>
std::array<cul::Vector2<T>, k_count> make_bezier_array
    (const std::tuple<cul::Vector2<T>, Types...> & tuple);

/** Generates some number of triangles formed between two Bezier curves.
 *
 *  @note This is a fairly intense algorithm.
 *
 *  This uses a "binary search" to find triangles that are approximately the
 *  specified area (but within some given error). This algorithm will attempt
 *  to follow along self-intersecting curves.
 *
 *  @tparam Func a function called for each triangle along the two Bezier
 *          curves. It must take the following form:
 *          void(cul::Vector2<T>, cul::Vector2<T>, cul::Vector2<T>). It is
 *          called approximately n times where n is the total area of this
 *          "strip" divided by the "resolution"/given area of this call.
 *  @param tuple_a one side of the strip of triangles
 *  @param tuple_b another side of the strip of triangles
 *  @param area or "resolution" area, the area of triangle that is desired
 *         along this strip, note that the last triangle that the algorithm
 *         gives, won't neccessarily be approximately this area
 *  @param error the error margin this algorithm should use, larger values will
 *         make for a faster call, but at the expense of "sloppier" triangle
 *         areas
 *  @param f the given callback function which takes three vectors as its
 *         arguments (see Func)
 */
template <typename Func, typename T, typename ... Types>
void for_bezier_triangles
    (const std::tuple<cul::Vector2<T>, Types...> & tuple_a,
     const std::tuple<cul::Vector2<T>, Types...> & tuple_b,
     T area, T error, Func && f);

/** @}*/

// ----------------------------------------------------------------------------

namespace detail {

template <typename T>
class BezierCurveDetails {
    template <typename U, typename Func, typename ... Types>
    friend void cul::for_bezier_points
        (const std::tuple<cul::Vector2<U>, Types...> &, int, Func &&);

    template <typename U, typename Func, typename ... Types>
    friend void cul::for_bezier_lines
        (const std::tuple<cul::Vector2<U>, Types...> &, int, Func &&);

    template <typename U, typename ... Types>
    friend cul::Vector2<U> cul::find_bezier_point
        (U t, const std::tuple<cul::Vector2<U>, Types...> &);

    using VecT = cul::Vector2<T>;

    template <typename ... Types>
    using Tuple = std::tuple<Types...>;

    template <typename Func, typename ... Types>
    static void for_points(const Tuple<Types...> & tuple, int step_count, Func && f) {
        verify_step_count(step_count, "for_points");
        for (int i = 0; i != step_count; ++i) {
            T v = T(i) / T(step_count);
            f(compute_point_tuple(v, tuple));
        }
        f(compute_point_tuple(1, tuple));
    }

    template <typename Func, typename ... Types>
    static void for_lines(const Tuple<Types...> & tuple, int line_count, Func && f) {
        verify_step_count(line_count, "for_lines");
        for (int i = 0; i != line_count; ++i) {
            auto v = T(i) / T(line_count + 1);
            T next = std::min(T(1), T(i + 1) / T(line_count + 1));
            f(compute_point_tuple(v, tuple), compute_point_tuple(next, tuple));
        }
    }

    template <typename ... Types>
    static VecT compute_point_tuple(T t, const Tuple<Types...> & tuple) {
        return compute_point_tuple<sizeof...(Types)>(t, tuple, std::index_sequence_for<Types...>());
    }

    static void verify_step_count(int t, const char * caller) {
        if (t >= 0) return;
        throw std::invalid_argument(std::string(caller)
                + ": step must be in [0 1].");
    }

    template <std::size_t k_tuple_size, typename TupleT, std::size_t ... kt_indicies>
    static VecT compute_point_tuple(T t, const TupleT & tuple, std::index_sequence<kt_indicies...>) {
        return compute_point<k_tuple_size>(t, std::get<kt_indicies>(tuple)...);
    }

    template <std::size_t k_tuple_size, typename ... Types>
    static VecT compute_point(T, Types ...) { return VecT(); }

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
    static T interpolate(T t)
        { return interpolate_1m<k_m1_degree>(t)*interpolate_0p<k_0p_degree>(t); }
};

template <typename T>
class BezierTriangleDetails {
    template <typename Func, typename U, typename ... Types>
    friend void cul::for_bezier_triangles
        (const std::tuple<cul::Vector2<U>, Types...> & tuple_a,
         const std::tuple<cul::Vector2<U>, Types...> & tuple_b,
         U area, U error, Func && f);

    template <typename Func, typename ... Types>
    static void for_bezier_triangles
        (const std::tuple<cul::Vector2<T>, Types...> & tuple_a,
         const std::tuple<cul::Vector2<T>, Types...> & tuple_b,
         T area, T error, Func && f)
    {

        using TupleType = std::tuple<cul::Vector2<T>, Types...>;
        const TupleType * work_side  = &tuple_a;
        const TupleType * other_side = &tuple_b;

        T os_pos = 0;
        T ws_pos = 0;
        // edge case where the two points at the top meet
        if (magnitude(  find_bezier_point(T(0), *work_side )
                      - find_bezier_point(T(0), *other_side)) < error)
        {
            std::tie(os_pos, ws_pos) = progress_from_pinch(
                *work_side, *other_side, area, error);
            auto tip_pt = find_bezier_point(T(0), *work_side);
            auto os_pt = find_bezier_point(os_pos, *other_side);
            auto ws_pt = find_bezier_point(ws_pos, *work_side );
            f(tip_pt, os_pt, ws_pt);
        }

        // main loop
        bool last_iter_finished = false;
        while (true) {
            auto pt_low_os = find_bezier_point(os_pos, *other_side);
            auto pt_low_ws = find_bezier_point(ws_pos, *work_side );
            auto gv = find_next_position(
                *work_side, pt_low_os, pt_low_ws, ws_pos, area, error);
            f(pt_low_ws, pt_low_os, find_bezier_point(gv.next_pos, *work_side));

            if (gv.finishes && last_iter_finished) return;
            last_iter_finished = gv.finishes;
            ws_pos = gv.next_pos;
            std::swap(work_side, other_side);
            std::swap(os_pos   , ws_pos    );
        }
    }

    struct WbcStep { // walk bezier curve step
        bool finishes = false;
        T next_pos;
    };

    template <typename ... Types>
    static std::tuple<T, T> progress_from_pinch
        (const std::tuple<cul::Vector2<T>, Types...> & work_side,
         const std::tuple<cul::Vector2<T>, Types...> & other_side,
         T area, T error)
    {
        const auto & any_side = work_side;
        // I'm going to need a combination things for my
        // resolution criteria
        auto tip_pt = find_bezier_point(T(0), any_side);
        auto os_pt = find_bezier_point(T(1), other_side);
        auto ws_pt = find_bezier_point(T(1), work_side );

        if (area_of_triangle(tip_pt, os_pt, ws_pt) < area) {
            return std::make_tuple(T(1), T(1));
        }
        T low = 0, high = 1;
        while (true) {
            auto mid = (low + high) / T(2);
            auto os_pt = find_bezier_point(mid, other_side);
            auto ws_pt = find_bezier_point(mid, work_side );
            auto tri_con_area = area_of_triangle(tip_pt, os_pt, ws_pt);
            if (magnitude(tri_con_area - area) < error) {
                return std::make_tuple(mid, mid);
            }
            *((tri_con_area > area) ? &high : &low) = mid;
        }
    }

    template <typename ... Types>
    static WbcStep find_next_position
        (const std::tuple<cul::Vector2<T>, Types...> & work_side,
         const cul::Vector2<T> & pt_low_os, const cul::Vector2<T> & pt_low_ws,
         T ws_pos, T area, T error)
    {
        // so now on the work side, we find another point such that we're close
        // to the given area (which is the "resolution" argument)
        WbcStep rv;
        auto pt_con_ws = find_bezier_point(T(1), work_side);
        if (area_of_triangle(pt_low_os, pt_low_ws, pt_con_ws) < area) {
            rv.finishes = true;
            rv.next_pos = T(1);
            return rv;
        }
        T low = ws_pos, high = 1;
        while (true) {
            pt_con_ws = find_bezier_point((low + high) / T(2), work_side);
            auto con_tri_area = area_of_triangle(pt_low_os, pt_low_ws, pt_con_ws);
            if (magnitude(con_tri_area - area) < error) {
                rv.finishes = false;
                rv.next_pos = (low + high) / T(2);
                return rv;
            }
            *((con_tri_area > area) ? &high : &low) = (low + high) / T(2);
        }
    }
};

} // end of detail namespace -> into cul

template <typename T, typename Func, typename ... Types>
void for_bezier_points
    (const std::tuple<Vector2<T>, Types...> & tuple, int step_count, Func && f)
{ return detail::BezierCurveDetails<T>::for_points(tuple, step_count, std::move(f)); }

template <typename T, typename Func, typename ... Types>
void for_bezier_lines
    (const std::tuple<Vector2<T>, Types...> & tuple, int line_count, Func && f)
{ return detail::BezierCurveDetails<T>::for_lines(tuple, line_count, std::move(f)); }

template <typename T, typename ... Types>
cul::Vector2<T> find_bezier_point
    (T t, const std::tuple<Vector2<T>, Types...> & tuple)
{ return detail::BezierCurveDetails<T>::compute_point_tuple(t, tuple); }

template <std::size_t k_count, typename T, typename ... Types>
std::array<cul::Vector2<T>, k_count> make_bezier_array
    (const std::tuple<cul::Vector2<T>, Types...> & tuple)
{
    static constexpr const T k_step = T(1) / T(k_count);
    std::array<cul::Vector2<T>, k_count> arr;
    T t = T(0);
    for (auto & v : arr) {
        v = find_bezier_point(std::min(T(1), t), tuple);
        t += k_step;
    }
    return arr;
}

template <typename Func, typename T, typename ... Types>
void for_bezier_triangles
    (const std::tuple<cul::Vector2<T>, Types...> & tuple_a,
     const std::tuple<cul::Vector2<T>, Types...> & tuple_b,
     T area, T error, Func && f)
{
    detail::BezierTriangleDetails<T>::for_bezier_triangles(
        tuple_a, tuple_b, area, error, std::move(f));
}
#endif

} // end of cul namespace
