/****************************************************************************

    MIT License

    Copyright (c) 2023 Aria Janke

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

#include <ariajanke/cul/FunctionTraits.hpp>

#include <optional>
#include <variant>
#include <stdexcept>

namespace cul {

template <typename LeftT, typename RightT>
class Either;

class EitherBase {
protected:
    template <typename Func>
    using ReturnTypeOf = typename FunctionTraitsOf<Func>::ReturnType;

    template <typename Func>
    using StrippedArgumentTypesOf = typename FunctionTraitsOf<Func>::
        ArgumentTypes::
        template Transform<std::remove_reference_t>::
        template Transform<std::remove_cv_t>;

    template <typename AnyT>
    struct IsEither final {
        static constexpr const bool k_value = false;
    };

    template <typename LeftT, typename RightT>
    struct IsEither<Either<LeftT, RightT>> final {
        static constexpr const bool k_value = true;
        using LeftType = LeftT;
        using RightType = RightT;
    };

    class LeftTag final {};
    class RightTag final {};

    template <typename ... Types>
    static constexpr const bool kt_is_either = IsEither<Types...>::k_value;

    template <typename Type>
    static constexpr const bool kt_is_void = std::is_same_v<void, Type>;
};

template <typename LeftT, typename RightT>
class Either final : public EitherBase {
public:
    using LeftType = LeftT;
    using RightType = RightT;
    template <typename T>
    using EnableIfReturnsEither =
        std::enable_if_t<kt_is_either<ReturnTypeOf<T>>, ReturnTypeOf<T>>;

    static_assert(!kt_is_void<LeftType> , "LeftType may not be void" );
    static_assert(!kt_is_void<RightType>, "RightType may not be void");

    template <typename RightType, typename LeftType>
    static constexpr Either<LeftType, RightType> left(LeftType && left_)
        { return Either<LeftType, RightType>{std::move(left_), LeftTag{}}; }

    template <typename LeftType, typename RightType>
    static constexpr Either<LeftType, RightType> right(RightType && right_)
        { return Either<LeftType, RightType>{std::move(right_), RightTag{}}; }

    template <typename CommonT>
    class Fold final {
    public:
        using CommonType = CommonT;

        constexpr explicit Fold(Either<LeftT, RightT> && either):
            m_ei(std::move(either)) {}

        template <typename Func>
        constexpr Fold<CommonType> map(Func && f) {
            using Rt = ReturnTypeOf<Func>;
            using ArgumentTypes = StrippedArgumentTypesOf<Func>;
            static_assert(std::is_same_v<Rt, CommonT>,
                          "given function must return common type");
            static_assert(ArgumentTypes::k_count == 1,
                          "Given function must accept exactly one argument");
            static_assert(ArgumentTypes::template kt_occurance_count<RightType> == 1,
                          "Given function must accept some form of RightType");
            if (m_ei.is_right()) {
                OptionalCommon rv = f(m_ei.right());
                return Fold{std::move(rv), std::move(m_ei)};
            }
            return Fold{std::move(m_rv), std::move(m_ei)};
        }

        template <typename Func>
        constexpr Fold<CommonType> map_left(Func && f) {
            using Rt = ReturnTypeOf<Func>;
            using ArgumentTypes = StrippedArgumentTypesOf<Func>;
            static_assert(std::is_same_v<Rt, CommonT>,
                          "given function must return common type");
            static_assert(ArgumentTypes::k_count == 1,
                          "Given function must accept exactly one argument");
            static_assert(ArgumentTypes::template kt_occurance_count<LeftType> == 1,
                          "Given function must accept some form of LeftType");
            if (m_ei.is_left()) {
                OptionalCommon rv = f(m_ei.left());
                return Fold{std::move(rv), std::move(m_ei)};
            }
            return Fold{std::move(m_rv), std::move(m_ei)};
        }

        constexpr CommonType operator () () const
            { return m_rv.value(); }

    private:
        using OptionalCommon = std::optional<CommonT>;

        constexpr Fold(OptionalCommon && rv, Either<LeftT, RightT> && either):
             m_rv(std::move(rv)),
             m_ei(std::move(either))
        {}

        OptionalCommon m_rv;
        Either<LeftT, RightT> m_ei;
    };

    template <typename EitherLeftOrRight>
    constexpr Either(EitherLeftOrRight && obj,
                     typename std::enable_if<
                        std::is_same_v<EitherLeftOrRight, LeftType> ^
                        std::is_same_v<EitherLeftOrRight, RightType>
                        >::type * = nullptr):
        m_datum(std::move(obj)) {}


    constexpr Either(LeftType && obj, LeftTag):
        m_datum(std::in_place_index_t<0>{}, std::move(obj)) {}

    constexpr Either(RightType && obj, RightTag):
        m_datum(std::in_place_index_t<1>{}, std::move(obj)) {}


    template <typename Func>
    [[nodiscard]] constexpr Either<LeftType, ReturnTypeOf<Func>> map(Func && f) {
        using NewRight = ReturnTypeOf<Func>;
        using ArgumentTypes = StrippedArgumentTypesOf<Func>;
        static_assert(!std::is_same_v<void, NewRight>,
                      "Return type of given function may not be void");
        static_assert(ArgumentTypes::k_count == 1,
                      "Given function must accept exactly one argument");
        static_assert(ArgumentTypes::template kt_occurance_count<RightType> == 1,
                      "Given function must accept some form of RightType");
        if (is_left())
            { return left<NewRight>(left()); }
        return right<LeftType>(f(right()));
    }

    template <typename Func>
    [[nodiscard]] constexpr Either<ReturnTypeOf<Func>, RightType> map_left(Func && f) {
        using NewLeft = ReturnTypeOf<Func>;
        using ArgumentTypes = StrippedArgumentTypesOf<Func>;
        static_assert(!std::is_same_v<void, NewLeft>,
                      "Return type of given function may not be void");
        static_assert(ArgumentTypes::k_count == 1,
                      "Given function must accept exactly one argument");
        static_assert(ArgumentTypes::template kt_occurance_count<LeftType> == 1,
                      "Given function must accept some form of LeftType");
        if (is_right())
            { return right<NewLeft>(right()); }
        return left<RightType>(f(left()));
    }

    template <typename CommonT>
    [[nodiscard]] constexpr Fold<CommonT> fold()
        { return Fold<CommonT>{std::move(*this)}; }

    template <typename Func>
    [[nodiscard]] constexpr EnableIfReturnsEither<Func> chain(Func && f) {
        using FuncReturnType = ReturnTypeOf<Func>;
        using NewLeftType = typename IsEither<FuncReturnType>::LeftType;
        using NewRightType = typename IsEither<FuncReturnType>::RightType;
        using ArgumentTypes = StrippedArgumentTypesOf<Func>;
        static_assert(std::is_same_v<NewLeftType, LeftType>,
            "Given function must preserve left type");
        static_assert(ArgumentTypes::k_count == 1,
                      "Given function must accept exactly one argument");
        static_assert(ArgumentTypes::template kt_occurance_count<RightType> == 1,
                      "Given function must accept some form of RightType");
        if (is_left())
            { return left<NewRightType>(left()); }
        return f(right());
    }

    template <typename Func>
    [[nodiscard]] constexpr EnableIfReturnsEither<Func> chain_left(Func && f) {
        using FuncReturnType = ReturnTypeOf<Func>;
        using NewLeftType = typename IsEither<FuncReturnType>::LeftType;
        using NewRightType = typename IsEither<FuncReturnType>::RightType;
        using ArgumentTypes = StrippedArgumentTypesOf<Func>;
        static_assert(std::is_same_v<NewRightType, RightType>,
            "Given function must preserve right type");
        static_assert(ArgumentTypes::k_count == 1,
                      "Given function must accept exactly one argument");
        static_assert(ArgumentTypes::template kt_occurance_count<LeftType> == 1,
                      "Given function must accept some form of LeftType");
        if (is_right())
            { return right<NewLeftType>(right()); }
        return f(left());
    }

    constexpr LeftT left()
        { return move_out_value<0>(); }

    constexpr RightT right()
        { return move_out_value<1>(); }

    constexpr bool is_left() const
        { return m_datum.index() == 0; }

    constexpr bool is_right() const
        { return m_datum.index() == 1; }

private:

    using DatumVariant = std::variant<LeftT, RightT, std::monostate>;

    constexpr Either(DatumVariant && either_datum_):
        m_datum(std::move(either_datum_)) {}

    template <int kt_index>
    constexpr auto move_out_value() {
        auto rv = std::move(std::get<kt_index>(m_datum));
        m_datum = std::monostate{};
        return rv;
    }

    DatumVariant m_datum;
};

template <typename LeftType, typename RightType>
Either<LeftType, RightType> make_right(RightType && obj)
    { return Either<LeftType, RightType>{std::move(obj)}; }

template <typename RightType, typename LeftType>
Either<LeftType, RightType> make_left(LeftType && obj)
    { return Either<LeftType, RightType>{std::move(obj)}; }
#if 0
// ----------------------------------------------------------------------------

template <typename Func>
Fold<CommonType> map(Func && f) {
    using Rt = ReturnTypeOf<Func>;
    static_assert(std::is_same_v<Rt, CommonT>,
                  "given function must return common type");
    OptionalCommon rv;
    auto ei = m_ei.map([&rv, f = std::move(f)]
         (RightType && right)
    { return *(rv = OptionalCommon{f(std::move(right))}); });
    return Fold{std::move(rv), std::move(ei)};
}

template <typename Func>
Fold<CommonType> map_left(Func && f) {
    using Rt = ReturnTypeOf<Func>;
    static_assert(std::is_same_v<Rt, CommonT>,
                  "given function must return common type");
    OptionalCommon rv;
    auto ei = m_ei.map_left([&rv, f = std::move(f)]
        (LeftType && left)
    { return *(rv = OptionalCommon{f(std::move(left))}); });
    return Fold{std::move(rv), std::move(m_ei)};
}

#endif
} // end of cul namespace
