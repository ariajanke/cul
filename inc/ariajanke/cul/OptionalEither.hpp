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
#include <ariajanke/cul/Util.hpp>

#include <optional>
#include <variant>
#include <stdexcept>

namespace cul {

template <typename LeftT, typename RightT>
class OptionalEither;

class FoldBase;

template <typename LeftT, typename RightT>
class Either;

class OptionalEitherBase {
protected:
    template <typename Func>
    using ReturnTypeOf = typename FunctionTraitsOf<Func>::ReturnType;

    template <typename Func>
    using StrippedArgumentTypesOf = typename FunctionTraitsOf<Func>::
        ArgumentTypes::
        template Transform<std::remove_reference_t>::
        template Transform<std::remove_cv_t>;

private:
    template <typename AnyT>
    struct IsEither final {
        static constexpr const bool k_value = false;
    };

    template <typename AnyT>
    struct IsOptionalEither final {
        static constexpr const bool k_value = false;
    };

    template <typename LeftT, typename RightT>
    struct IsOptionalEither<OptionalEither<LeftT, RightT>> final {
        static constexpr const bool k_value = true;
    };

    template <typename LeftT, typename RightT>
    struct IsEither<Either<LeftT, RightT>> final {
        static constexpr const bool k_value = true;
    };

    template <typename AnyT>
    struct EitherTraits final {};

    template <typename LeftT, typename RightT>
    struct EitherTraits<Either<LeftT, RightT>> final {
        using LeftType = LeftT;
        using RightType = RightT;
    };

    template <typename LeftT, typename RightT>
    struct EitherTraits<OptionalEither<LeftT, RightT>> final {
        using LeftType = LeftT;
        using RightType = RightT;
    };

protected:
    template <typename ... Types>
    static constexpr const bool kt_is_either = IsEither<Types...>::k_value;

    template <typename ... Types>
    static constexpr const bool kt_is_optional_either =
        IsOptionalEither<Types...>::k_value;

    template <typename Type>
    static constexpr const bool kt_is_void = std::is_same_v<void, Type>;

    template <typename ... Types>
    using EitherTypeLeft = typename EitherTraits<Types...>::LeftType;

    template <typename ... Types>
    using EitherTypeRight = typename EitherTraits<Types...>::RightType;

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

    template <typename T>
    using EnableIfReturnsOptionalEither =
        std::enable_if_t<kt_is_optional_either<ReturnTypeOf<T>>, ReturnTypeOf<T>>;

    static constexpr const int k_left_idx = 0;
    static constexpr const int k_right_idx = 1;
    using InPlaceRight = std::in_place_index_t<k_right_idx>;
    using InPlaceLeft = std::in_place_index_t<k_left_idx>;

    template <typename LeftType, typename RightType, typename EitherLeftOrRight>
    using EnableIfLeftXorRightPtr_ = typename std::enable_if_t<
        bool(std::is_same_v<EitherLeftOrRight, LeftType > ^
             std::is_same_v<EitherLeftOrRight, RightType>),
        void *>;

    class BareEitherBase {
    protected:
        struct Empty final {};
        struct Consumed final {};

        static constexpr const int k_empty_idx = 2;
        static constexpr const int k_consumed_idx = 3;

        constexpr BareEitherBase() {}
    };

    template <typename LeftT, typename RightT>
    class BareEither final : public BareEitherBase {
        template <typename EitherLeftOrRight>
        using EnableIfLeftXorRightPtr =
            EnableIfLeftXorRightPtr_<LeftT, RightT, EitherLeftOrRight>;
    public:
        constexpr BareEither(): m_datum(Empty{}) {}

        constexpr BareEither(const BareEither & rhs):
            m_datum(rhs.m_datum) {}

        constexpr BareEither(BareEither && rhs):
            m_datum(std::move(rhs.m_datum))
        { rhs.m_datum = DatumVariant{Consumed{}}; }

        template <typename EitherLeftOrRight>
        constexpr explicit BareEither(EitherLeftOrRight && obj,
                                      EnableIfLeftXorRightPtr<EitherLeftOrRight> = nullptr):
            m_datum(std::move(obj)) {}

        template <typename ... FurtherTypes>
        constexpr BareEither(InPlaceLeft, FurtherTypes &&... args):
            m_datum(std::in_place_index_t<k_left_idx>{}, std::forward<FurtherTypes>(args)...) {}

        template <typename ... FurtherTypes>
        constexpr BareEither(InPlaceRight, FurtherTypes &&... args):
            m_datum(std::in_place_index_t<k_right_idx>{}, std::forward<FurtherTypes>(args)...) {}

        constexpr BareEither & operator = (const BareEither & rhs) {
            m_datum = rhs.m_datum;
            return *this;
        }

