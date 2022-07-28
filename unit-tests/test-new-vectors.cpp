#include <stdexcept>
#include <utility>
#include <random>

#include <cmath>

#include <common/Util.hpp>
#include <common/Vector2.hpp>
#include <common/Vector3.hpp>
#include <common/VectorUtils.hpp>
#include <common/sf/VectorTraits.hpp>

#if 0
// following prototype... replaced with simply "cul"
namespace culn {

/** Default definitions for VectorTraits for all non-vector types.
 *
 *  This also serves as a template to specializing the template for an actual
 *  vector type.
 *
 *  @note Further documentation on this will not (usually) be provided. So
 *        documentation here will explain what specializations should mean.
 *  @note to make constexpr functions constexpr all features defined in a
 *        class specialization must also be constexpr
 */
template <typename VectorType>
struct VectorTraits final {
    /** Specializations of this class need not define this type. It is here
     *  simply to make other structures/functions defined here valid.
     */
    class Dummy final {};

    /** When this constant is true, this tells many utility functions that this
     *  VectorType maybe treated as a vector.
     */
    static constexpr const bool k_is_vector_type = false;

    /** When this constant is true, this will enable a number of vector
     *  operator definitions including unary -, addition, subtraction, scalar
     *  multipication, scalar division, equality and inequality operators.
     *  @note that operators are all defined under this namespace
     */
    static constexpr const bool k_should_define_operators = false;

    /** All specializations should define "ScalarType" which aliases the type
     *  used for the vector's components, and scalars used in other utilities.
     */
    using ScalarType = Dummy;

    // index selection... should I use enumerations?
    // I wish to also use indices
    template <int kt_idx>
    struct Get final {
        constexpr ScalarType operator () (const VectorType &) const
            { return Dummy{}; }
    };

    /** Each specialization should define a "Make" struct.
     *
     *  @see the defined member operator for more details
     */
    struct Make final {

        /** The call operator for Make should take sequentially provided
         *  components. The number of components should match the
         *  dimensionality of the vector type.
         *  @note to make constexpr functions constexpr, this operator will
         *        need to be constexpr itself
         *  @returns a freshly created instance of vector type
         *
         */
        template <typename ... Types>
        VectorType operator () (Types && ...) const
            { throw std::runtime_error("VectorTraits::Make::operator(): Should not use \"Make\" on non-vector type."); }
    };

    /** Each specialization should also define a template type alias, allowing
     *  utilities to figure out what a differect vector type would be for a
     *  different component type.
     *
     *  @note It is not needed by many of the utilities however. Many vector
     *        types are usually defined as templates themselves.
     */
    template <typename U>
    using ChangeScalarType = Dummy;

    /** Dimensionality of the vector type
     *
     *  3D vector types for instance would indicate that this value is declared,
     *  constexpr, and equal to three
     */
    static constexpr const int k_dimension_count = 0;
};

template <typename VectorType>
constexpr const bool k_is_vector_type = VectorTraits<VectorType>::k_is_vector_type;

template <typename VectorType>
constexpr const bool k_should_define_vector_operators
    = VectorTraits<VectorType>::k_should_define_operators;

template <bool kt_condition, typename T>
using EnableIf = std::enable_if_t<kt_condition, T>;

template <typename VectorType>
using ScalarTypeOf = typename VectorTraits<VectorType>::ScalarType;

template <typename VectorType, typename T>
using EnableVector = EnableIf<
    k_should_define_vector_operators<VectorType>, T>;

template <typename VectorType>
using EnableVectorOp = EnableIf<
    k_should_define_vector_operators<VectorType>, VectorType>;

// ---------------------------- Unary Vector Operator --------------------------

template <typename Vec>
class VecOpHelpers final {
    VecOpHelpers() {}
public:
    using Traits = VectorTraits<Vec>;
    using Make = typename Traits::Make;
    template <int kt_idx>
    using Get = typename Traits::template Get<kt_idx>;
    using ScalarType = typename Traits::ScalarType;
    static constexpr const auto k_dim_count = Traits::k_dimension_count;

    template <int kt_idx, typename ... Types>
    static constexpr Vec neg(const Vec & r, Types && ... args) {
        if constexpr (kt_idx >= k_dim_count) {
            return Make{}( std::forward<Types>(args)... );
        } else {
            return neg<kt_idx + 1>
                (r, std::forward<Types>(args)..., -Get<kt_idx>{}(r));
        }
    }

    template <int kt_idx>
    static constexpr EnableIf<k_is_vector_type<Vec>, bool> eq(const Vec & l, const Vec & r) {
        if constexpr (kt_idx >= Traits::k_dimension_count) {
            // interesting base case wherein to 0D vectors are always equal
            return true;
        } else {
            return    eq<kt_idx + 1>(l, r)
                   && Get<kt_idx>{}(l) == Get<kt_idx>{}(r);
        }
    }

    template <int kt_idx, typename ... Types>
    static constexpr EnableIf<k_is_vector_type<Vec>, Vec> plus
        (const Vec & l, const Vec & r, Types && ... comps)
    {
        using std::forward;
        if constexpr (kt_idx >= k_dim_count) {
            return Make{}( forward<Types>(comps)... );
        } else {
            using GetComp = Get<kt_idx>;
            return plus<kt_idx + 1>(l, r, forward<Types>(comps)...,
                                    GetComp{}(l) + GetComp{}(r));
        }
    }

    template <int kt_idx, typename ... Types>
    static constexpr EnableIf<k_is_vector_type<Vec>, Vec> sub
        (const Vec & l, const Vec & r, Types && ... comps)
    {
        using std::forward;
        if constexpr (kt_idx >= k_dim_count) {
            return Make{}( forward<Types>(comps)... );
        } else {
            using GetComp = Get<kt_idx>;
            return sub<kt_idx + 1>(l, r, forward<Types>(comps)...,
                                   GetComp{}(l) - GetComp{}(r));
        }
    }

    template <int kt_idx, typename ... Types>
    static constexpr EnableIf<k_is_vector_type<Vec>, Vec> mul
        (const Vec & l, const ScalarType & a, Types && ... comps)
    {
        using std::forward;
        if constexpr (kt_idx >= k_dim_count) {
            return Make{}( forward<Types>(comps)... );
        } else {
            return mul<kt_idx + 1>(l, a, std::forward<Types>(comps)...,
                                   Get<kt_idx>{}(l)*a);
        }
    }

    template <int kt_idx, typename ... Types>
    static constexpr EnableIf<k_is_vector_type<Vec>, Vec> div
        (const Vec & l, const ScalarType & a, Types && ... comps)
    {
        using std::forward;
        if constexpr (kt_idx >= k_dim_count) {
            return Make{}( forward<Types>(comps)... );
        } else {
            return div<kt_idx + 1>(l, a, forward<Types>(comps)...,
                                   Get<kt_idx>{}(l) / a);
        }
    }

    template <typename DestVec, int kt_idx, typename ... Types>
    static constexpr EnableIf<k_is_vector_type<Vec>, DestVec>
        convert(const Vec & r, Types && ... comps)
    {
        using std::forward;
        using DestMake = typename VectorTraits<DestVec>::Make;
        using DestScalar = ScalarTypeOf<DestVec>;
        if constexpr (kt_idx >= k_dim_count) {
            return DestMake{}( forward<Types>(comps)... );
        } else {
            return convert<DestVec, kt_idx + 1>
                (r, forward<Types>(comps)..., DestScalar(Get<kt_idx>{}(r)));
        }
    }

