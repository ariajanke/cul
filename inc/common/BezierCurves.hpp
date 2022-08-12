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
#include <common/VectorTraits.hpp>
#include <common/VectorUtils.hpp>
#include <common/BezierCurvesDetails.hpp>

#include <tuple>

namespace cul {

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
    View<detail::BezierLineIterator<Vec, Types...>, detail::BezierEndIterator>>
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

/** Type returned be the "make_bezier_strip" free function. Allows the client
 *  to select an appropriate level of detail for their needs.
 *
 *  Always providing a detailed view may make code more verbose and clunkier
 *  to use. A so follows the rationale of this design.
 */
template <typename Vec, typename ... Types>
class BezierStrip final {
public:
    /** @warning this type alias is more implementation detail than interface */
    using PointsView = View<detail::BezierStripTrianglesIterator<Vec, Types...>, detail::BezierEndIterator>;

    /** @warning this type alias is more implementation detail than interface */
    using DetailedView = View<detail::BezierStripDetailedIterator<Vec, Types...>, detail::BezierEndIterator>;

    /** @warning this type alias is more implementation detail than interface */
    using PtIterator = detail::BezierIterator<Vec, Types...>;

    /** @warning iterator type passed here is implementation detail, use
     *           "make_bezier_strip" instead.
     */
    constexpr BezierStrip(PtIterator &&, PtIterator &&);

    /** @returns a view type, whose iterators dereference to a tuple of three
     *           points
     */
    constexpr PointsView points_view() const;

    /** @returns a view type, whote iterators dereference to a
     *           "BezierStripDetails" type
     *  @see BezierStripDetails
     */
    constexpr DetailedView details_view() const;

private:
#   ifndef DOXYGEN_SHOULD_SKIP_THIS
    PtIterator m_lhs, m_rhs;
#   endif
};

/** A utility class which offers greater detailed information on points along a
 *  Bezier strip.
 */
template <typename Vec>
class BezierStripDetails final {
public:
    /** While the client may simply instantiate this class, it's not
     *  recommended. This constructor exists as the easiest implementation.
     */
    constexpr BezierStripDetails(bool on_left_, const Vec & pt_, const ScalarTypeOf<Vec> & pos_):
        m_on_left(on_left_),
        m_pos(pos_),
        m_pt(pt_)
    {}

    /** @returns an interploated vector along the Bezier curve on one side of
     *           the strip
     */
    constexpr Vec point() const { return m_pt; }

    /** @returns the position on the Bezier curve used to compute the vector */
    constexpr ScalarTypeOf<Vec> position() const { return m_pos; }

    /** @returns true if this point is on the curve defined by the left tuple */
    constexpr bool on_left() const { return m_on_left; }

    /** @returns true if this point is on the curve defined by the right tuple
     */
    constexpr bool on_right() const { return !m_on_left; }

private:
#   ifndef DOXYGEN_SHOULD_SKIP_THIS
    bool m_on_left;
    ScalarTypeOf<Vec> m_pos;
    Vec m_pt;
#   endif
};

/** A Bezier strip in this case, is created from two tuples. Which allows the
 *  creation of more interesting shapes.
 *  @returns a special "strip" type, which then maybe used to get a "points
 *           only" view, or a detailed view, see the class definition for more
 *           details
 *  @see BezierStripDetails
 *  @tparam Vec any vector type
 *  @tparam Types every subsequent type must also be (the same) vector type
 *  @param lhs defines controls points for one bezier curve
 *  @param rhs defines controls points for the other bezier curve
 *  @param number_of_points_per_side
 */
template <typename Vec, typename ... Types>
constexpr EnableIf<detail::k_are_vector_types<Vec, Types...>,
    BezierStrip<Vec, Types...>> make_bezier_strip
    (const Tuple<Vec, Types...> & lhs, const Tuple<Vec, Types...> & rhs,
     int number_of_points_per_side);

/** @} */

// ----------------------------------------------------------------------------

#ifndef DOXYGEN_SHOULD_SKIP_THIS

// must not be "detail"
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

    constexpr bool operator != (const BezierEndIterator &) const
        { return !is_end(); }

    constexpr bool operator == (const BezierEndIterator &) const
        { return is_end(); }

    constexpr Vec operator * () const
        { return find_bezier_point(curve_position(), m_tuple); }

    constexpr ScalarTypeOf<Vec> curve_position() const {
        return   find_bezier_point(m_pos, m_scalar_tuple)
               / get_last(m_scalar_tuple);
    }

    constexpr bool next_is_end() const
        { return next_position() > 1; }

private:

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

    constexpr bool operator == (const BezierEndIterator &) const
        { return is_end(); }

    constexpr bool operator != (const BezierEndIterator &) const
        { return !is_end(); }

    constexpr BezierLineIterator & operator ++ () {
        ++m_itr;
        return *this;
    }
private:
    constexpr bool is_end() const
        { return m_itr.next_is_end(); }

    BezierIterator<Vec, TupleTypes...> m_itr;
};

template <typename Vec, typename ... TupleTypes>
class BezierStripBaseIterator {
protected:
    using PtIterator = BezierIterator<Vec, TupleTypes...>;

