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

#include <ariajanke/cul/OptionalEither.hpp>

namespace cul {
#if 0
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

    class FoldBase {
    protected:
        FoldBase() {}

        template <typename CommonT, typename Func, typename SideType>
        struct VerifyFoldFunctionForSide final {
            using ReturnType = ReturnTypeOf<Func>;
            using ArgumentTypes = StrippedArgumentTypesOf<Func>;
            static_assert(std::is_same_v<ReturnType, CommonT>,
                          "given function must return common type");
            static_assert(ArgumentTypes::k_count == 1,
                          "Given function must accept exactly one argument");
            static_assert
                (ArgumentTypes::template kt_occurance_count<SideType> == 1,
                 "Given function must accept some form of the side's type");
        };
    };

    template <typename Func, typename SideType>
    struct VerifyArgumentsForSide final {
        using ArgumentTypes = StrippedArgumentTypesOf<Func>;
        static_assert(ArgumentTypes::k_count == 1,
                      "Given function must accept exactly one argument");
        static_assert
            (ArgumentTypes::template kt_occurance_count<SideType> == 1,
             "Given function must accept some form of the side's type");
    };

    template <typename T>
    using EnableIfReturnsEither =
        std::enable_if_t<kt_is_either<ReturnTypeOf<T>>, ReturnTypeOf<T>>;

};

template <typename LeftT, typename RightT>
class Either final : public EitherBase {
public:
    using LeftType = LeftT;
    using RightType = RightT;

private:
    template <typename EitherLeftOrRight>
    using EnableIfLeftXorRightPtr = typename std::enable_if_t<
        bool(std::is_same_v<EitherLeftOrRight, LeftType > ^
             std::is_same_v<EitherLeftOrRight, RightType>),
        void *>;

public:
    static_assert(!kt_is_void<LeftType> , "LeftType may not be void" );
    static_assert(!kt_is_void<RightType>, "RightType may not be void");

    template <typename CommonT>
    class Fold final : public FoldBase {
    public:
        using CommonType = CommonT;

        constexpr explicit Fold(Either<LeftT, RightT> && either):
            m_ei(std::move(either)) {}

        template <typename Func>
        constexpr Fold<CommonType> map(Func && f);

        template <typename Func>
        constexpr Fold<CommonType> map_left(Func && f);

        constexpr CommonType operator () () const
            { return m_rv.value(); }

    private:
        using OptionalCommon = std::optional<CommonT>;

        constexpr Fold(OptionalCommon && rv, Either<LeftT, RightT> && either);

        OptionalCommon m_rv;
        Either<LeftT, RightT> m_ei;
    };

    template <typename RightType, typename LeftType>
    static constexpr Either<LeftType, RightType> left(LeftType && left_)
        { return Either<LeftType, RightType>{std::move(left_), LeftTag{}}; }

    template <typename LeftType, typename RightType>
    static constexpr Either<LeftType, RightType> right(RightType && right_)
        { return Either<LeftType, RightType>{std::move(right_), RightTag{}}; }

    template <typename EitherLeftOrRight>
    constexpr Either(EitherLeftOrRight && obj,
                     EnableIfLeftXorRightPtr<EitherLeftOrRight> = nullptr):
        m_datum(std::move(obj)) {}

    template <typename Func>
    [[nodiscard]] constexpr Either<LeftType, ReturnTypeOf<Func>> map(Func && f) {
        using NewRight = ReturnTypeOf<Func>;
        static_assert(!std::is_same_v<void, NewRight>,
                      "Return type of given function may not be void");
        VerifyArgumentsForSide<Func, RightType>{};
        if (is_left())
            { return left<NewRight>(left()); }
        return right<LeftType>(f(right()));
    }

    template <typename Func>
    [[nodiscard]] constexpr Either<ReturnTypeOf<Func>, RightType> map_left(Func && f) {
        using NewLeft = ReturnTypeOf<Func>;
        static_assert(!std::is_same_v<void, NewLeft>,
                      "Return type of given function may not be void");
        VerifyArgumentsForSide<Func, LeftType>{};
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
        static_assert(std::is_same_v<NewLeftType, LeftType>,
            "Given function must preserve left type");
        VerifyArgumentsForSide<Func, RightType>{};
        if (is_left())
            { return left<NewRightType>(left()); }
        return f(right());
    }