    template <int kt_idx, typename ... Types>
    static constexpr EnableIf<k_is_vector_type<Vec>, Vec>
        make_from_single_scalar(const ScalarType & a, Types && ... comps)
    {
        using std::forward;
        if constexpr (kt_idx >= k_dim_count) {
            return Make{}( forward<Types>(comps)... );
        } else {
            return make_from_single_scalar<kt_idx +1>(a, forward<Types>(comps)..., a);
        }
    }
};


// A lot of operators are map/reduce?
// At least put them in a class...
// though hardly conducive to "privating" everything without friend
// declarations everywhere

// ---------------------------- Unary Vector Operator --------------------------

template <typename VectorType>
constexpr EnableVectorOp<VectorType>
    operator - (const VectorType & r)
{ return VecOpHelpers<VectorType>::template neg<0>(r); }

// -------------------------- Vector on Vector operators -----------------------

template <typename VectorType>
constexpr EnableVectorOp<VectorType>
    operator + (const VectorType & lhs, const VectorType & rhs)
{ return VecOpHelpers<VectorType>::template plus<0>(lhs, rhs); }

template <typename VectorType>
constexpr EnableVectorOp<VectorType>
    operator - (const VectorType & lhs, const VectorType & rhs)
{ return VecOpHelpers<VectorType>::template sub<0>(lhs, rhs); }

template <typename VectorType>
constexpr EnableVector<VectorType, bool> operator ==
    (const VectorType & lhs, const VectorType & rhs)
{ return VecOpHelpers<VectorType>::template eq<0>(lhs, rhs); }

template <typename VectorType>
constexpr EnableVector<VectorType, bool> operator !=
    (const VectorType & lhs, const VectorType & rhs)
{ return !VecOpHelpers<VectorType>::template eq<0>(lhs, rhs); }

// guess these are runtime only :/
template <typename VectorType>
EnableVector<VectorType, VectorType> &
    operator += (VectorType & r, const VectorType & v)
{ return ( r = (r + v) ); }

template <typename VectorType>
EnableVector<VectorType, VectorType> &
    operator -= (VectorType & r, const VectorType & v)
{ return ( r = (r - v) ); }

// -------------------------- Vector on Scalar operators -----------------------

template <typename VectorType>
EnableVector<VectorType, VectorType> &
    operator *= (VectorType & r, const ScalarTypeOf<VectorType> & a)
{ return (r = ( r*a )); }

template <typename VectorType>
EnableVector<VectorType, VectorType> &
    operator /= (VectorType & r, const ScalarTypeOf<VectorType> & a)
{ return (r = ( r/a )); }

template <typename VectorType>
constexpr EnableVector<VectorType, VectorType>
    operator * (const VectorType & r, const ScalarTypeOf<VectorType> & a)
{ return VecOpHelpers<VectorType>::template mul<0>(r, a); }

template <typename VectorType>
constexpr EnableVector<VectorType, VectorType>
    operator / (const VectorType & r, const ScalarTypeOf<VectorType> & a)
{ return VecOpHelpers<VectorType>::template div<0>(r, a); }

template <typename VectorType>
constexpr EnableVector<VectorType, VectorType>
    operator * (const ScalarTypeOf<VectorType> & a, const VectorType & r)
{ return VecOpHelpers<VectorType>::template mul<0>(r, a); }

// ----------------------------------------------------------------------------

/** Defines a 2D vector for a given scalar type.
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
struct Vector2 final {
    constexpr Vector2() {}

    constexpr Vector2(T x_, T y_): x(x_), y(y_) {}

    template <typename U>
    constexpr explicit Vector2(U x_, U y_): x(T(x_)), y(T(y_)) {}

    template <typename U>
    constexpr explicit Vector2(const Vector2<U> & r): x(T(r.x)), y(T(r.y)) {}

    T x = 0, y = 0;
};

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
struct VectorTraits<Vector2<T>> final {
    static constexpr const bool k_is_vector_type = true;

    static constexpr const bool k_should_define_operators = true;

    using ScalarType = T;

    // may not be feasible in g++... :c
    template <int kt_idx, typename=int>
    struct Get final {};

    template <typename U>
    struct Get<0, U> final {
        constexpr ScalarType operator () (const Vector2<T> & r) const
            { return r.x; }
    };

    template <typename U>
    struct Get<1, U> final {
        constexpr ScalarType operator () (const Vector2<T> & r) const
            { return r.y; }
    };

    template <int kt_idx>
    static constexpr ScalarType get(const Vector2<T> &);

    struct Make final {
        constexpr Vector2<T> operator () (const T & x, const T & y) const
            { return Vector2<T>{x, y}; }
    };

    template <typename U>
    using ChangeScalarType = Vector2<U>;

    static constexpr const int k_dimension_count = 2;
};

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
    using ChangeScalarType = Vector2<U>;

    static constexpr const int k_dimension_count = 3;
};

using cul::is_real, cul::magnitude;

#endif

// ----------------------------- Utility Functions ----------------------------

template <typename ... Types>
using Tuple = std::tuple<Types...>;

/** @addtogroup vecutils
 *  @{
 */

/** return type for round_to function */
template <typename OtherScalar, typename Vec>
using EnableRoundTo = EnableIf<
    std::is_floating_point_v<ScalarTypeOf<Vec>> &&
    std::is_integral_v<OtherScalar>,
    typename VectorTraits<Vec>::template ChangeScalarType<OtherScalar> >;

/** @returns the magnitude of the angle between two vectors, in radians
 *
 *  @throws if any component of either vector is not a real number
 *  @note if direction is also desired directed_angle_between can be used
 *  @see rotate_vector, directed_angle_between
 *  @tparam Vec maybe any vector type
 *  @param v any real vector
 *  @param u any real vector
 */
template <typename Vec>
EnableIf<k_is_vector_type<Vec>, ScalarTypeOf<Vec>>
    angle_between(const Vec & v, const Vec & u);

/** @returns true if both vectors are within some number of units within each
 *           other.
 *  @throws if any are not real numbers: either vector's component, the error
 *          parameter
 *  @tparam Vec maybe any vector type
 *  @param a any real vector
 *  @param b any real vector
 *  @param error any non-negative real number, providing a negative number will
 *         cause this function to always return true
 */
template <typename Vec>
constexpr EnableIf<k_is_vector_type<Vec>, bool>
    are_within(const Vec & a, const Vec & b, const ScalarTypeOf<Vec> & error);

/** @returns the area of a triangle as defined by three points.
 *
 *  @tparam Vec maybe any vector type
 *  @param a any real vector
 *  @param b any real vector
 *  @param c any real vector
 */
template <typename Vec>
EnableIf<k_is_vector_type<Vec>, ScalarTypeOf<Vec>>
    area_of_triangle(const Vec & a, const Vec & b, const Vec & c);

/** @returns a vector of a desired destination type
 *  @tparam DestType is an explicitly specified destination vector type of the
 *          same dimensionality as the source type. Both types may vary in
 *          their scalar types.
 *  @tparam SourceType is an (usually) implicitly specified vector type.
 *  @param r any vector of SourceType, to be converted
 */
template <typename DestType, typename SourceType>
constexpr EnableIf<
       k_is_vector_type<DestType> && k_is_vector_type<SourceType>
    && (   VectorTraits<DestType  >::k_dimension_count
        == VectorTraits<SourceType>::k_dimension_count),
    DestType>
    convert_to(const SourceType & r)
{ return VecOpHelpers<SourceType>::template convert<DestType, 0>(r); }

/** @returns the "z" component of the cross product of two 2D vectors
 *  @tparam Vec any 2D vector type
 *  @param v any vector
 *  @param u any vector
 *  @note no checks if components are real numbers
 */
template <typename Vec>
constexpr EnableIf<VectorTraits<Vec>::k_dimension_count == 2,
         ScalarTypeOf<Vec>> cross(const Vec & v, const Vec & u);

/** @returns the cross product for two 3D vectors
 *  @tparam Vec any 3D vector type
 *  @param v any vector
 *  @param u any vector
 *  @note no checks if components are real numbers
 */
template <typename Vec>
constexpr EnableIf<VectorTraits<Vec>::k_dimension_count == 3,
         Vec> cross(const Vec & v, const Vec & u);

/** @returns the angle between the "from" and "to" vectors such that,
 *           rotate_vector(from, this_return_value) is roughly equal to vector
 *           "to".
 *
 *  @throws if any component of either vector is not a real number
 *  @see rotate_vector, angle_between
 *  @tparam Vec any 2D vector type
 *  @param from starting vector prerotation
 *  @param to   destination vector of any magnitude
 */
template <typename Vec>
EnableIf<VectorTraits<Vec>::k_dimension_count == 2,
    ScalarTypeOf<Vec>>
    directed_angle_between(const Vec & from, const Vec & to);

/** @returns the dot product of two vectors
 *  @tparam Vec maybe any vector type
 *  @param v any vector
 *  @param u any vector
 *  @note no checks if components are real numbers
 */
template <typename Vec>
constexpr EnableIf<k_is_vector_type<Vec>, ScalarTypeOf<Vec>>
    dot(const Vec & v, const Vec & u);

/** @returns the closest point on the line defined by points "a" and "b" to
 *           some given external point.
 *  @tparam Vec any vector type
 *  @throws if any component of any vector is not a real number
 *  @param a one extreme point of the line segment
 *  @param b another extreme point of the line segment
 *  @param external_point some point outside of the line
 */
template <typename Vec>
constexpr EnableIf<k_is_vector_type<Vec>, Vec>
    find_closest_point_to_line
    (const Vec & a, const Vec & b, const Vec & external_point);

/** @returns The intersection between two lines "a" and "b", or a special
 *           sentinel value if no intersection can be found.
 *
 *  @throws if any vector as a non real component
 *  @tparam Vec any two dimensional vector
 *  @param a_first the first point of line "a"
 *  @param a_second the second point of line "a"
 *  @param b_first the first point of line "b"
 *  @param b_second the second point of line "b"
 */
template <typename Vec>
constexpr EnableIf<VectorTraits<Vec>::k_dimension_count == 2, Vec>
    find_intersection(const Vec & a_first, const Vec & a_second,
                      const Vec & b_first, const Vec & b_second);

/** Computes velocties in which a projectile can reach a target from a source
 *  given speed and influencing_acceleration (like gravity).
 *
 *  @returns a tuple with two solutions to this problem. Special sentinel
 *           values are returned if there is no solution
 *
 *  @see is_solution
 *  @see Vec make_nonsolution_sentinel()
 *
 *  @throws if any component of any vector or scalar is not a real number
 *  @tparam Vec any vector type
 *  @param source projectile starting point
 *  @param target projectile's target
 *  @param influencing_acceleration any influencing force (without the mass
 *         portion, e.g. gravity)
 *  @param speed The speed at which to fire the projectile
 */
template <typename Vec>
EnableIf<k_is_vector_type<Vec> &&
         std::is_floating_point_v<ScalarTypeOf<Vec>>, Tuple<Vec, Vec>>
    find_velocities_to_target
    (const Vec & source, const Vec & target,
     const Vec & influencing_acceleration, ScalarTypeOf<Vec> speed);

/** @returns true if a test point is inside a triangle defined by three points.
 *  @tparam Vec any vector type, testing non-coplanar points is left undefined
 *  @param a A single end point defining the perimeter of the triangle.
 *  @param b A single end point defining the perimeter of the triangle.
 *  @param c A single end point defining the perimeter of the triangle.
 *  @param test_point a point to test whether it is inside the triangle or not.
 */
template <typename Vec>
constexpr EnableIf<k_is_vector_type<Vec>, bool> is_inside_triangle
    (const Vec & a, const Vec & b, const Vec & c,
     const Vec & test_point);

/** @returns true if the vector has all real components */
template <typename Vec>
constexpr EnableIf<k_is_vector_type<Vec> &&
         std::is_floating_point_v<ScalarTypeOf<Vec>>, bool>
    is_real(const Vec & r);

/** @returns true if the vector has all real components */
template <typename Vec>
constexpr EnableIf<k_is_vector_type<Vec> &&
         std::is_integral_v<ScalarTypeOf<Vec>>, bool>
    is_real(const Vec &)
{ return true; }

/** Some other functions may return "no solution" vectors. This function is
 *  defined to succinctly test that.
 *  @returns true if the vector is a valid solution, false if it's an instance
 *           of the "nonsolution" sentinel vector
 */
template <typename Vec>
constexpr EnableIf<
    k_is_vector_type<Vec>,
    bool> is_solution(const Vec & r);

/** (usefulness?)
 *  @warning Note that this says "is_zero_vector", which does not mean "close".
 *           Exercise the appropriate care when working floating points.
 *  @param r
 *  @returns true if r is the zero vector
 */
template <typename Vec>
constexpr EnableIf<k_is_vector_type<Vec>, bool>
    is_zero_vector(const Vec & r);

/** @returns magnitude of v
 *
 *  @throws if r does not have real components
 *  @tparam Vec any vector type
 *  @param r must be a vector of real components
 */
template <typename Vec>
EnableIf<k_is_vector_type<Vec>, ScalarTypeOf<Vec>>
    magnitude(const Vec & r);

/** @returns an instance of the zero vector
 *  @note some libraries do not default construct their vectors with zerod
 *        component values
 */
template <typename Vec>
constexpr EnableIf<k_is_vector_type<Vec>, Vec>
    make_zero_vector()
{ return VecOpHelpers<Vec>::template make_from_single_scalar<0>(0); }

/** @returns an instance of the "nonsolution" sentinel for a chosen vector type.
 *  @note that many utilities return this value, and the client may need to
 *        test if the vector is this sentinel
 *  @see is_solution
 */
template <typename Vec>
constexpr EnableIf<
    k_is_vector_type<Vec> && std::is_floating_point_v<ScalarTypeOf<Vec>>,
    Vec> make_nonsolution_sentinel();

/** @returns an instance of the "nonsolution" sentinel for a chosen vector type.
 *  @note that many utilities return this value, and the client may need to
 *        test if the vector is this sentinel
 *  @see is_solution
 */
template <typename Vec>
constexpr EnableIf<
    k_is_vector_type<Vec> && !std::is_floating_point_v<ScalarTypeOf<Vec>>,
    Vec> make_nonsolution_sentinel();

/** @returns normal vector of r
 *
 *  @throws if r does not have real components or is the zero vector
 *  @tparam Vec any vector type
 *  @param r must be a vector of real components
 */
template <typename Vec>
EnableIf<k_is_vector_type<Vec>, Vec>
    normalize(const Vec & r)
{ return VecOpHelpers<Vec>::template div<0>(r, magnitude(r)); }

/** @returns the projection of vector a onto vector b
 *
 *  @throws if either a or b have non real components or if b is the zero
 *          vector
 *  @tparam Vec any vector type
 *  @param a
 *  @param b
 */
template <typename Vec>
constexpr EnableIf<k_is_vector_type<Vec>, Vec>
    project_onto(const Vec & a, const Vec & b);

/** @returns the projection of vector a onto a plane described by its normal
 *
 *  @throws if either a or b have non real components or if b is the zero
 *          vector
 *  @tparam Vec any vector type
 *  @param a
 *  @param plane_normal plane's normal vector
 *         (does not actually need to be a normal vector, so long as it's
 *          orthogonal to the plane)
 */
template <typename Vec>
constexpr EnableIf<VectorTraits<Vec>::k_dimension_count == 3, Vec>
    project_onto_plane(const Vec & r, const Vec & plane_normal)
{ return VecOpHelpers<Vec>::template sub<0>(r, project_onto(r, plane_normal)); }

/** @returns A vector rounded from a floating point to an integer type.
 *
 *  This just calls "round_to" for each respective component, no regard for
 *  whether they are real numbers or not.
 */
template <typename OtherScalar, typename Vec>
EnableRoundTo<OtherScalar, Vec>
    round_to(const Vec & r);

/** Rotates vector r along the unit circle of some given number of radians rot.
 *
 *  @throws if rot or the components of r are not real numbers
 *  @tparam Vec any two dimensional vector type
 *  @param r must be a vector of real components
 *  @param rot must be a real number, in radians
 *  @return the rotated vector
 */
template <typename Vec>
EnableIf<VectorTraits<Vec>::k_dimension_count == 2, Vec>
    rotate_vector(const Vec & r, ScalarTypeOf<Vec> rot);

/** @returns the sum of squares for each component
 *  @tparam Vec any vector type
 *  @param r
 */
template <typename Vec>
constexpr EnableIf<k_is_vector_type<Vec>, ScalarTypeOf<Vec>>
    sum_of_squares(const Vec & r);

/** @}*/

// ----------------------------- Implementations ------------------------------

namespace detail {

// these need to be moved or something ;-;
template <int kt_idx, typename Vec, typename ... Types>
constexpr EnableIf<
    k_is_vector_type<Vec> && std::is_integral_v<ScalarTypeOf<Vec>>,
    Vec> round_div_
    (const Vec & l, const ScalarTypeOf<Vec> & a, Types && ... comps)
{
    using std::forward;
    constexpr const auto k_dim_count = VectorTraits<Vec>::k_dimension_count;
    if constexpr (kt_idx >= k_dim_count) {
        using Make = typename VectorTraits<Vec>::Make;
        return Make{}( forward<Types>(comps)... );
    } else {
        using Get = typename VectorTraits<Vec>::template Get<kt_idx>;
        return round_div_<kt_idx + 1>(l, a, forward<Types>(comps)...,
                                      (Get{}(l) + a / 2) / a);
    }
}

template <int kt_idx, typename Vec>
constexpr EnableIf<
    k_is_vector_type<Vec> && !std::is_integral_v<ScalarTypeOf<Vec>>,
    Vec> round_div_
    (const Vec & l, const ScalarTypeOf<Vec> & a)
{ return VecOpHelpers<Vec>::template div<kt_idx>(l, a); }

template <int kt_idx, typename Vec>
constexpr EnableIf<k_is_vector_type<Vec>, ScalarTypeOf<Vec>>
    sum_of_squares_(const Vec & r)
{
    if constexpr (kt_idx >= VectorTraits<Vec>::k_dimension_count) {
        return 0;
    } else {
        using Comp = typename VectorTraits<Vec>::template Get<kt_idx>;
        return Comp{}(r)*Comp{}(r) + sum_of_squares_<kt_idx + 1>(r);
    }
}

template <int kt_idx, typename Vec>
constexpr EnableIf<k_is_vector_type<Vec>, ScalarTypeOf<Vec>>
    dot_(const Vec & v, const Vec & u, ScalarTypeOf<Vec> acc)
{

    if constexpr (kt_idx >= VectorTraits<Vec>::k_dimension_count) {
        return acc;
    } else {
        using Comp = typename VectorTraits<Vec>::template Get<kt_idx>;
        return dot_<kt_idx + 1>(v, u, Comp{}(v)*Comp{}(u) + acc);
    }
}

template <int kt_idx, typename Vec>
constexpr EnableIf<k_is_vector_type<Vec>, bool>
    is_zero_vector_(const Vec & r)
{
    using Tr = VectorTraits<Vec>;
    if constexpr (kt_idx == Tr::k_dimension_count) {
        return true;
    } else {
        return    typename Tr::template Get<kt_idx>{}(r) == 0
               && is_zero_vector_<kt_idx + 1>(r);
    }
}

template <int kt_idx, typename Vec>
constexpr EnableIf<k_is_vector_type<Vec> &&
         std::is_floating_point_v<ScalarTypeOf<Vec>>, bool>
    is_real_(const Vec & r) noexcept
{
    using Tr = VectorTraits<Vec>;
    if constexpr (kt_idx == Tr::k_dimension_count) {
        return true;
    } else {
        constexpr const auto k_inf = std::numeric_limits<ScalarTypeOf<Vec>>::infinity();
        auto comp = typename Tr::template Get<kt_idx>{}(r);
        return    comp == comp && comp != k_inf && comp != -k_inf
               && is_real_<kt_idx + 1>(r);
    }
}

template <int kt_idx, typename OtherScalar, typename Vec, typename ... Types>
EnableRoundTo<OtherScalar, Vec>
    round_to_(const Vec & r, Types && ... comps)
{
    using std::forward;
    if constexpr (kt_idx >= VectorTraits<Vec>::k_dimension_count) {
        using OtherVector = typename VectorTraits<Vec>::template ChangeScalarType<OtherScalar>;
        using OtherMake = typename VectorTraits<OtherVector>::Make;
        return OtherMake{}( forward<Types>(comps)... );
    } else {
        using Get = typename VectorTraits<Vec>::template Get<kt_idx>;
        using std::round;
        return round_to_<kt_idx + 1, OtherScalar>
                (r, forward<Types>(comps)..., round(Get{}(r)));
    }
}

// just say 1620
template <typename Vec>
class Fraction final {
public:
    using Numeric = ScalarTypeOf<Vec>;

