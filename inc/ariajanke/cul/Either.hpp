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

    static_assert(!kt_is_void<LeftType> , "LeftType may not be void" );
    static_assert(!kt_is_void<RightType>, "RightType may not be void");

    template <typename RightType, typename LeftType>
    static Either<LeftType, RightType> left(LeftType && left_)
        { return Either<LeftType, RightType>{std::move(left_)}; }

    template <typename LeftType, typename RightType>
    static Either<LeftType, RightType> right(RightType && right_)
        { return Either<LeftType, RightType>{std::move(right_)}; }

    template <typename CommonT>
    class Fold final {
    public:
        using CommonType = CommonT;

        explicit Fold(Either<LeftT, RightT> && either):
            m_ei(std::move(either)) {}

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

        CommonType operator () () const {
            return m_rv.value();
        }

    private:
        using OptionalCommon = std::optional<CommonT>;

        Fold(OptionalCommon && rv, Either<LeftT, RightT> && either):
             m_rv(std::move(rv)),
             m_ei(std::move(either))
        {}

        OptionalCommon m_rv;
        Either<LeftT, RightT> m_ei;
    };

    Either(LeftType && left_):
        m_datum(std::move(left_)) {}

    Either(RightType && right_):
        m_datum(std::move(right_)) {}

    template <typename Func>
    [[nodiscard]] Either<LeftType, ReturnTypeOf<Func>> map(Func && f) {
        using NewRight = ReturnTypeOf<Func>;
        using ArgumentTypes = StrippedArgumentTypesOf<Func>;
        static_assert(!std::is_same_v<void, NewRight>,
                      "Return type of given function may not be void");
        static_assert(ArgumentTypes::k_count == 1,
                      "Given function must accept exactly one argument");
        static_assert(ArgumentTypes::template kt_occurance_count<RightType> == 1,
                      "Given function must accept some form of RightType");
        if (is_left())
            { return left<NewRight>(move_out_left()); }
        return right<LeftType>(f(move_out_right()));
    }

    template <typename Func>
    [[nodiscard]] Either<ReturnTypeOf<Func>, RightType> map_left(Func && f) {
        using NewLeft = ReturnTypeOf<Func>;
        using ArgumentTypes = StrippedArgumentTypesOf<Func>;
        static_assert(!std::is_same_v<void, NewLeft>,
                      "Return type of given function may not be void");
        static_assert(ArgumentTypes::k_count == 1,
                      "Given function must accept exactly one argument");
        static_assert(ArgumentTypes::template kt_occurance_count<LeftType> == 1,
                      "Given function must accept some form of LeftType");
        if (is_right())
            { return right<NewLeft>(move_out_right()); }
        return left<RightType>(f(move_out_left()));
    }

    template <typename CommonT>
    [[nodiscard]] Fold<CommonT> fold()
        { return Fold<CommonT>{std::move(*this)}; }

    template <typename Func>
    [[nodiscard]] ReturnTypeOf<Func> chain
        (std::enable_if_t<kt_is_either<ReturnTypeOf<Func>>, Func> && f)
    {
        using FuncReturnType = ReturnTypeOf<Func>;
        using NewLeftType = typename IsEither<FuncReturnType>::LeftType;
        using NewRightType = typename IsEither<FuncReturnType>::RightType;
        static_assert(std::is_same_v<NewLeftType, LeftType>,
            "Given function must preserve left type");
        if (is_left())
            { return left<NewRightType>(move_out_left()); }
        return f(move_out_right());
    }

    template <typename Func>
    [[nodiscard]] ReturnTypeOf<Func> chain_left
        (std::enable_if_t<kt_is_either<ReturnTypeOf<Func>>, Func> && f)
    {
        using FuncReturnType = ReturnTypeOf<Func>;
        using NewLeftType = typename IsEither<FuncReturnType>::LeftType;
        using NewRightType = typename IsEither<FuncReturnType>::RightType;
        static_assert(std::is_same_v<NewRightType, RightType>,
            "Given function must preserve right type");
        if (is_right())
            { return right<NewLeftType>(move_out_right()); }
        return f(move_out_left());
    }

private:
    using DatumVariant = std::variant<LeftT, RightT, std::monostate>;

    Either(DatumVariant && either_datum_):
        m_datum(std::move(either_datum_)) {}

    LeftT move_out_left()
        { return move_out_value<LeftT>(); }

    RightT move_out_right()
        { return move_out_value<RightT>(); }

    bool is_left() const
        { return std::holds_alternative<LeftType>(m_datum); }

    bool is_right() const
        { return std::holds_alternative<RightType>(m_datum); }

    template <typename T>
    T move_out_value() {
        if (std::holds_alternative<T>(m_datum)) {
            auto rv = std::move(std::get<T>(m_datum));
            m_datum = std::monostate{};
            return rv;
        }
        // must cover incorrect uses somehow...
        throw std::runtime_error("Cannot use a consumed Either");
    }

    DatumVariant m_datum;
};

} // end of cul namespace