    template <typename Func>
    [[nodiscard]] constexpr EnableIfReturnsEither<Func> chain_left(Func && f) {
        using FuncReturnType = ReturnTypeOf<Func>;
        using NewLeftType = typename IsEither<FuncReturnType>::LeftType;
        using NewRightType = typename IsEither<FuncReturnType>::RightType;
        static_assert(std::is_same_v<NewRightType, RightType>,
            "Given function must preserve right type");
        VerifyArgumentsForSide<Func, LeftType>{};
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

    constexpr Either(LeftType && obj, LeftTag):
        m_datum(std::in_place_index_t<0>{}, std::move(obj)) {}

    constexpr Either(RightType && obj, RightTag):
        m_datum(std::in_place_index_t<1>{}, std::move(obj)) {}

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

// ----------------------------------------------------------------------------

template <typename LeftT, typename RightT>
template <typename CommonT>
template <typename Func>
constexpr typename Either<LeftT, RightT>::template Fold<CommonT>
    Either<LeftT, RightT>::Fold<CommonT>::map(Func && f)
{
    VerifyFoldFunctionForSide<CommonT, Func, RightType>{};
    if (m_ei.is_right()) {
        OptionalCommon rv = f(m_ei.right());
        return Fold{std::move(rv), std::move(m_ei)};
    }
    return Fold{std::move(m_rv), std::move(m_ei)};
}

template <typename LeftT, typename RightT>
template <typename CommonT>
template <typename Func>
constexpr typename Either<LeftT, RightT>::template Fold<CommonT>
    Either<LeftT, RightT>::Fold<CommonT>::map_left(Func && f)
{
    VerifyFoldFunctionForSide<CommonT, Func, LeftType>{};
    if (m_ei.is_left()) {
        OptionalCommon rv = f(m_ei.left());
        return Fold{std::move(rv), std::move(m_ei)};
    }
    return Fold{std::move(m_rv), std::move(m_ei)};
}

template <typename LeftT, typename RightT>
template <typename CommonT>
constexpr Either<LeftT, RightT>::Fold<CommonT>::
    Fold(OptionalCommon && rv, Either<LeftT, RightT> && either):
     m_rv(std::move(rv)),
     m_ei(std::move(either))
{}

// ----------------------------------------------------------------------------



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
#endif

template <typename LeftT, typename RightT>
class Either final : public OptionalEitherBase {
public:
    using LeftType = LeftT;
    using RightType = RightT;

private:
    template <typename EitherLeftOrRight>
    using EnableIfLeftXorRightPtr = typename std::enable_if_t<
        bool(std::is_same_v<EitherLeftOrRight, LeftType > ^
             std::is_same_v<EitherLeftOrRight, RightType>),
        void *>;

    struct SkipEmptyCheck final {};

public:
#   if 0
    template <typename RightType, typename LeftType>
    static constexpr Either<LeftType, RightType> left(LeftType && left_)
        { return OptionalEither<LeftType, RightType>::left(std::move(left_)); }

    template <typename LeftType, typename RightType>
    static constexpr Either<LeftType, RightType> right(RightType && right_)
        { return OptionalEither<LeftType, RightType>::right(std::move(right_)); }

    template <typename LeftType, typename RightType>
    static constexpr Either<LeftType, RightType> left()
        { return OptionalEither<LeftType, RightType>::template left(LeftType{}); }

    template <typename LeftType, typename RightType>
    static constexpr Either<LeftType, RightType> right()
        { return OptionalEither<LeftType, RightType>::template right(RightType{}); }
#   endif
    constexpr explicit Either(OptionalEither<LeftT, RightT> && opt_either):
        m_datum(verify_non_empty("Either", std::move(opt_either))) {}

    template <typename EitherLeftOrRight>
    constexpr Either(EitherLeftOrRight && obj,
                     EnableIfLeftXorRightPtr<EitherLeftOrRight> = nullptr):
        m_datum(std::move(obj)) {}

    constexpr Either(TypeTag<LeftType>, RightType && right):
        m_datum(TypeTag<LeftType>{}, std::move(right)) {}

    constexpr Either(LeftType && left, TypeTag<RightType>):
        m_datum(std::move(left), TypeTag<RightType>{}) {}

    template <typename CommonT>
    constexpr Fold<LeftType, RightType, CommonT> fold()
        { return FoldAttn::make_fold<LeftType, RightType, CommonT>(std::move(m_datum)); }

    template <typename Func>
    [[nodiscard]] constexpr Either<LeftType, ReturnTypeOf<Func>> map(Func && f)
        { return Either<LeftType, ReturnTypeOf<Func>>{m_datum.template map<Func>(std::move(f))}; }

    template <typename Func>
    [[nodiscard]] constexpr Either<ReturnTypeOf<Func>, RightType> map_left(Func && f)
        { return Either<ReturnTypeOf<Func>, RightType>{m_datum.template map_left<Func>(std::move(f))}; }

    template <typename Func>
    [[nodiscard]] constexpr EnableIfReturnsEither<Func> chain(Func && f) {
        RightChainFunctionReturnRequirements<Func>{};
        VerifyArgumentsForSide<Func, RightType>{};
        if (is_right())
            { return f(right()); }
        return with_new_right_type<EitherTypeRight<ReturnTypeOf<Func>>>();
    }

    template <typename Func>
    [[nodiscard]] constexpr EnableIfReturnsEither<Func> chain_left(Func && f) {
        LeftChainFunctionReturnRequirements<Func>{};
        VerifyArgumentsForSide<Func, LeftType>{};
        if (is_left())
            { return f(left()); }
        return with_new_left_type<EitherTypeLeft<ReturnTypeOf<Func>>>();
    }

    constexpr LeftT left()
        { return m_datum.left(); }

    constexpr RightT right()
        { return m_datum.right(); }

    constexpr bool is_left() const
        { return m_datum.is_left(); }

    constexpr bool is_right() const
        { return m_datum.is_right(); }

private:
    static OptionalEither<LeftT, RightT> verify_non_empty
        (const char * caller, OptionalEither<LeftT, RightT> && opt_either)
    {
        using namespace exceptions_abbr;
        if (!opt_either.is_empty()) return opt_either;
        throw RtError{"Either::" + std::string{caller} +
                      ": given optional either must not be empty."};
    }

    template <typename Func>
    struct RightChainFunctionReturnRequirements final :
        public ChainFunctionReturnsEither<Func>,
        public RightChainPreserveLeft<LeftType, Func>
    {};

    template <typename Func>
    struct LeftChainFunctionReturnRequirements final :
        public ChainFunctionReturnsEither<Func>,
        public LeftChainPreserveRight<RightType, Func>
    {};

    template <typename NewRightType>
    Either<LeftType, NewRightType> with_new_right_type() {
        return Either<LeftType, NewRightType>
            {OptionalEitherToEitherAttn::new_right_type_with
                <LeftType, RightType, NewRightType>(std::move(m_datum))};
    }

    template <typename NewLeftType>
    Either<NewLeftType, RightType> with_new_left_type() {
        return Either<NewLeftType, RightType>
            {OptionalEitherToEitherAttn::new_left_type_with
                <LeftType, RightType, NewLeftType>(std::move(m_datum))};
    }

    OptionalEither<LeftT, RightT> m_datum;
};

template <typename LeftType>
class EitherRightMaker final {
public:
    template <typename RightType>
    constexpr Either<LeftType, RightType> with(RightType && right) const
        { return Either<LeftType, RightType>{TypeTag<LeftType>{}, std::move(right)}; }
#   if 1
    template <typename RightType>
    constexpr Either<LeftType, RightType> operator() (RightType && right) const
        { return with<RightType>(std::move(right)); }
#   endif
};

template <typename LeftType>
class EitherLeftMaker final {
public:
    explicit constexpr EitherLeftMaker(LeftType && left_obj):
        m_obj(left_obj) {}

    template <typename RightType>
    constexpr Either<LeftType, RightType> with()
        { return Either<LeftType, RightType>{std::move(m_obj), TypeTag<RightType>{}}; }

private:
    LeftType m_obj;
};

namespace either {

template <typename LeftType>
EitherRightMaker<LeftType> right() {
    return EitherRightMaker<LeftType>{};
}

template <typename LeftType>
EitherLeftMaker<LeftType> left(LeftType && obj) {
    return EitherLeftMaker<LeftType>{std::move(obj)};
}

} // end of either namespace -> into ::cul

} // end of cul namespace