    constexpr Fraction(Numeric num, Numeric demon): m_num(num), m_denom(demon) {}

    constexpr Vec operator * (const Vec & r) const
        { return mul(r); }

    constexpr Vec mul(const Vec & r) const {
        return VecOpHelpers<Vec>::template div<0>(
            VecOpHelpers<Vec>::template mul<0>(r, m_num), m_denom);
    }

    constexpr bool is_within_0_or_1() const {
        return    m_num*m_denom >= 0
               && magnitude( m_num ) <= magnitude( m_denom );
    }

private:
    Numeric m_num, m_denom;
};

template <typename Vec>
class RightTriangle final {
public:
    using Numeric = ScalarTypeOf<Vec>;
    constexpr RightTriangle() {}

    constexpr RightTriangle(Numeric oppo_, Numeric adj_, Numeric hypot_):
        m_oppo(oppo_), m_adj(adj_), m_hypot_sqr(hypot_)
    {}

    constexpr static RightTriangle from_tan(Numeric oppo, Numeric adj)
        { return RightTriangle{oppo, adj, oppo*oppo + adj*adj}; }

    // but if I sine???
    // at this point become runtime because of sqrt
    constexpr Fraction<Vec> mul_sine(Numeric n) const {
        return Fraction<Vec>{ m_oppo*n, std::sqrt( m_hypot_sqr ) };
    }

