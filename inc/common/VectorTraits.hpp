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

#include <type_traits>
#include <stdexcept>

#include <common/Util.hpp>

namespace cul {

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

template <typename VectorType>
using ScalarTypeOf = typename VectorTraits<VectorType>::ScalarType;

template <typename VectorType, typename T>
using EnableVector = EnableIf<
    k_should_define_vector_operators<VectorType>, T>;

template <typename VectorType>
using EnableVectorOp = EnableIf<
    k_should_define_vector_operators<VectorType>, VectorType>;

// ---------------------------- Unary Vector Operator --------------------------

template <typename VectorType>
constexpr EnableVectorOp<VectorType>
    operator - (const VectorType & r);

// -------------------------- Vector on Vector operators -----------------------

template <typename VectorType>
constexpr EnableVectorOp<VectorType>
    operator + (const VectorType & lhs, const VectorType & rhs);

template <typename VectorType>
constexpr EnableVectorOp<VectorType>
    operator - (const VectorType & lhs, const VectorType & rhs);

template <typename VectorType>
constexpr EnableVector<VectorType, bool> operator ==
    (const VectorType & lhs, const VectorType & rhs);

template <typename VectorType>
constexpr EnableVector<VectorType, bool> operator !=
    (const VectorType & lhs, const VectorType & rhs);

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
    operator * (const VectorType & r, const ScalarTypeOf<VectorType> & a);

template <typename VectorType>
constexpr EnableVector<VectorType, VectorType>
    operator / (const VectorType & r, const ScalarTypeOf<VectorType> & a);

template <typename VectorType>
constexpr EnableVector<VectorType, VectorType>
    operator * (const ScalarTypeOf<VectorType> & a, const VectorType & r);

// -------------------------- Implementation details --------------------------

#ifndef DOXYGEN_SHOULD_SKIP_THIS

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

// -------------------------- Vector on Scalar operators -----------------------

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

#endif // DOXYGEN_SHOULD_SKIP_THIS

} // end of cul namespace