        constexpr BareEither & operator = (BareEither && rhs) {
            if (this != &rhs) {
                m_datum = std::move(rhs);
                rhs.m_datum = DatumVariant{Consumed{}};
            }
            return *this;
        }

        constexpr bool is_empty() const
            { return std::holds_alternative<Empty>(m_datum); }

        constexpr bool is_left() const
            { return m_datum.index() == k_left_idx; }

        constexpr bool is_right() const
            { return m_datum.index() == k_right_idx; }

        constexpr LeftT left() { return move_out_value<k_left_idx>(); }

        constexpr RightT right() { return move_out_value<k_right_idx>(); }

        template <typename NewLeft>
        constexpr BareEither<NewLeft, RightT> with_new_left_type() {
            switch (m_datum.index()) {
            case k_right_idx:
                return BareEither<NewLeft, RightT>{InPlaceRight{}, right()};
            case k_empty_idx: return BareEither<NewLeft, RightT>{};
            default:
                throw std::runtime_error{""};
            }
        }

        template <typename NewRight>
        constexpr BareEither<LeftT, NewRight> with_new_right_type() {
            switch (m_datum.index()) {
            case k_left_idx :
                return BareEither<LeftT, NewRight>{InPlaceLeft{}, left()};
            case k_empty_idx: return BareEither<LeftT, NewRight>{};
            default:
                throw std::runtime_error{""};
            }
        }

    private:        
        using DatumVariant = std::variant<LeftT, RightT, Empty, Consumed>;

        template <int kt_index>
        constexpr auto move_out_value() {
            auto rv = std::move(std::get<kt_index>(m_datum));
            m_datum = DatumVariant{Consumed{}};
            return rv;
        }

        constexpr bool is_consumed() const
            { return std::holds_alternative<Consumed>(m_datum); }

        static constexpr DatumVariant verify_not_consumed(DatumVariant && datum) {
            if (!std::holds_alternative<Consumed>(datum))
                { return std::move(datum); }
            throw std::runtime_error{""};
        }
        DatumVariant m_datum;
    };

    friend class FoldBase;

    template <typename Func>
    struct ChainFunctionReturnsOptionalEither {
        static_assert(kt_is_optional_either<ReturnTypeOf<Func>>,
                      "Must return an optional either type.");
    };

    template <typename Func>
    struct ChainFunctionReturnsEither {
        static_assert(kt_is_either<ReturnTypeOf<Func>>,
                      "Must return an either type.");
    };

    template <typename LeftType, typename Func>
    struct RightChainPreserveLeft {
        static_assert
            (std::is_same_v<EitherTypeLeft<ReturnTypeOf<Func>>, LeftType>,
             "Given function must preserve left type");
    };

    template <typename RightType, typename Func>
    struct LeftChainPreserveRight {
        static_assert
            (std::is_same_v<EitherTypeRight<ReturnTypeOf<Func>>, RightType>,
             "Given function must preserve right type");
    };
};

template <typename LeftT, typename RightT>
class Either;

template <typename LeftT, typename RightT, typename CommonT>
class Fold;



class FoldBase {
protected:
    template <typename LeftT, typename RightT>
    using BareEither = OptionalEitherBase::BareEither<LeftT, RightT>;

    constexpr FoldBase() {}

    template <typename CommonT, typename Func, typename SideType>
    struct VerifyFoldFunctionForSide final {
        using ReturnType = OptionalEitherBase::ReturnTypeOf<Func>;
        using ArgumentTypes = OptionalEitherBase::StrippedArgumentTypesOf<Func>;
        static_assert(std::is_same_v<ReturnType, CommonT>,
                      "given function must return common type");
        static_assert(ArgumentTypes::k_count == 1,
                      "Given function must accept exactly one argument");
        static_assert
            (ArgumentTypes::template kt_occurance_count<SideType> == 1,
             "Given function must accept some form of the side's type");
    };
};

class FoldAttn final : public FoldBase {
private:
    template <typename LeftT, typename RightT>
    friend class Either;

    template <typename LeftT, typename RightT>
    friend class OptionalEither;

    template <typename LeftT, typename RightT, typename CommonT>
    static constexpr Fold<LeftT, RightT, CommonT>
        make_fold(OptionalEither<LeftT, RightT> &&);

    template <typename LeftT, typename RightT, typename CommonT>
    static constexpr Fold<LeftT, RightT, CommonT>
        make_fold(CommonT &&, BareEither<LeftT, RightT> &&);
};

template <typename LeftT, typename RightT, typename CommonT>
class Fold final : public FoldBase {
public:
    using CommonType = CommonT;
    using LeftType = LeftT;
    using RightType = RightT;

    template <typename Func>
    constexpr Fold map(Func && f);

    template <typename Func>
    constexpr Fold map_left(Func && f);