    constexpr Fraction<Vec> mul_cosine(Numeric n) const {
        return Fraction<Vec>{ m_adj*n, std::sqrt( m_hypot_sqr ) };
    }

private:
    Numeric m_oppo = 0, m_adj = 0, m_hypot_sqr = 0;
};

} // end of detail namespace -> into ::cul(n)


template <typename Vec>
EnableIf<k_is_vector_type<Vec>, ScalarTypeOf<Vec>>
    angle_between(const Vec & v, const Vec & u)
{
    using namespace cul::exceptions_abbr;
    if (!is_real(v) || !is_real(u)) {
        throw InvArg("angle_between: both vectors must be real vectors.");
    }
    if (is_zero_vector(v) || is_zero_vector(u)) {
        throw InvArg("angle_between: both vectors must be non-zero.");
    }

    auto mag_v = magnitude(v);
    auto mag_u = magnitude(u);
    // this issue needs to be better understood

#   if 0
    using T = typename Vector2Scalar<Vec>::Type;
    static const constexpr T k_error = 0.00005;

    if (   are_within(mag_v / mag_u, T(0), k_error)
        || are_within(mag_u / mag_v, T(0), k_error))
    {
        // this will have to be tested with my game and other stuff
        throw InvArg("angle_between: both vectors must not be too far in magnitude.");
    }
#   endif
    static constexpr const auto k_pi = cul::k_pi_for_type<ScalarTypeOf<Vec>>;
    auto frac = dot(v, u) / (mag_v*mag_u);
    if      (frac >  1) { return 0   ; }
    else if (frac < -1) { return k_pi; }
    return std::acos(frac);
}

template <typename Vec>
constexpr EnableIf<k_is_vector_type<Vec>, bool>
    are_within(const Vec & a, const Vec & b, const ScalarTypeOf<Vec> & error)
{
    return   sum_of_squares(VecOpHelpers<Vec>::template sub<0>(a, b))
           < error*error;
}

template <typename Vec>
EnableIf<k_is_vector_type<Vec>, ScalarTypeOf<Vec>>
    area_of_triangle(const Vec & a, const Vec & b, const Vec & c)
{
    auto sub = [](const Vec & a, const Vec & b)
        { return VecOpHelpers<Vec>::template sub<0>(a, b); };
    return magnitude(cross(sub(a, b), sub(c, b))) / 2;
}

template <typename Vec>
constexpr EnableIf<VectorTraits<Vec>::k_dimension_count == 2,
         ScalarTypeOf<Vec>> cross(const Vec & v, const Vec & u)
{
    typename VectorTraits<Vec>::template Get<0> get_x;
    typename VectorTraits<Vec>::template Get<1> get_y;
    return get_x(v)*get_y(u) - get_x(u)*get_y(v);
}

template <typename Vec>
constexpr EnableIf<VectorTraits<Vec>::k_dimension_count == 3,
         Vec> cross(const Vec & v, const Vec & u)
{
    typename VectorTraits<Vec>::template Get<0> get_x;
    typename VectorTraits<Vec>::template Get<1> get_y;
    typename VectorTraits<Vec>::template Get<2> get_z;
    using Make = typename VectorTraits<Vec>::Make;
    return Make{}(
        get_y(v)*get_z(u) - get_z(v)*get_y(u),
        get_z(v)*get_x(u) - get_x(v)*get_z(u),
        get_x(v)*get_y(u) - get_y(v)*get_x(u));
}

template <typename Vec>
EnableIf<VectorTraits<Vec>::k_dimension_count == 2,
         ScalarTypeOf<Vec>>
    directed_angle_between(const Vec & from, const Vec & to)
{
    using namespace cul::exceptions_abbr;
    if (!is_real(from) || !is_real(to)) {
        throw InvArg("directed_angle_between: both vectors must be real vectors.");
    } else if (is_zero_vector(from) || is_zero_vector(to)) {
        throw InvArg("directed_angle_between: both vectors must be non-zero.");
    }

    using Tr = VectorTraits<Vec>;
    using std::atan2;
    typename Tr::template Get<0> get_x;
    typename Tr::template Get<1> get_y;
    return atan2(get_y(to), get_x(to)) - atan2(get_y(from), get_x(from));
}

template <typename Vec>
constexpr EnableIf<k_is_vector_type<Vec>, ScalarTypeOf<Vec>>
    dot(const Vec & v, const Vec & u)
{ return detail::dot_<0>(v, u, ScalarTypeOf<Vec>(0)); }

template <typename Vec>
constexpr EnableIf<k_is_vector_type<Vec>, Vec>
    find_closest_point_to_line
    (const Vec & a, const Vec & b, const Vec & external_point)
{
    auto sub = [](auto && ... args)
        { return VecOpHelpers<Vec>::template sub<0>(args...); };
    auto eq = [](auto && ... args)
        { return VecOpHelpers<Vec>::template eq<0>(args...); };
    auto mul = [](auto && ... args)
        { return VecOpHelpers<Vec>::template mul<0>(args...); };
    auto plus = [](auto && ... args)
        { return VecOpHelpers<Vec>::template plus<0>(args...); };

    const auto k_zero_v = make_zero_vector<Vec>();
    const auto & c = external_point;
    if (eq(a, b)) return a;
    if (eq(sub(a, c), k_zero_v)) return a;
    if (eq(sub(b, c), k_zero_v)) return b;

    auto has_obtuse_angle_between = [](const Vec & a, const Vec & b) {
        // derived from deconstruct angle_between
        // arccos (any negative number) > pi / 2
        // so any negative number describes an obtuse angle
        // the denominator (product of vector magnitudes) is always positive
        // the numerator maybe negative, a dot product, which is a constexpr
        // function
        return dot(a, b) < 0;
    };
    // obtuse angles -> snap to extreme points
    if (has_obtuse_angle_between(sub(a, b), sub(a, c))) return a;
    if (has_obtuse_angle_between(sub(b, a), sub(b, c))) return b;

    // https://www.eecs.umich.edu/courses/eecs380/HANDOUTS/PROJ2/LinePoint.html
    // rounds for integer division
    return plus(a, detail::round_div_<0>(
        // scalar must come second
        mul( sub(b, a), dot(sub(c, a), sub(b, a)) ),
        sum_of_squares(sub(b, a))
    ));
}

template <typename Vec>
constexpr EnableIf<VectorTraits<Vec>::k_dimension_count == 2, Vec>
    find_intersection
    (const Vec & a_first, const Vec & a_second,
     const Vec & b_first, const Vec & b_second)
{
    using Helpers = VecOpHelpers<Vec>;
    auto sub = [](auto && ... args)
        { return Helpers::template sub<0>(args...); };

    auto p = a_first;
    auto r = sub(a_second, p);

    auto q = b_first;
    auto s = sub(b_second, q);

    auto k_no_intersection = make_nonsolution_sentinel<Vec>();

    // http://stackoverflow.com/questions/563198/how-do-you-detect-where-two-line-segments-intersect
    // two points of early failure
    auto r_cross_s = cross(r, s);
    if (r_cross_s == 0) return k_no_intersection;

    auto q_sub_p = sub(q, p);
    auto t_num = cross(q_sub_p, s);
#   if 0
    auto t = cross(q_sub_p, s) / r_cross_s;

    if (t < 0 || t > 1) return k_no_intersection;
#   endif
    using Scalar = ScalarTypeOf<Vec>;
    auto outside_0_1 = [](const Scalar & num, const Scalar & denom)
        { return num*denom < 0 || magnitude(num) > magnitude(denom); };
    // overflow concerns?
    if (outside_0_1(t_num, r_cross_s)) return k_no_intersection;
#   if 0
    auto u = cross(q_sub_p, r) / r_cross_s;
    if (u < 0 || u > 1) return k_no_intersection;
#   endif

    if (outside_0_1(cross(q_sub_p, r), r_cross_s)) return k_no_intersection;
    return Helpers::template plus<0>(
        p, Helpers::template div<0>(
                Helpers::template mul<0>(r, t_num), r_cross_s
           ));
}

template <typename Vec>
EnableIf<k_is_vector_type<Vec> &&
         std::is_floating_point_v<ScalarTypeOf<Vec>>, Tuple<Vec, Vec>>
    find_velocities_to_target
    (const Vec & source, const Vec & target,
     const Vec & influencing_acceleration, ScalarTypeOf<Vec> speed)
{
    // operator assumptions: subtraction, negation, scalar multiplication
    using namespace cul::exceptions_abbr;
    using Scalar = ScalarTypeOf<Vec>;
    using Helpers = VecOpHelpers<Vec>;
    using std::make_tuple;
    if (   !is_real(source) || !is_real(speed)
        || !is_real(target) || !is_real(influencing_acceleration))
    {
        throw InvArg("find_velocities_to_target: All arguments must be real "
                     "numbers.");
    }

    static constexpr const Scalar k_error = 0.00025;
#   if 0
    auto are_very_close_v = [](const Vec & a, const Vec & b) {

        return sum_of_squares(Helpers::template sub<0>(a, b)) < k_error;
    };
#   endif
    using std::forward;
    auto sub = [](auto && ... args)
        { return Helpers::template sub<0>(forward<decltype(args)>(args)...); };
    auto plus = [](auto && ... args)
        { return Helpers::template plus<0>(forward<decltype(args)>(args)...); };
    auto mul = [](auto && ... args)
        { return Helpers::template mul<0>(forward<decltype(args)>(args)...); };
    // I believe this maybe generalizable
    // for the 3D case as it can be "simplified" onto a plane
    //
    // for 4D or higher diminsions... I'm not sure
    // how should I handle operators here? Do I rely on them being defined for
    // *all* vector types?
    // j is "up"
    auto j = Helpers::template neg<0>(normalize(influencing_acceleration));
    // this i should be good for either two or three D
    auto t_sub_s = sub(target, source);
    // i is the ground's direction
    auto i = normalize(sub( t_sub_s, project_onto(t_sub_s, j) ));

    // special cases
    const auto k_zero_v = make_zero_vector<Vec>();
    if (are_within(influencing_acceleration, k_zero_v, k_error)) {// are_very_close_v(influencing_acceleration, k_zero_v)) {
        // return straight to target
        auto s = mul(normalize(t_sub_s), speed);
        return make_tuple(s, s);
    }
    if (are_within(source, target, k_error)) {// are_very_close_v(source, target)) {
        if (are_within(influencing_acceleration, k_zero_v, k_error)) {//are_very_close_v(influencing_acceleration, k_zero_v)) {
            return make_tuple(k_zero_v, k_zero_v);
        }
        // return straight up trajectory
        auto s = mul(j, speed);
        return make_tuple(s, s);
    }

    static const auto k_inf = std::numeric_limits<Scalar>::infinity();
    static const auto k_inf_v = VecOpHelpers<Vec>::template make_from_single_scalar<0>(k_inf);
    static const auto k_no_solution = make_tuple(k_inf_v, k_inf_v);
    using std::atan, std::sqrt, std::sin, std::cos, std::make_tuple;

    Scalar t0, t1;
    {
        // assumptions at this point source != target && acc != 0
        auto comp_from_basis = [](Vec a, Vec basis) {
            auto are_parallel = [](Vec a, Vec b)
                { return are_within(normalize(a), normalize(b), k_error); };// are_very_close_v(normalize(a), normalize(b)); };
            auto proj = project_onto(a, basis);
            return magnitude(proj)*( are_parallel(proj, basis) ? 1 : -1 );
        };

        auto diff_i = magnitude(project_onto(t_sub_s, i));
        auto diff_j = comp_from_basis(t_sub_s, j);

        auto spd_sq = speed*speed;
        auto g = magnitude(influencing_acceleration);
        auto do_atan_with_sqpart = [spd_sq, g, diff_i] (Scalar sqpart)
            { return atan( (spd_sq + sqpart) / (g*diff_i) ); };

        auto randicand = spd_sq*spd_sq - g*(g*diff_i*diff_i + 2*spd_sq*diff_j);
        if (randicand < 0) return k_no_solution;
        auto sqpart = sqrt(randicand);
        t0 = do_atan_with_sqpart( sqpart);
        t1 = do_atan_with_sqpart(-sqpart);
    }

    // triangle.mul_cosine(speed)

    return make_tuple(
        plus( mul(i, cos(t0)*speed), mul(j, sin(t0)*speed) ),
        plus( mul(i, cos(t1)*speed), mul(j, sin(t1)*speed) )
    );
}

template <typename Vec>
EnableIf<k_is_vector_type<Vec> &&
         std::is_floating_point_v<ScalarTypeOf<Vec>>, Tuple<Vec, Vec>>
    find_velocities_to_target_tri
    (const Vec & source, const Vec & target,
     const Vec & influencing_acceleration, ScalarTypeOf<Vec> speed)
{
    // operator assumptions: subtraction, negation, scalar multiplication
    using namespace cul::exceptions_abbr;
    using Scalar = ScalarTypeOf<Vec>;
    using Helpers = VecOpHelpers<Vec>;
    using std::make_tuple;
    if (   !is_real(source) || !is_real(speed)
        || !is_real(target) || !is_real(influencing_acceleration))
    {
        throw InvArg("find_velocities_to_target: All arguments must be real "
                     "numbers.");
    }

    static constexpr const Scalar k_error = 0.00025;

    using std::forward;
    auto sub = [](auto && ... args)
        { return Helpers::template sub<0>(forward<decltype(args)>(args)...); };
    auto plus = [](auto && ... args)
        { return Helpers::template plus<0>(forward<decltype(args)>(args)...); };
    auto mul = [](auto && ... args)
        { return Helpers::template mul<0>(forward<decltype(args)>(args)...); };
    // I believe this maybe generalizable
    // for the 3D case as it can be "simplified" onto a plane
    //
    // for 4D or higher diminsions... I'm not sure
    // how should I handle operators here? Do I rely on them being defined for
    // *all* vector types?
    // j is "up"
    auto j = Helpers::template neg<0>(normalize(influencing_acceleration));
    // this i should be good for either two or three D
    auto t_sub_s = sub(target, source);
    // i is the ground's direction
    auto i = normalize(sub( t_sub_s, project_onto(t_sub_s, j) ));

    // special cases
    const auto k_zero_v = make_zero_vector<Vec>();
    if (are_within(influencing_acceleration, k_zero_v, k_error)) {// are_very_close_v(influencing_acceleration, k_zero_v)) {
        // return straight to target
        auto s = mul(normalize(t_sub_s), speed);
        return make_tuple(s, s);
    }
    if (are_within(source, target, k_error)) {// are_very_close_v(source, target)) {
        if (are_within(influencing_acceleration, k_zero_v, k_error)) {//are_very_close_v(influencing_acceleration, k_zero_v)) {
            return make_tuple(k_zero_v, k_zero_v);
        }
        // return straight up trajectory
        auto s = mul(j, speed);
        return make_tuple(s, s);
    }

    static const auto k_inf = std::numeric_limits<Scalar>::infinity();
    static const auto k_inf_v = VecOpHelpers<Vec>::template make_from_single_scalar<0>(k_inf);
    static const auto k_no_solution = make_tuple(k_inf_v, k_inf_v);
    using std::atan, std::sqrt, std::sin, std::cos, std::make_tuple;

    // optimize for funsies?
    using RTri = detail::RightTriangle<Vec>;
    RTri t0, t1;
    {
        // assumptions at this point source != target && acc != 0
        auto comp_from_basis = [](Vec a, Vec basis) {
            auto are_parallel = [](Vec a, Vec b)
                { return are_within(normalize(a), normalize(b), k_error); };// are_very_close_v(normalize(a), normalize(b)); };
            auto proj = project_onto(a, basis);
            return magnitude(proj)*( are_parallel(proj, basis) ? 1 : -1 );
        };

        auto diff_i = magnitude(project_onto(t_sub_s, i));
        auto diff_j = comp_from_basis(t_sub_s, j);

        auto spd_sq = speed*speed;
        auto g = magnitude(influencing_acceleration);

        auto do_atan_with_sqpart = [spd_sq, g, diff_i] (Scalar sqpart)
            { return detail::RightTriangle<Vec>::from_tan(spd_sq + sqpart, g*diff_i); };

        auto randicand = spd_sq*spd_sq - g*(g*diff_i*diff_i + 2*spd_sq*diff_j);
        if (randicand < 0) return k_no_solution;
        auto sqpart = sqrt(randicand);
        t0 = do_atan_with_sqpart( sqpart);
        t1 = do_atan_with_sqpart(-sqpart);
    }

    // triangle.mul_cosine(speed)
    return make_tuple(
        plus(t0.mul_cosine(speed).mul(i), t0.mul_sine(speed).mul(j)),
        plus(t1.mul_cosine(speed).mul(i), t1.mul_sine(speed).mul(j))
    );
}

template <typename Vec>
constexpr EnableIf<k_is_vector_type<Vec>, bool> is_inside_triangle
    (const Vec & a, const Vec & b, const Vec & c,
     const Vec & test_point)
{
    // derived from mathematics presented here:
    // https://blackpawn.com/texts/pointinpoly/default.html
    //
    // what does this mean in a 3D context? Does the proof hold up in any
    // meaningful way?
    const auto & p = test_point;
    // convert to Barycentric cordinates
    auto ca = c - a;
    auto ba = b - a;
    auto pa = p - a;

    auto dot_caca = dot(ca, ca);
    auto dot_caba = dot(ca, ba);
    auto dot_capa = dot(ca, pa);
    auto dot_baba = dot(ba, ba);
    auto dot_bapa = dot(ba, pa);

    auto denom = dot_caca*dot_baba - dot_caba*dot_caba;
    auto u = dot_baba*dot_capa - dot_caba*dot_bapa;
    auto v = dot_caca*dot_bapa - dot_caba*dot_capa;

    return u >= 0 && v >= 0 && (u + v < denom);
}

template <typename Vec>
constexpr EnableIf<k_is_vector_type<Vec> &&
         std::is_floating_point_v<ScalarTypeOf<Vec>>, bool>
    is_real(const Vec & r)
{ return detail::is_real_<0>(r); }

template <typename Vec>
constexpr EnableIf<
    k_is_vector_type<Vec>,
    bool> is_solution(const Vec & r)
{ return !VecOpHelpers<Vec>::template eq<0>(r, make_nonsolution_sentinel<Vec>()); }

template <typename Vec>
constexpr EnableIf<k_is_vector_type<Vec>, bool>
    is_zero_vector(const Vec & r)
{ return VecOpHelpers<Vec>::template eq<0>(r, make_zero_vector<Vec>());  }

template <typename Vec>
EnableIf<k_is_vector_type<Vec>, ScalarTypeOf<Vec>>
    magnitude(const Vec & r)
{ return std::sqrt( sum_of_squares(r) ); }

template <typename Vec>
constexpr EnableIf<
    k_is_vector_type<Vec> && std::is_floating_point_v<ScalarTypeOf<Vec>>,
    Vec> make_nonsolution_sentinel()
{
    return VecOpHelpers<Vec>::template make_from_single_scalar<0>
           ( std::numeric_limits<ScalarTypeOf<Vec>>::infinity() );
}

template <typename Vec>
constexpr EnableIf<
    k_is_vector_type<Vec> && !std::is_floating_point_v<ScalarTypeOf<Vec>>,
    Vec> make_nonsolution_sentinel()
{
    return VecOpHelpers<Vec>::template make_from_single_scalar<0>
           ( std::numeric_limits<ScalarTypeOf<Vec>>::min() );
}

template <typename Vec>
constexpr EnableIf<k_is_vector_type<Vec>, Vec>
    project_onto(const Vec & a, const Vec & b)
{
    using namespace cul::exceptions_abbr;
    if (!is_real(a) || !is_real(b)) {
        throw InvArg{"directed_angle_between: both vectors must be real vectors."};
    } else if (is_zero_vector(b)) {
        throw InvArg{"directed_angle_between: vector b must be non-zero."};
    }
    return VecOpHelpers<Vec>::template mul<0>
        (b, dot(a, b) / sum_of_squares(b));
}

template <typename OtherScalar, typename Vec>
EnableRoundTo<OtherScalar, Vec>
    round_to(const Vec & r)
{ return detail::round_to_<0, OtherScalar, Vec>(r); }

template <typename Vec>
EnableIf<VectorTraits<Vec>::k_dimension_count == 2, Vec>
    rotate_vector(const Vec & r, ScalarTypeOf<Vec> rot)
{
    using GetX = typename VectorTraits<Vec>::template Get<0>;
    using GetY = typename VectorTraits<Vec>::template Get<1>;
    using Make = typename VectorTraits<Vec>::Make;
    using std::sin, std::cos;
    return Make{}(GetX{}(r)*cos(rot) - GetY{}(r)*sin(rot),
                  GetX{}(r)*sin(rot) + GetY{}(r)*cos(rot));
}

template <typename Vec>
constexpr EnableIf<k_is_vector_type<Vec>, ScalarTypeOf<Vec>>
    sum_of_squares(const Vec & r)
{ return detail::sum_of_squares_<0>(r); }

} // end of cul(n) namespace

