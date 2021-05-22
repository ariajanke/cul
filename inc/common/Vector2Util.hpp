/****************************************************************************

    MIT License

    Copyright (c) 2021 Aria Janke

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

#include <common/Util.hpp>

#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Rect.hpp>

#include <tuple>

/* Styling note: "inline" is used for every function that are implemented
 * where they are declared, including template functions.
 *
 */
namespace cul {

template <typename T>
using Vector2 = sf::Vector2<T>;

template <typename T>
using Rectangle = sf::Rect<T>;

template <typename T>
using EnableVector2 = std::enable_if_t<std::is_arithmetic_v<T>, Vector2<T>>;

template <typename T>
using EnableRectangle = std::enable_if_t<std::is_arithmetic_v<T>, Rectangle<T>>;

/** @returns magnitude of v
 *
 *  @throws if v does not have real components
 *  @tparam T must be arithmetic
 *  @param v must be a vector of real components
 */
template <typename T>
EnableArithmetic<T> magnitude(const Vector2<T> & v);

/** @returns normal vector of v
 *
 *  @throws if v does not have real components or is the zero vector
 *  @tparam T must be arithmetic
 *  @param v must be a vector of real components
 */
template <typename T>
EnableVector2<T> normalize(const Vector2<T> & v);

template <typename T>
inline std::enable_if_t<std::is_arithmetic_v<T>, bool>
    are_within(const Vector2<T> & a, const Vector2<T> & b, T error)
{ return are_within(a.x, b.x, error) && are_within(a.y, b.y, error); }

/** Rotates vector r along the unit circle of some given number of radians rot.
 *
 *  @throws if rot or the components of r are not real numbers
 *  @tparam T must be arithmetic
 *  @param r must be a vector of real components
 *  @param rot must be a real number, in radians
 *  @return the rotated vector
 */
template <typename T>
EnableVector2<T> rotate_vector(const Vector2<T> & r, T rot);

/** @returns the dot product of two vectors
 *
 *  @tparam T must be arithmetic
 *  @note no checks if components are real numbers
 */
template <typename T>
inline EnableArithmetic<T> dot(const Vector2<T> & v, const Vector2<T> & u) noexcept
    { return v.x*u.x + v.y*u.y; }

/** @returns the "z" component of the cross product of two 2D vectors.
 *
 *  @tparam T must be arithmetic
 *  @note no checks if components are real numbers
 */
template <typename T>
inline EnableArithmetic<T> cross(const Vector2<T> & v, const Vector2<T> & u) noexcept
    { return v.x*u.y - u.x*v.y; }

/** @returns the magnitude of the angle between two vectors
 *
 *  @throws if any component of either vector is not a real number
 *  @note if direction is also desired std::atan2 can be used
 *  @tparam T must be arithmetic
 */
template <typename T>
EnableArithmetic<T> angle_between(const Vector2<T> & v, const Vector2<T> & u);

/** @returns the projection of vector a onto vector b
 *
 *  @throws if either a or b have non real components or if b is the zero
 *          vector
 *  @tparam T must be arithmetic
 */
template <typename T>
EnableVector2<T> project_onto(const Vector2<T> & a, const Vector2<T> & b);

/** @returns The intersection between two lines "a" and "b", or a special
 *           sentinel value if no intersection can be found.
 *
 *  @note You can find the "no intersection" sentinel value by given vectors
 *        that you know will produce it. Unfortunately revealing this value
 *        directly is not possible while no "constexpr" constructor exists for
 *        the Vector2 type.
 *
 *  @throws if any vector as a non real component
 *  @tparam T must be arithmetic
 *  @param a_first the first point of line "a"
 *  @param a_second the second point of line "a"
 *  @param b_first the first point of line "b"
 *  @param b_second the second point of line "b"
 */
template <typename T>
EnableVector2<T> find_intersection
    (const Vector2<T> & a_first, const Vector2<T> & a_second,
     const Vector2<T> & b_first, const Vector2<T> & b_second);

/** @returns the "no intersection" value for the find_intersection function.
 */
template <typename T>
EnableVector2<T> get_find_intersection_no_solution();

/** @returns A vector rounded from a floating point to an integer type.
 *
 *  This just calls "round_to" for each respective component, no regard for
 *  whether they are real numbers or not.
 */
template <typename T, typename U>
inline std::enable_if_t<std::is_integral_v<T> && std::is_floating_point_v<U>, Vector2<T>>
    round_to(const Vector2<U> & r)
{ return Vector2<T>(round_to<T>(r.x), round_to<T>(r.y)); }

/** Computes velocties in which a projectile can reach a target from a source
 *  given speed and influencing_acceleration (like gravity).
 *
 *  @returns a tuple with two solutions to this problem, special sentinel
 *           values are returned if there is no solution, and if there is only
 *           one solution to this ballistics problem, then both values returned
 *           will be the same
 *
 *  @note much like find_intersection, the special "no solutions" sentinel
 *        value can be found by providing parameters which you no there is no
 *        solution (like shooting at a target with a gravity which is *far* too
 *        strong), this value does not have real components
 *
 *  @tparam T must be a floating point
 *  @throws if any component of any vector or scalar is not a real number
 *  @param source projectile starting point
 *  @param target projectile's target
 *  @param influencing_acceleration any influencing force (without the mass
 *         portion, e.g. gravity)
 *  @param speed The speed at which to fire the projectile
 */
template <typename T>
std::enable_if_t<std::is_floating_point_v<T>, std::tuple<Vector2<T>, Vector2<T>>>
    compute_velocities_to_target
    (const Vector2<T> & source, const Vector2<T> & target,
     const Vector2<T> & influencing_acceleration, T speed);

/** @returns the "no solution" vector for the compute_velocities_to_target
 *           function.
 */
template <typename T>
std::enable_if_t<std::is_floating_point_v<T>, std::tuple<Vector2<T>, Vector2<T>>>
    get_compute_velocities_to_target_no_solution();

/** @returns the closest point on the line defined by points "a" and "b" to
 *           some given external point
 *  @tparam T must be arithmetic
 *  @throws if any component of any vector is not a real number
 *  @param a one extreme point of the line segment
 *  @param b another extreme point of the line segment
 *  @param external_point some point outside of the line
 */
template <typename T>
EnableVector2<T> find_closest_point_to_line
    (const Vector2<T> & a, const Vector2<T> & b, const Vector2<T> & external_point);

/** @returns true if both components are real numbers, false otherwise */
template <typename T>
inline std::enable_if_t<std::is_arithmetic_v<T>, bool> is_real
    (const Vector2<T> & r)
    { return is_real(r.x) && is_real(r.y); }

// ------------------ everything pretaining to rectangles ---------------------

/** Size in two dimensions, a width and a height. */
template <typename T>
struct Size2 {
    Size2() {}
    Size2(T w_, T h_): width(w_), height(h_) {}