    constexpr CommonType operator () () const
        { return m_value.value(); }

private:
    using OptionalCommon = std::optional<CommonT>;
    friend class FoldAttn;

    constexpr Fold
        (OptionalCommon && default_value, BareEither<LeftT, RightT> && datum):
        m_value(std::move(default_value)),
        m_datum(std::move(datum)) {}

    OptionalCommon m_value;
    BareEither<LeftT, RightT> m_datum;
};

template <typename LeftT, typename RightT, typename CommonT>
/* private static */ constexpr Fold<LeftT, RightT, CommonT>
    FoldAttn::make_fold
    (OptionalEither<LeftT, RightT> && opt_either)
{
    return Fold<LeftT, RightT, CommonT>
        {std::optional<CommonT>{}, std::move(opt_either.m_datum)};
}

template <typename LeftT, typename RightT, typename CommonT>
/* private static */ constexpr Fold<LeftT, RightT, CommonT>
    FoldAttn::make_fold(CommonT && obj, BareEither<LeftT, RightT> && opt_either)
{
    return Fold<LeftT, RightT, CommonT>
        {std::optional<CommonT>{std::move(obj)}, std::move(opt_either)};
}

class OptionalEitherToEitherAttn final {
    template <typename LeftT, typename RightT>
    friend class Either;

    template <typename LeftType, typename RightType, typename NewRightType>
    static OptionalEither<LeftType, NewRightType>
        new_right_type_with(OptionalEither<LeftType, RightType> && opt_either)
    { return opt_either.template with_new_right_type<NewRightType>(); }

    template <typename LeftType, typename RightType, typename NewLeftType>
    static OptionalEither<NewLeftType, RightType>
        new_left_type_with(OptionalEither<LeftType, RightType> && opt_either)
    { return opt_either.template with_new_left_type<NewLeftType>(); }
};

// ----------------------------------------------------------------------------

/// An optional either, is an either with a "third not present" state.
///
/// Optional either is going to be *very* similar. It can be empty, folding has
/// a required argument.
template <typename LeftT, typename RightT>
class OptionalEither final : public OptionalEitherBase {
    template <typename EitherLeftOrRight>
    using EnableIfLeftXorRightPtr =
        EnableIfLeftXorRightPtr_<LeftT, RightT, EitherLeftOrRight>;
public:
    using LeftType = LeftT;
    using RightType = RightT;

    static_assert(!kt_is_void<LeftType> , "LeftType may not be void" );
    static_assert(!kt_is_void<RightType>, "RightType may not be void");

    constexpr OptionalEither():
        m_datum() {}

    template <typename EitherLeftOrRight>
    constexpr OptionalEither(EitherLeftOrRight && obj,
                             EnableIfLeftXorRightPtr<EitherLeftOrRight> = nullptr):
        m_datum(std::move(obj)) {}

    constexpr OptionalEither(TypeTag<LeftType>, RightType && obj):
        m_datum(InPlaceRight{}, std::move(obj)) {}

    constexpr OptionalEither(LeftType && obj, TypeTag<RightType>):
        m_datum(InPlaceLeft{}, std::move(obj)) {}

    constexpr OptionalEither(OptionalEither && rhs):
        m_datum(std::move(rhs.m_datum)) {}

    constexpr OptionalEither(const OptionalEither & rhs):
        m_datum(rhs.m_datum) {}

    template <typename Func>
    [[nodiscard]] constexpr OptionalEither<LeftType, ReturnTypeOf<Func>> map(Func && f) {
        using NewRight = ReturnTypeOf<Func>;
        static_assert(!std::is_same_v<void, NewRight>,
                      "Return type of given function may not be void");
        VerifyArgumentsForSide<Func, RightType>{};
        if (is_right())
            return OptionalEither<LeftType, NewRight>{TypeTag<LeftType>{}, f(right())};
        return with_new_right_type<NewRight>();
    }

    template <typename Func>
    [[nodiscard]] constexpr OptionalEither<ReturnTypeOf<Func>, RightType> map_left(Func && f) {
        using NewLeft = ReturnTypeOf<Func>;
        static_assert(!std::is_same_v<void, NewLeft>,
                      "Return type of given function may not be void");
        VerifyArgumentsForSide<Func, LeftType>{};
        if (is_left())
            return OptionalEither<NewLeft, RightType>{f(left()), TypeTag<RightType>{}};
        return with_new_left_type<NewLeft>();
    }

    template <typename CommonT>
    [[nodiscard]] constexpr Fold<LeftT, RightT, CommonT> fold
        (CommonT && default_value)
    {
        return FoldAttn::make_fold<LeftT, RightT, CommonT>
            (std::move(default_value), std::move(m_datum));
    }