// --- Traits for SFML ---

namespace sf {

template <typename T>
class Vector2;

}

namespace culn {

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

} // end of cul(n) namespace
#endif
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace cul {

template <int kt_dimensionality, typename T, glm::qualifier Q>
struct VectorTraits<glm::vec<kt_dimensionality, T, Q>> {
    static constexpr const bool k_is_vector_type          = true;
    static constexpr const bool k_should_define_operators = false;

    using GlmVec = glm::vec<kt_dimensionality, T, Q>;
    using ScalarType = T;

    template <int kt_idx, typename = int>
    struct Get final {};

    template <typename U>
    struct Get<0, U> final {
        GLM_CONSTEXPR ScalarType operator () (const GlmVec & r) const
            { return r.x; }
    };

    template <typename U>
    struct Get<1, U> final {
        GLM_CONSTEXPR ScalarType operator () (const GlmVec & r) const
            { return r.y; }
    };

    template <typename U>
    struct Get<2, U> final {
        GLM_CONSTEXPR ScalarType operator () (const GlmVec & r) const
            { return r.z; }
    };

    template <typename U>
    struct Get<3, U> final {
        GLM_CONSTEXPR ScalarType operator () (const GlmVec & r) const
            { return r.w; }
    };

    struct Make final {
        template <typename ... Types>
        GLM_CONSTEXPR GlmVec operator () (Types && ... comps) const
            { return GlmVec{ std::forward<Types>(comps)... }; }
    };

    template <typename U>
    using ChangeScalarType = glm::vec<kt_dimensionality, U, Q>;

    static constexpr const int k_dimension_count = kt_dimensionality;
};

} // end of cul(n) namespace