    template <typename U>
    explicit Size2(const Size2<U> & rhs):
        width(T(rhs.width)), height(T(rhs.height)) {}

    T width = 0, height = 0;
};

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
inline void set_size_of(EnableRectangle<T> & rect, const Size2<T> & r)
    { set_size_of(rect, r.width, r.height); }

template <typename T>
inline Vector2<T> top_left_of(const Rectangle<T> & rect)
    { return Vector2<T>(rect.left, rect.top); }

template <typename T>
inline EnableArithmetic<T>
    right_of(const Rectangle<T> & rect) { return rect.left + rect.width; }

template <typename T>
inline EnableArithmetic<T>
    bottom_of(const Rectangle<T> & rect) { return rect.top + rect.height; }

template <typename T>
std::enable_if_t<std::is_arithmetic_v<T>, Size2<T>>
    size_of(const Rectangle<T> &);

template <typename T>
EnableVector2<T> center_of(const Rectangle<T> &);

template <typename T>
EnableRectangle<T> compute_rectangle_intersection
    (const Rectangle<T> &, const Rectangle<T> &);

template <typename T>
inline EnableArithmetic<T> area_of(const Rectangle<T> & a)
    { return a.width*a.height; }

template <typename T>
EnableRectangle<T> compose(const Vector2<T> & top_left, const Size2<T> &);

template <typename T>
inline std::enable_if_t<std::is_arithmetic_v<T>, Tuple<Vector2<T>, Size2<T>>>
    decompose(const Rectangle<T> & rect)
{ return std::make_tuple(top_left_of(rect), size_of(rect)); }

// ----------------------- Implementation Details -----------------------------

template <typename T>
EnableArithmetic<T> magnitude(const Vector2<T> & v) {
    using namespace exceptions_abbr;
    if (is_real(v)) return std::sqrt(v.x*v.x + v.y*v.y);
    throw InvArg("magnitude: given vector must have real number components.");
}

template <typename T>
EnableVector2<T> normalize(const Vector2<T> & v) {
    using namespace exceptions_abbr;
    if (is_real(v) && v != Vector2<T>()) return v*(T(1) / magnitude(v));
    throw InvArg("normalize: attempting to normalize a non real or zero vector.");
}

template <typename T>
EnableVector2<T> rotate_vector(const Vector2<T> & r, T rot) {
    // [r.x] * [ cos(rot) sin(rot)]
    // [r.y]   [-sin(rot) cos(rot)]
    return Vector2<T>(r.x*std::cos(rot) - r.y*std::sin(rot),
                      r.x*std::sin(rot) + r.y*std::cos(rot));
}

template <typename T>
EnableArithmetic<T> angle_between(const Vector2<T> & v, const Vector2<T> & u) {
    using namespace exceptions_abbr;
    static const constexpr T k_error = 0.00005;

    if (!is_real(v) || !is_real(u)
        || (v.x*v.x + v.y*v.y) < k_error*k_error
        || (u.x*u.x + u.y*u.y) < k_error*k_error)
    {
        throw InvArg("angle_between: both vectors must be non-zero vectors "
                     "with real components.");
    }
    T frac = dot(v, u) / (magnitude(u)*magnitude(v));
    if      (frac > T( 1)) { frac = T( 1); }
    else if (frac < T(-1)) { frac = T(-1); }
    return std::acos(frac);
}

template <typename T>
EnableVector2<T> project_onto(const Vector2<T> & a, const Vector2<T> & b) {
    using namespace exceptions_abbr;
    static const constexpr T k_error = 0.00005;
    if ((a.x*a.x + a.y*a.y) < k_error*k_error)
        throw InvArg("project_onto: cannot project onto the zero vector.");
    if (!is_real(a) || !is_real(b))
        throw InvArg("project_onto: both vectors must be real.");
    return (dot(b, a)/(b.x*b.x + b.y*b.y))*b;
}

template <typename T>
EnableVector2<T> find_intersection
    (const Vector2<T> & a_first, const Vector2<T> & a_second,
     const Vector2<T> & b_first, const Vector2<T> & b_second)
{
    using namespace exceptions_abbr;
    if constexpr (!std::is_floating_point_v<T>) {
        throw RtError("find_intersection: finding intersections on integer "
                      "vectors is not defined (perhaps a future feature?).");
    }
    static const Vector2<T> k_no_intersection = Vector2<T>
        (std::numeric_limits<T>::infinity(), std::numeric_limits<T>::infinity());

    auto p = a_first;
    auto r = a_second - p;

    auto q = b_first ;
    auto s = b_second - q;

    // http://stackoverflow.com/questions/563198/how-do-you-detect-where-two-line-segments-intersect
    // two points of early failure
    auto r_cross_s = cross(r, s);
    if (r_cross_s == T(0)) return k_no_intersection;

    auto q_sub_p = q - p;
    auto t = cross(q_sub_p, s) / r_cross_s;

    if (t < T(0) || t > T(1)) return k_no_intersection;

    auto u = cross(q_sub_p, r) / r_cross_s;
    if (u < T(0) || u > T(1)) return k_no_intersection;

    return p + t*r;
}

template <typename T>
EnableVector2<T> get_find_intersection_no_solution() {
    return find_intersection(Vector2<T>(0, 0), Vector2<T>(1, 0)
                            ,Vector2<T>(0, 2), Vector2<T>(1, 2));
}

template <typename T>
std::enable_if_t<std::is_floating_point_v<T>, std::tuple<Vector2<T>,Vector2<T>>>
    compute_velocities_to_target
    (const Vector2<T> & source, const Vector2<T> & target,
     const Vector2<T> & influencing_acceleration, T speed)
{
    // Made possible by this wonderful resource:
    // https://www.forrestthewoods.com/blog/solving_ballistic_trajectories/
    // Thank you for releasing your work (demo source code specifically) under
    // the public domain Forrest Smith

    // note: I plan on relocating many of my utils to the commons library
    //       in order to release them under a more permissive license
    using Vec = sf::Vector2<T>;
    static constexpr const T k_error = 0.00025;
    static auto are_very_close_v = [](const Vec & a, const Vec & b)
        { return are_within(a, b, k_error); };
    static auto are_very_close_s = [](const T & a, const T & b)
        { return are_within(a, b, k_error); };
    static const auto k_no_solution = std::make_tuple
        (get_find_intersection_no_solution<T>(), get_find_intersection_no_solution<T>());
    using std::make_tuple;
    if (   !is_real(source) || !is_real(speed)
        || !is_real(target) || !is_real(influencing_acceleration))
    {
        throw std::invalid_argument("compute_velocities_to_target: All "
                                    "arguments must be real numbers.");
    }
    if (are_very_close_v(source, target)) {
        if (are_very_close_v(influencing_acceleration, Vec())) {
            return make_tuple(Vec(), Vec());
        }
        // return straight up trajectory
        auto s = -normalize(influencing_acceleration)*speed;
        return make_tuple(s, s);
    }
    if (are_very_close_v(influencing_acceleration, Vec())) {
        // return straight to target
        auto s = normalize(target - source)*speed;
        return make_tuple(s, s);
    }

    // assumptions at this point source != target && acc != 0
    static auto are_parallel = [](Vec a, Vec b)
        { return are_very_close_v(normalize(a), normalize(b)); };
    static auto comp_from_basis = [](Vec a, Vec basis) {
        auto proj = project_onto(a, basis);
        return magnitude(proj)*( are_parallel(proj, basis) ? T(1) : T(-1) );
    };

    auto j = -normalize(influencing_acceleration);
    auto i = rotate_vector(j, k_pi_for_type<T>*0.5);

    T t0, t1;
    {
        auto diff_i = magnitude(project_onto(target - source, i));
        auto diff_j = comp_from_basis(target - source, j);

        auto spd_sq = speed*speed;
        auto g = magnitude(influencing_acceleration);
        auto do_atan_with_sqpart = [spd_sq, g, diff_i] (T sqpart)
            { return std::atan( (spd_sq + sqpart) / (g*diff_i) ); };

        auto randicand = spd_sq*spd_sq - g*(g*diff_i*diff_i + T(2)*spd_sq*diff_j);
        if (randicand < T(0)) return k_no_solution;
        auto sqpart = std::sqrt(randicand);
        t0 = do_atan_with_sqpart( sqpart);
        t1 = do_atan_with_sqpart(-sqpart);

    }
    auto ground_dir = normalize(project_onto(target - source, i));
    auto up         = -normalize(influencing_acceleration);

    auto s0 = ground_dir*std::cos(t0)*speed + up*std::sin(t0)*speed;
    if (are_very_close_s(t0, t1)) { return make_tuple(s0, s0); }
    return make_tuple(s0, ground_dir*std::cos(t1)*speed + up*std::sin(t1)*speed);
}

template <typename T>
std::enable_if_t<std::is_floating_point_v<T>, std::tuple<Vector2<T>, Vector2<T>>>
    get_compute_velocities_to_target_no_solution()
{
    return compute_velocities_to_target
        (Vector2<T>(0, 0), Vector2<T>(10000, 10000)
        ,Vector2<T>(-10000, -10000), T(1));
}

template <typename T>
EnableVector2<T> find_closest_point_to_line
    (const Vector2<T> & a, const Vector2<T> & b,
     const Vector2<T> & external_point)
{
    using Vec = Vector2<T>;
    const auto & c = external_point;
    if (a == b) return a;
    if (a - c == Vec()) return a;
    if (b - c == Vec()) return b;
    // obtuse angles -> snap to extreme points
    auto angle_at_a = angle_between(a - b, a - c);
    if (angle_at_a > k_pi_for_type<T>*0.5) return a;

    auto angle_at_b = angle_between(b - a, b - c);
    if (angle_at_b > k_pi_for_type<T>*0.5) return b;

    // https://www.eecs.umich.edu/courses/eecs380/HANDOUTS/PROJ2/LinePoint.html
    T mag = [&a, &b, &c]() {
        auto num = (c.x - a.x)*(b.x - a.x) + (c.y - a.y)*(b.y - a.y);
        auto denom = magnitude(b - a);
        return num / (denom*denom);
    }();
    return Vec(a.x, a.y) + mag*Vec(b.x - a.x, b.y - a.y);
}

// ----------------------- Implementation Details -----------------------------
// ------------------ everything pretaining to rectangles ---------------------

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
EnableVector2<T> center_of(const Rectangle<T> & rect) {
    return sf::Vector2<T>(rect.left + rect.width  / T(2),
                          rect.top  + rect.height / T(2));
}

template <typename T>
EnableRectangle<T> compute_rectangle_intersection
    (const Rectangle<T> & a, const Rectangle<T> & b)
{
    using TVec = sf::Vector2<T>;
    auto high_a = TVec(right_of(a), bottom_of(a));
    auto high_b = TVec(right_of(b), bottom_of(b));

    auto low_rv  = TVec(std::max(a.left, b.left), std::max(a.top, b.top));
    auto high_rv = TVec(std::min(high_a.x, high_b.x), std::min(high_a.y, high_b.y));
    if (low_rv.x >= high_rv.x || low_rv.y >= high_rv.y) {
        return sf::Rect<T>();
    } else {
        return sf::Rect<T>(low_rv.x, low_rv.y,
                           high_rv.x - low_rv.x, high_rv.y - low_rv.y);
    }
}

template <typename T>
EnableRectangle<T> compose(const Vector2<T> & top_left, const Size2<T> & size) {
    return Rectangle<T>(top_left.x, top_left.y, size.width, size.height);
}

} // end of cul namespace

template <typename T>
bool operator == (const cul::Size2<T> & lhs, const cul::Size2<T> & rhs)
    { return rhs.width == lhs.width && rhs.height == lhs.height; }

template <typename T>
bool operator != (const cul::Size2<T> & lhs, const cul::Size2<T> & rhs)
    { return rhs.width != lhs.width && rhs.height != lhs.height; }