    template <typename Func>
    [[nodiscard]] constexpr EnableIfReturnsOptionalEither<Func> chain(Func && f) {
        RightChainFunctionReturnRequirements<Func>{};
        VerifyArgumentsForSide<Func, RightType>{};
        if (is_right())
            { return f(right()); }
        return with_new_right_type<EitherTypeRight<ReturnTypeOf<Func>>>();
    }

    template <typename Func>
    [[nodiscard]] constexpr EnableIfReturnsOptionalEither<Func> chain_left(Func && f) {
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

    constexpr bool is_empty() const
        { return m_datum.is_empty(); }

private:
    using BareEither_ = BareEither<LeftT, RightT>;

    template <typename OtherLeft, typename OtherRight>
    friend class OptionalEither;

    friend class OptionalEitherToEitherAttn;
    friend class FoldAttn;

    template <typename Func>
    struct RightChainFunctionReturnRequirements final :
        public ChainFunctionReturnsOptionalEither<Func>,
        public RightChainPreserveLeft<LeftType, Func>
    {};

    template <typename Func>
    struct LeftChainFunctionReturnRequirements final :
        public ChainFunctionReturnsOptionalEither<Func>,
        public LeftChainPreserveRight<RightType, Func>
    {};

    constexpr explicit OptionalEither(BareEither_ && either_datum_):
        m_datum(std::move(either_datum_)) {}

    template <typename NewRightType>
    constexpr OptionalEither<LeftType, NewRightType> with_new_right_type() {
        return OptionalEither<LeftType, NewRightType>
            {m_datum.template with_new_right_type<NewRightType>()};
    }

    template <typename NewLeftType>
    constexpr OptionalEither<NewLeftType, RightType> with_new_left_type() {
        return OptionalEither<NewLeftType, RightType>
            {m_datum.template with_new_left_type<NewLeftType>()};
    }

    BareEither_ m_datum;
};

namespace either {

template <typename LeftType>
class OptionalEitherRightMaker final {
public:
    template <typename RightType>
    constexpr OptionalEither<LeftType, RightType> with(RightType && right) const
        { return OptionalEither<LeftType, RightType>{TypeTag<LeftType>{}, std::move(right)}; }

    template <typename RightType>
    constexpr OptionalEither<LeftType, RightType> with() const
        { return OptionalEither<LeftType, RightType>{}; }

    template <typename RightType>
    constexpr OptionalEither<LeftType, RightType> operator() (RightType && right) const
        { return with<RightType>(std::move(right)); }
};

template <typename LeftType>
class OptionalEitherLeftMaker final {
public:
    explicit constexpr OptionalEitherLeftMaker(LeftType && left_obj):
        m_obj(left_obj) {}

    constexpr OptionalEitherLeftMaker(): m_obj() {}

    template <typename RightType>
    constexpr OptionalEither<LeftType, RightType> with() {
        if (!m_obj) { return OptionalEither<LeftType, RightType>{}; }
        return OptionalEither<LeftType, RightType>{std::move(*m_obj), TypeTag<RightType>{}}; }

private:
    std::optional<LeftType> m_obj;
};

template <typename LeftType>
constexpr OptionalEitherRightMaker<LeftType> optional_right()
    { return OptionalEitherRightMaker<LeftType>{}; }

template <typename LeftType>
constexpr OptionalEitherLeftMaker<LeftType> optional_left(LeftType && obj)
    { return OptionalEitherLeftMaker<LeftType>{std::move(obj)}; }

template <typename LeftType>
constexpr OptionalEitherLeftMaker<LeftType> optional_left()
    { return OptionalEitherLeftMaker<LeftType>{}; }

} // end of either namespace -> into ::cul


// ----------------------------------------------------------------------------

template <typename LeftT, typename RightT, typename CommonT>
template <typename Func>
constexpr Fold<LeftT, RightT, CommonT>
    Fold<LeftT, RightT, CommonT>::map(Func && f)
{
    VerifyFoldFunctionForSide<CommonT, Func, RightType>{};
    if (m_datum.is_right()) {
        OptionalCommon rv = f(m_datum.right());
        return Fold{std::move(rv), std::move(m_datum)};
    }

    return Fold{std::move(m_value), std::move(m_datum)};
}

template <typename LeftT, typename RightT, typename CommonT>
template <typename Func>
constexpr Fold<LeftT, RightT, CommonT>
    Fold<LeftT, RightT, CommonT>::map_left(Func && f)
{
    VerifyFoldFunctionForSide<CommonT, Func, LeftType>{};
    if (m_datum.is_left()) {
        OptionalCommon rv = f(m_datum.left());
        return Fold{std::move(rv), std::move(m_datum)};
    }
    return Fold{std::move(m_value), std::move(m_datum)};
}

// ----------------------------------------------------------------------------



} // end of cul namespace
