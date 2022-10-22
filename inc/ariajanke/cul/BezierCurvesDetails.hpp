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

#include <ariajanke/cul/VectorTraits.hpp>

#include <tuple>

namespace cul {

namespace detail {

#ifndef DOXYGEN_SHOULD_SKIP_THIS

template <typename Vec, typename ... Types>
struct AreVectorTypes final {
    static constexpr const bool k_value = k_is_vector_type<Vec>;
};

template <typename Vec, typename Other, typename ... Types>
struct AreVectorTypes<Vec, Other, Types...> final {
    static constexpr const bool k_value =
           k_is_vector_type<Vec>
        && AreVectorTypes<Other, Types...>::k_value;
};

template <typename Vec, typename ... Types>
struct AreArithmeticTypes final {
    static constexpr const bool k_value = std::is_arithmetic_v<Vec>;
};

template <typename Vec, typename Other, typename ... Types>
struct AreArithmeticTypes<Vec, Other, Types...> final {
    static constexpr const bool k_value =
           std::is_arithmetic_v<Vec>
        && AreArithmeticTypes<Other, Types...>::k_value;
};

template <typename ... Types>
class TransformTypes_ {
    static_assert(sizeof...(Types) == 0, "Should be zero at primary.");


    template <typename T>
    struct WithoutType_ {
        using Set = TransformTypes_<>;
    };

    template <typename T>
    struct SetWithHead_ {
        using Set = TransformTypes_<T>;
    };


public:
    using Tail = TransformTypes_<>;

    template <template <typename> class Transformer>
    using Transform = TransformTypes_<>;

    template <typename T>
    using SetWithHead = typename SetWithHead_<T>::Set;
};

// no ordering of types can be done so many operations become O(n^2)
template <typename Head, typename ... Types>
class TransformTypes_<Head, Types...> {
    using Tail_ = TransformTypes_<Types...>;

    template <typename T>
    struct SetWithHead_ {
        using Set = TransformTypes_<T, Head, Types...>;
    };

    template <template <typename ...> class Other>
    struct DefineWithTypes_ {
        using Type = Other<Head, Types...>;
    };

public:

    template <typename T>
    using SetWithHead = typename SetWithHead_<T>::Set;

    template <template <typename> class Transformer>
    using Transform = typename Tail_::template Transform<Transformer>
        ::template SetWithHead<Transformer<Head>>;

    template <template <typename ...> class Other>
    using DefineWithTypes = typename DefineWithTypes_<Other>::Type;
};

template <typename ... Types>
struct ScalarsTuple {};

template <typename ... Types>
struct ScalarsTuple<Tuple<Types...>> {
    // no Scalar
    using Type = typename TransformTypes_<Types...>
        ::template Transform<ScalarTypeOf>
        ::template DefineWithTypes<Tuple>;
};

template <typename ... Types>
constexpr const bool k_are_arithmetic = AreArithmeticTypes<Types...>::k_value;

template <typename ... Types>
constexpr const bool k_are_vector_types = AreVectorTypes<Types...>::k_value;

class BezierEndIterator final {};

template <typename Vec, typename ... TupleTypes>
class BezierIterator;

template <typename Vec, typename ... TupleTypes>
class BezierLineIterator;

template <typename Vec, typename ... TupleTypes>
class BezierStripTrianglesIterator;

template <typename Vec, typename ... TupleTypes>
class BezierStripDetailedIterator;

} // end of details into -> ::cul

#endif // DOXYGEN_SHOULD_SKIP_THIS

} // end of cul namespace