struct UserVector final {
    double x = 0, y = 0;
};


struct UserVector3 final {
    double x = 0, y = 0, z = 0;
};

namespace cul {

template <>
struct VectorTraits<UserVector> {
    static constexpr const bool k_is_vector_type          = true;
    static constexpr const bool k_should_define_operators = false;

    using ScalarType = double;

    template <int kt_idx, typename = int>
    struct Get final {};

    template <typename U>
    struct Get<0, U> final {
        ScalarType operator () (const UserVector & r) const
            { return r.x; }
    };

    template <typename U>
    struct Get<1, U> final {
        ScalarType operator () (const UserVector & r) const
            { return r.y; }
    };

    struct Make final {
        UserVector operator () (const double & x, const double & y) const
            { return UserVector{x, y}; }
    };
#   if 0
    template <typename U>
    using ChangeScalarType = ; // not defined...
#   endif
    static constexpr const int k_dimension_count = 2;
};

template <>
struct VectorTraits<UserVector3> {
    static constexpr const bool k_is_vector_type          = true;
    static constexpr const bool k_should_define_operators = false;

    using ScalarType = double;

    template <int kt_idx, typename = int>
    struct Get final {};

    template <typename U>
    struct Get<0, U> final {
        ScalarType operator () (const UserVector3 & r) const
            { return r.x; }
    };