    constexpr BezierStripBaseIterator(PtIterator && ws_, PtIterator && os_):
        m_ws(std::move(ws_)),
        m_os(std::move(os_))
    {
        // tip skip
        if (magnitude(VecOpHelpers<Vec>::template sub<0>( *m_ws, *m_os )) < 0.005)
            { ++m_os; }
    }

    constexpr void advance() {
        std::swap(m_os, m_ws);
        m_on_left = !m_on_left;
        ++m_os;
    }

    constexpr bool is_end() const {
        auto ws_cpy = m_ws;
        ++ws_cpy;
        return ws_cpy == BezierEndIterator{};
    }

    constexpr Tuple<Vec, Vec, Vec> points() const {
        auto ws_cpy = m_ws;
        ++ws_cpy;
        auto rv = std::make_tuple( *m_ws, *m_os, *ws_cpy );
        return rv;
    }

    constexpr auto points_and_sides() const {
        using Impl = BezierStripDetails<Vec>;
        auto nws = m_ws;
        ++nws;
        return std::make_tuple(
            Impl{ m_on_left, *m_ws, m_ws.curve_position()},
            Impl{!m_on_left, *m_os, m_os.curve_position()},
            Impl{ m_on_left, *nws , nws .curve_position()});
    }

private:
    PtIterator m_ws, m_os;
    bool m_on_left = true;
};

template <typename Vec, typename ... TupleTypes>
class BezierStripTrianglesIterator final :
    public BezierStripBaseIterator<Vec, TupleTypes...>
{
    using Base = BezierStripBaseIterator<Vec, TupleTypes...>;
public:
    using PtIterator = typename Base::PtIterator;

    constexpr BezierStripTrianglesIterator
        (PtIterator && ws_, PtIterator && os_):
        Base(std::move(ws_), std::move(os_)) {}

    constexpr BezierStripTrianglesIterator & operator ++ () {
        Base::advance();
        return *this;
    }

    constexpr Tuple<Vec, Vec, Vec> operator * () const
        { return Base::points(); }

    constexpr bool operator != (const BezierEndIterator &) const
        { return !Base::is_end(); }

    constexpr bool operator == (const BezierEndIterator &) const
        { return Base::is_end(); }
};

template <typename Vec, typename ... TupleTypes>
class BezierStripDetailedIterator final :
    public BezierStripBaseIterator<Vec, TupleTypes...>
{
    using Base = BezierStripBaseIterator<Vec, TupleTypes...>;
public:
    using PtIterator = typename Base::PtIterator;

    constexpr BezierStripDetailedIterator
        (PtIterator && ws_, PtIterator && os_):
        Base(std::move(ws_), std::move(os_)) {}

    constexpr BezierStripDetailedIterator & operator ++ () {
        Base::advance();
        return *this;
    }

    constexpr auto operator * () const
        { return Base::points_and_sides(); }

    constexpr bool operator != (const BezierEndIterator &) const
        { return !Base::is_end(); }

    constexpr bool operator == (const BezierEndIterator &) const
        { return Base::is_end(); }
};

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
    View<detail::BezierLineIterator<Vec, Types...>, detail::BezierEndIterator>>
    make_bezier_line_view
    (const Tuple<Vec, Types...> & tuple, int number_of_points)
{
    // number of points must be at least two
    using namespace detail;
    using LineItr = BezierLineIterator<Vec, Types...>;
    auto pt_view = make_bezier_point_view(tuple, number_of_points);
    return View<LineItr, BezierEndIterator>{LineItr{pt_view.begin()}, BezierEndIterator{}};
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
constexpr BezierStrip<Vec, Types...>::BezierStrip
    (PtIterator && lhs, PtIterator && rhs):
    m_lhs(std::move(lhs)),
    m_rhs(std::move(rhs))
{}

template <typename Vec, typename ... Types>
constexpr typename BezierStrip<Vec, Types...>::PointsView
    BezierStrip<Vec, Types...>::points_view() const
{
    using namespace detail;
    // copy has to be used here... what if the view is used not solely as an
    // rvalue?
    using std::move;
    auto l = m_lhs, r = m_rhs;
    return PointsView{
        BezierStripTrianglesIterator<Vec, Types...>{move(l), move(r)},
        BezierEndIterator{}};
}

template <typename Vec, typename ... Types>
constexpr typename BezierStrip<Vec, Types...>::DetailedView
    BezierStrip<Vec, Types...>::details_view() const
{
    using namespace detail;
    using std::move;
    auto l = m_lhs, r = m_rhs;
    return DetailedView{
        BezierStripDetailedIterator<Vec, Types...>{move(l), move(r)},
        BezierEndIterator{}};
}

template <typename Vec, typename ... Types>
constexpr EnableIf<detail::k_are_vector_types<Vec, Types...>,
    BezierStrip<Vec, Types...>>
    make_bezier_strip
    (const Tuple<Vec, Types...> & lhs, const Tuple<Vec, Types...> & rhs,
     int number_of_points_per_side)
{
    return BezierStrip<Vec, Types...>{
        make_bezier_point_view(lhs, number_of_points_per_side).begin(),
        make_bezier_point_view(rhs, number_of_points_per_side).begin()};
}

#endif // ifndef DOXYGEN_SHOULD_SKIP_THIS

} // end of cul namespace