    template <typename U>
    struct Get<1, U> final {
        ScalarType operator () (const UserVector3 & r) const
            { return r.y; }
    };

    template <typename U>
    struct Get<2, U> final {
        ScalarType operator () (const UserVector3 & r) const
            { return r.z; }
    };

    struct Make final {
        UserVector3 operator () (const double & x, const double & y, const double & z) const
            { return UserVector3{x, y, z}; }
    };
#   if 0
    template <typename U>
    using ChangeScalarType = ; // not defined...
#   endif
    static constexpr const int k_dimension_count = 3;
};

} // end of cul(n) namespace


// naughty, naughty...
#include <iostream>
#include <SFML/System/Vector2.hpp>

#include <cassert>

constexpr const auto k_pi_f = cul::k_pi_for_type<float>;
constexpr const auto k_inf = std::numeric_limits<double>::infinity();

int main() {

    using Vec2I = cul::Vector2<int>;
    using Vec2D = cul::Vector2<double>;
    using Vec3I = cul::Vector3<int>;
    using SfVec2F = sf::Vector2f;
    using GlmVec3 = glm::vec3;
    using GlmVec4 = glm::vec4;
    using GlmVec3I = cul::VectorTraits<glm::vec3>::ChangeScalarType<int>;
    constexpr double k_error = 0.00005;

    // operators for builtin type

    static_assert(-Vec2I{1, 2} == Vec2I{-1, -2});
    static_assert(Vec2I{1, 2} + Vec2I{1, 3} == Vec2I{2, 5});
    static_assert(Vec3I{ 3, 4, 5 } - Vec3I{ 2, 2, -1 } == Vec3I{1, 2, 6});
    static_assert(-Vec3I{1, -2, 3} == Vec3I{-1, 2, -3});
    static_assert(Vec3I{ 1, 2, 3 }*2 == Vec3I{2, 4, 6});
    static_assert(Vec3I{ 4, 8, 16 }/2 == Vec3I{2, 4, 8});
    static_assert(2*Vec2I{ 4, -2 } == Vec2I{8, -4});

    using namespace cul;
    static_assert(project_onto( Vec2I{ 3, 4 }, Vec2I{ -3, 0 } ) == Vec2I{3, 0});
    auto res = magnitude( convert_to<sf::Vector2i>(Vec2I{ 3, 4 }) );
    std::cout << "mag of vector <3, 4> = " << res << std::endl;
    {
        auto v = cul::round_to<int>(sf::Vector2f{ 1.2, 5.5 });
        static_assert(std::is_same_v<decltype(v), sf::Vector2i>);
        assert(([v] { return v == sf::Vector2i{1, 6}; } ()));
    }
    cul::find_velocities_to_target(Vec2D{ 1, 1 }, Vec2D{ 11, 5}, Vec2D{ 0, -8 }, 12);
    cul::find_velocities_to_target(SfVec2F{ 1, 1 }, SfVec2F{ 11, 5}, SfVec2F{ 0, -8 }, 12);

    // ---

    // angle_between

    assert(magnitude( angle_between(GlmVec3{-1, 0, 3}, GlmVec3{3, 0, 1}) - k_pi_f*0.5) < k_error);
    std::cout << angle_between(UserVector{-1, -2}, UserVector{1, 2}) << std::endl;
    assert(magnitude( angle_between(UserVector{-1, -2}, UserVector{1, 2}) - k_pi_f) < k_error);
    static_assert(std::is_same_v<decltype(angle_between(UserVector{-1, -2}, UserVector{1, 2})), double>);

    // are_within
    static_assert(are_within( Vec2I{1, 2}, Vec2I{1, 3}, 2));
    assert(are_within( UserVector{1, 2}, UserVector{1, 3}, 2));

    // area_of_triangle
    std::cout << area_of_triangle(Vec2I{0, 0}, Vec2I{10, 10}, Vec2I{10, 0}) << std::endl;
    std::cout << area_of_triangle(Vec3I{0, 0, 0}, Vec3I{10, 10, 0}, Vec3I{10, 0, 0}) << std::endl;
    assert(area_of_triangle(Vec2I{0, 0}, Vec2I{10, 10}, Vec2I{10, 0}) == 50);
    assert(area_of_triangle(UserVector{0, 0}, UserVector{10, 10}, UserVector{10, 0}) == 50);

    // convert_to
    // conversions should work to and from other library types
    static_assert(convert_to<Vec3I>( GlmVec3{ 10.4, 8.7, 9.4 } ) == Vec3I{10, 8, 9});
    assert(magnitude(convert_to<SfVec2F>( Vec2I{9, 8} ) - SfVec2F{9, 8}) < k_error);

    // cross (2D)
    static_assert(cross(Vec2I{ -1, 0 }, Vec2I{ 0, 1 }) == -1);
    assert(cross(UserVector{-1, 0}, UserVector{0, 1}) == -1);

    // cross (3D)
    {
    constexpr auto cpr = cross(GlmVec3{ -1, 0, 0 }, GlmVec3{ 0, 1, 0 });
    static_assert(cpr.z < -.9 && cpr.z > -1.1);
    }
    assert(magnitude( cross(UserVector3{ -1, 0, 0 }, UserVector3{ 0, 1, 0 }).z + 1) < k_error);

    // directed_angle_between (2D context only)
    {
    auto da = directed_angle_between( Vec2D{1, 0}, Vec2D{0, 1} );
    // I'm not sure what "convention" I'm using here...
    assert(magnitude( da - k_pi_f*0.5 ) < k_error );
    // must work with "rotate_vector"
    assert(magnitude( rotate_vector(Vec2D{1, 0}, da) - Vec2D{0, 1} ) < k_error);
    assert(directed_angle_between(UserVector{1, 0}, UserVector{0, -1}) < 0);
    }

    // dot
    {
    constexpr auto dpr = dot( GlmVec4{0, 1, 2, 3}, GlmVec4{3, 2, 1, 0} );
    static_assert(dpr < 4.1 && dpr > 3.9);
    auto dpr2 = dot(UserVector{1, 2}, UserVector{2, 3});
    assert(magnitude(dpr2 - 8) < k_error);
    }

    // find_closest_point_to_line
    {
    constexpr auto r = find_closest_point_to_line(Vec2I{0, 0}, Vec2I{6, 3}, Vec2I{5, 4});
    std::cout << "Closest point from <5, 4> to <0, 0> - <6, 3> = " << r.x << ", " << r.y << std::endl;
    }
    {
    auto res = find_closest_point_to_line(UserVector3{0, 0, 0}, UserVector3{10, 10, 10}, UserVector3{5, 5, 6});
    std::cout << "(" << res.x << ", " << res.y << ", " << res.z << ")" << std::endl;
    }
    // find_intersection
    {
    constexpr auto res = find_intersection(Vec2I{0, 0}, Vec2I{10, 10}, Vec2I{0, 10}, Vec2I{10, 0});
    static_assert(res == Vec2I{5, 5});
    std::cout << "intx: " << res.x << ", " << res.y << std::endl;
    }
    find_intersection(UserVector{0, 0}, UserVector{10, 10}, UserVector{0, 10}, UserVector{10, 0});
#   if 1
    // find_velocities_to_target
    {
    auto [s0, s1] = find_velocities_to_target(UserVector{0, 0}, UserVector{10, 2}, UserVector{0, -10}, 12.);
    std::cout << "vel to target: (" << s0.x << ", " << s0.y
              << "), ("             << s1.x << ", " << s1.y
              << ")" << std::endl;
    }
#   endif
#   if 0
    {
    auto [s0, s1] = find_velocities_to_target(Vec2I{0, 0}, Vec2I{10, 2}, Vec2I{0, -10}, 12);
    std::cout << "vel to target: (" << s0.x << ", " << s0.y
              << "), ("             << s1.x << ", " << s1.y
              << ")" << std::endl;
    }
    // now in 3D
    {
    auto [s0, s1] = find_velocities_to_target(Vec2I{0, 0}, Vec2I{10, 2}, Vec2I{0, -10}, 12);
    std::cout << "vel to target: (" << s0.x << ", " << s0.y
              << "), ("             << s1.x << ", " << s1.y
              << ")" << std::endl;
    }
#   endif
    // is_inside_triangle
    static_assert( is_inside_triangle(Vec2I{0, 0}, Vec2I{6, 0}, Vec2I{3, 3}, Vec2I{3,  1}));
    static_assert(!is_inside_triangle(Vec2I{0, 0}, Vec2I{6, 0}, Vec2I{3, 3}, Vec2I{3, -1}));
    // is_real

    static_assert(is_real(Vec2I{1, 2}));
    static_assert(!is_real(Vec2D{1, k_inf}));
    assert(is_real(UserVector{1., 2.}));
    assert(!is_real(UserVector3{1., k_inf, 2.}));
#   if 1
    // is_solution
    assert(!is_solution(std::get<0>(
        find_velocities_to_target(Vec2D{0, 0}, Vec2D{100, 100}, Vec2D{0, -10}, 2.)
    )));
    assert(!is_solution(std::get<0>(
        find_velocities_to_target(UserVector{0, 0}, UserVector{100, 100}, UserVector{0, -10}, 2.)
    )));
#   endif
    // is_zero_vector

    assert(is_zero_vector(Vec2I{}));
    assert(!is_zero_vector(GlmVec3{0, 0, 1}));
    assert(is_zero_vector(UserVector{ 0, 0 }));
    // magnitude
    assert(magnitude(Vec2I{ 3, 4 }) == 5);
    magnitude(GlmVec3{ 5, 4, 6 });

    // make_zero_vector
    static_assert(make_zero_vector<GlmVec3>().z == 0);
    assert(make_zero_vector<UserVector3>().z == 0);

    // make_nonsolution_sentinel (both integer and floating point)
    static_assert(!is_solution(make_nonsolution_sentinel<Vec2I>()));
    static_assert(!is_solution(make_nonsolution_sentinel<GlmVec3>()));
    assert(!is_solution(make_nonsolution_sentinel<UserVector>()));

    // normalize
    assert(magnitude( magnitude( normalize(GlmVec4{ 5, 4, 2, 1}) ) - 1 ) < k_error);

    // project_onto
    {
    auto res = project_onto(UserVector{ 1, 3 }, UserVector{ -1, 0 });
    assert(magnitude(res.x - 1) < k_error && magnitude(res.y) < k_error);
    }
    // project_onto_plane
    {
    auto res = project_onto_plane(UserVector3{ 1, 1, 1 }, UserVector3{ 0, -1, 0 });
    assert(res.x > 0 && res.z > 0 && res.y < k_error);
    }
    // round_to
    assert((round_to<int>( Vec2D{ 2.3, 4.5 } ) == Vec2I{2, 5}));
    static_assert(std::is_same_v<decltype(round_to<int>(GlmVec3{})), GlmVec3I>);

    // rotate_vector
    // test library stated interface
    {
        using RealDistri = std::uniform_real_distribution<double>;
        using IntDistri = std::uniform_int_distribution<int>;
        auto rndg = std::default_random_engine{0xDEADBEEF};
        auto rndv = [&rndg] {
            auto x = RealDistri{-5, 5}(rndg);
            double y = magnitude(x) < k_error ?
                       RealDistri{1, 5}(rndg)*( IntDistri{0, 1}(rndg) ? 1 : -1 ) :
                       RealDistri{-5, 5}(rndg);
            return Vec2D{x, y};
        };
        for (int i = 0; i != 100; ++i) {
            auto from = rndv();
            auto to   = rndv();
            auto t      = directed_angle_between(from, to);
            auto new_to = rotate_vector(from, t);
            assert(angle_between(new_to, to) < k_error);
        }
    }

    // sum_of_squares
    static_assert(sum_of_squares(Vec2I{ 3, 4 }) == 25);
    static_assert(sum_of_squares(GlmVec3I{ 3, 4, 5 }) == 50);
    assert(magnitude(sum_of_squares(UserVector{3, 4}) - 25) < k_error);
}
