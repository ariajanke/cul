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

template <typename LeftT, typename RightT>
class Either;

namespace either {

template <typename LeftT, typename RightT, typename CommonT>
class Fold;

} // end of either namespace -> into ::cul
#if 0
namespace detail {

class FoldBase;

template <typename LeftType, typename RightType>
static constexpr const bool kt_are_copyable =
    std::is_copy_constructible_v<LeftType> &&
    std::is_copy_constructible_v<RightType>;

template <typename LeftType, typename RightType>
static constexpr const bool kt_are_moveable =
    std::is_move_constructible_v<LeftType> &&
    std::is_move_constructible_v<RightType>;

template <typename LeftT, typename RightT,
    bool kt_is_copy_enabled = kt_are_copyable<LeftT, RightT>,
    bool kt_is_move_enabled = kt_are_moveable<LeftT, RightT>>
class BaseEitherCopyMove;

class EitherBase {
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

    template <typename LeftType, typename RightType, typename EitherLeftOrRight>
    using EnableIfLeftXorRightPtr_ = typename std::enable_if_t<
        bool(std::is_same_v<EitherLeftOrRight, LeftType > ^
             std::is_same_v<EitherLeftOrRight, RightType>),
        void *>;

    template <typename LeftType, typename RightType>
    using EnableIfCopyConstructible =
        typename std::enable_if_t<
            std::is_copy_constructible_v<LeftType> &&
            std::is_copy_constructible_v<RightType>, void *>;

    friend class ::cul::detail::FoldBase;

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


    template <typename LeftT, typename RightT,
        bool kt_is_copy_enabled,
        bool kt_is_move_enabled>
    friend class BaseEitherCopyMove;
};

class BareEitherBase {
protected:
    struct Empty final {};
    struct Consumed final {};
};

static constexpr const int k_left_idx = 0;
static constexpr const int k_right_idx = 1;
using InPlaceRight = std::in_place_index_t<k_right_idx>;
using InPlaceLeft = std::in_place_index_t<k_left_idx>;

template <typename LeftT, typename RightT,
    bool kt_is_copy_enabled = kt_are_copyable<LeftT, RightT>,
    bool kt_is_move_enabled = kt_are_moveable<LeftT, RightT>>
class BareEitherCopyMove : public BareEitherBase {
public:
    static_assert(std::is_copy_constructible_v<LeftT> ==
                  std::is_copy_assignable_v<LeftT>      );
    static_assert(std::is_copy_constructible_v<RightT> ==
                  std::is_copy_assignable_v<RightT>      );

    constexpr BareEitherCopyMove(): m_datum(Empty{}) {}

    constexpr BareEitherCopyMove(const BareEitherCopyMove & rhs):
        m_datum(rhs.m_datum) {}

    constexpr BareEitherCopyMove(BareEitherCopyMove && rhs):
        m_datum(std::move(rhs.m_datum))
    { rhs.m_datum = DatumVariant{Consumed{}}; }

    template <typename ... Types>
    constexpr BareEitherCopyMove(InPlaceLeft, Types &&... args):
        m_datum(InPlaceLeft{}, std::forward<Types>(args)...) {}

    template <typename ... Types>
    constexpr BareEitherCopyMove(InPlaceRight, Types &&... args):
        m_datum(InPlaceRight{}, std::forward<Types>(args)...) {}

    constexpr BareEitherCopyMove & operator = (const BareEitherCopyMove & rhs) {
        if (this != & rhs)
            { m_datum = rhs.m_datum; }
        return *this;
    }

    constexpr BareEitherCopyMove & operator = (BareEitherCopyMove && rhs) {
        if (this != &rhs) {
            m_datum = std::move(rhs.m_datum);
            rhs.m_datum = DatumVariant{Consumed{}};
        }
        return *this;
    }

protected:
    using DatumVariant = std::variant<LeftT, RightT, Empty, Consumed>;
    DatumVariant m_datum;
};

template <typename LeftT, typename RightT>
class BareEitherCopyMove<LeftT, RightT, true, false> :
    public BareEitherCopyMove<LeftT, RightT, true, true>
{
public:
    using BareEitherCopyMove<LeftT, RightT, true, true>::BareEitherCopyMove;

    constexpr BareEitherCopyMove(const BareEitherCopyMove &) = default;

    constexpr BareEitherCopyMove(BareEitherCopyMove &&) = delete;

    constexpr BareEitherCopyMove & operator = (const BareEitherCopyMove &) = default;

    constexpr BareEitherCopyMove & operator = (BareEitherCopyMove &&) = delete;
};

template <typename LeftT, typename RightT>
class BareEitherCopyMove<LeftT, RightT, false, true> :
    public BareEitherCopyMove<LeftT, RightT, true, true>
{
public:
    using BareEitherCopyMove<LeftT, RightT, true, true>::BareEitherCopyMove;

    constexpr BareEitherCopyMove(const BareEitherCopyMove &) = delete;

    constexpr BareEitherCopyMove(BareEitherCopyMove &&) = default;

    constexpr BareEitherCopyMove & operator = (const BareEitherCopyMove &) = delete;

    constexpr BareEitherCopyMove & operator = (BareEitherCopyMove &&) = default;
};

template <typename LeftT, typename RightT>
class BareEitherCopyMove<LeftT, RightT, false, false> :
    public BareEitherCopyMove<LeftT, RightT, true, true>
{
public:
    using BareEitherCopyMove<LeftT, RightT, true, true>::BareEitherCopyMove;

    constexpr BareEitherCopyMove(const BareEitherCopyMove &) = delete;

    constexpr BareEitherCopyMove(BareEitherCopyMove &&) = delete;

    constexpr BareEitherCopyMove & operator = (const BareEitherCopyMove &) = delete;

    constexpr BareEitherCopyMove & operator = (BareEitherCopyMove &&) = delete;
};

// gotta move this out
template <typename LeftT, typename RightT>
class BareEither final : public BareEitherCopyMove<LeftT, RightT> {
    using Super = BareEitherCopyMove<LeftT, RightT>;
public:
    using Super::Super;

    constexpr bool is_empty() const
        { return type_index() == k_empty_idx; }

    constexpr bool is_left() const
        { return type_index() == k_left_idx; }

    constexpr bool is_right() const
        { return type_index() == k_right_idx; }

    constexpr LeftT left() { return move_out_value<k_left_idx>(); }

    constexpr RightT right() { return move_out_value<k_right_idx>(); }

    template <typename NewLeft>
    constexpr BareEither<NewLeft, RightT> with_new_left_type() {
        switch (type_index()) {
        case k_right_idx:
            return BareEither<NewLeft, RightT>{InPlaceRight{}, right()};
        case k_empty_idx:
            return BareEither<NewLeft, RightT>{};
        default:
            throw RtError{k_consumed_msg};
        }
    }

    template <typename NewRight>
    constexpr BareEither<LeftT, NewRight> with_new_right_type() {
        switch (type_index()) {
        case k_left_idx :
            return BareEither<LeftT, NewRight>{InPlaceLeft{}, left()};
        case k_empty_idx:
            return BareEither<LeftT, NewRight>{};
        default:
            throw RtError{k_consumed_msg};
        }
    }

private:
    //using Empty = typename Super::Empty;
    using Consumed = typename Super::Consumed;
    using DatumVariant = typename Super::DatumVariant;// std::variant<LeftT, RightT, Empty, Consumed>;
    using RtError = std::runtime_error;

    static constexpr const int k_empty_idx = 2;
    static constexpr const char * k_consumed_msg =
        "The either has been 'consumed', and can no longer be used.";

    template <int kt_index>
    constexpr auto move_out_value() {
        auto rv = std::get<kt_index>(verify_not_consumed(std::move(this->m_datum)));
        this->m_datum = DatumVariant{Consumed{}};
        return rv;
    }

    constexpr bool is_consumed() const
        { return std::holds_alternative<Consumed>(this->m_datum); }

    constexpr std::size_t type_index() const
        { return this->m_datum.index(); }

    static constexpr DatumVariant verify_not_consumed(DatumVariant && datum) {
        if (!std::holds_alternative<Consumed>(datum))
            { return std::move(datum); }
        throw RtError{k_consumed_msg};
    }
};

class FoldBase {
protected:
    template <typename LeftT, typename RightT>
    using BareEither = BareEither<LeftT, RightT>;

    constexpr FoldBase() {}

    template <typename CommonT, typename Func, typename SideType>
    struct VerifyFoldFunctionForSide final {
        using ReturnType = detail::EitherBase::ReturnTypeOf<Func>;
        using ArgumentTypes = detail::EitherBase::StrippedArgumentTypesOf<Func>;
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
    friend class ::cul::Either;

    template <typename LeftT, typename RightT>
    friend class ::cul::OptionalEither;

    template <typename LeftT, typename RightT, typename CommonT>
    static constexpr either::Fold<LeftT, RightT, CommonT>
        make_fold(OptionalEither<LeftT, RightT> &&);

    template <typename LeftT, typename RightT, typename CommonT>
    static constexpr either::Fold<LeftT, RightT, CommonT>
        make_fold(CommonT &&, BareEither<LeftT, RightT> &&);
};

template <typename LeftT, typename RightT, typename CommonT>
/* private static */ constexpr either::Fold<LeftT, RightT, CommonT>
    FoldAttn::make_fold
    (OptionalEither<LeftT, RightT> && opt_either)
{
    return either::Fold<LeftT, RightT, CommonT>
        {std::optional<CommonT>{}, std::move(opt_either.m_datum)};
}

template <typename LeftT, typename RightT, typename CommonT>
/* private static */ constexpr either::Fold<LeftT, RightT, CommonT>
    FoldAttn::make_fold(CommonT && obj, BareEither<LeftT, RightT> && opt_either)
{
    return either::Fold<LeftT, RightT, CommonT>
        {std::optional<CommonT>{std::move(obj)}, std::move(opt_either)};
}

class OptionalEitherToEitherAttn final {
    template <typename LeftT, typename RightT>
    friend class ::cul::Either;

    template <typename LeftType, typename RightType, typename NewRightType>
    static OptionalEither<LeftType, NewRightType>
        new_right_type_with(OptionalEither<LeftType, RightType> && opt_either)
    { return opt_either.template with_new_right_type<NewRightType>(); }

    template <typename LeftType, typename RightType, typename NewLeftType>
    static OptionalEither<NewLeftType, RightType>
        new_left_type_with(OptionalEither<LeftType, RightType> && opt_either)
    { return opt_either.template with_new_left_type<NewLeftType>(); }
};

template <typename LeftT, typename RightT,
    bool kt_is_copy_enabled,
    bool kt_is_move_enabled>
class BaseEitherCopyMove {
public:
    // ---------------------------- !WARNING! -----------------------------
    //                     exposed to public interface

    static_assert(!EitherBase::kt_is_void<LeftT> , "LeftType may not be void" );
    static_assert(!EitherBase::kt_is_void<RightT>, "RightType may not be void");

    // I can't have this on Either
    constexpr BaseEitherCopyMove(): m_datum() {}

    constexpr BaseEitherCopyMove(const BaseEitherCopyMove & rhs):
        m_datum(rhs.m_datum) {}

    constexpr BaseEitherCopyMove(BaseEitherCopyMove && rhs):
        m_datum(std::move(rhs.m_datum)) {}

    constexpr BaseEitherCopyMove & operator = (const BaseEitherCopyMove & rhs) {
        if (this != & rhs)
            { m_datum = rhs.m_datum; }
        return *this;
    }

    constexpr BaseEitherCopyMove & operator = (BaseEitherCopyMove && rhs) {
        if (this != &rhs)
            { m_datum = std::move(rhs.m_datum); }
        return *this;
    }

protected:
    BareEither<LeftT, RightT> m_datum;
};

template <typename LeftT, typename RightT>
class BaseEitherCopyMove<LeftT, RightT, true, false> :
    BaseEitherCopyMove<LeftT, RightT, true, true>
{
public:
    // ---------------------------- !WARNING! -----------------------------
    //                     exposed to public interface

    using BaseEitherCopyMove<LeftT, RightT, true, true>::BaseEitherCopyMove;

    constexpr BaseEitherCopyMove(const BaseEitherCopyMove &) = default;

    constexpr BaseEitherCopyMove(BaseEitherCopyMove &&) = delete;

    constexpr BaseEitherCopyMove & operator = (const BaseEitherCopyMove &) = default;

    constexpr BaseEitherCopyMove & operator = (BaseEitherCopyMove &&) = delete;
};

template <typename LeftT, typename RightT>
class BaseEitherCopyMove<LeftT, RightT, false, true> :
    BaseEitherCopyMove<LeftT, RightT, true, true>
{
public:
    // ---------------------------- !WARNING! -----------------------------
    //                     exposed to public interface

    using BaseEitherCopyMove<LeftT, RightT, true, true>::BaseEitherCopyMove;

    constexpr BaseEitherCopyMove(const BaseEitherCopyMove &) = delete;

    constexpr BaseEitherCopyMove(BaseEitherCopyMove &&) = default;

    constexpr BaseEitherCopyMove & operator = (const BaseEitherCopyMove &) = delete;

    constexpr BaseEitherCopyMove & operator = (BaseEitherCopyMove &&) = default;
};

template <typename LeftT, typename RightT>
class BaseEitherCopyMove<LeftT, RightT, false, false> :
    BaseEitherCopyMove<LeftT, RightT, true, true>
{
public:
    // ---------------------------- !WARNING! -----------------------------
    //                     exposed to public interface

    using BaseEitherCopyMove<LeftT, RightT, true, true>::BaseEitherCopyMove;

    constexpr BaseEitherCopyMove(const BaseEitherCopyMove &) = delete;

    constexpr BaseEitherCopyMove(BaseEitherCopyMove &&) = delete;

    constexpr BaseEitherCopyMove & operator = (const BaseEitherCopyMove &) = delete;

    constexpr BaseEitherCopyMove & operator = (BaseEitherCopyMove &&) = delete;
};

} // end of detail namespace -> ::cul
#endif

namespace detail {

template <typename Head, typename ... Types>
struct AreAllCopyable {
    static constexpr const bool k_value = std::is_copy_constructible_v<Head>;
};

template <typename Head, typename HeadTwo, typename ... Types>
struct AreAllCopyable<Head, HeadTwo, Types...> :
    public AreAllCopyable<HeadTwo, Types...>
{
    static constexpr const bool k_value =
        std::is_copy_constructible_v<Head> &&
        AreAllCopyable<HeadTwo, Types...>::k_value;
};

template <typename Head, typename ... Types>
struct AreAllMoveable {
    static constexpr const bool k_value = std::is_move_constructible_v<Head>;
};

template <typename Head, typename HeadTwo, typename ... Types>
struct AreAllMoveable<Head, HeadTwo, Types...> :
    public AreAllMoveable<HeadTwo, Types...>
{
    static constexpr const bool k_value =
        std::is_move_constructible_v<Head> &&
        AreAllCopyable<HeadTwo, Types...>::k_value;
};

template <typename ... Types>
constexpr const bool kt_are_all_copyable = AreAllCopyable<Types...>::k_value;

template <typename ... Types>
constexpr const bool kt_are_all_moveable = AreAllMoveable<Types...>::k_value;

template <typename T>
constexpr const bool kt_is_void = std::is_same_v<void, T>;
#if 0
template <typename LeftT, typename RightT>
constexpr const bool kt_both_copyable =
    std::is_copy_constructible_v<LeftT> && std::is_copy_constructible_v<RightT>;

template <typename LeftT, typename RightT>
constexpr const bool kt_both_moveable =
    std::is_move_constructible_v<LeftT> && std::is_move_constructible_v<RightT>;
#endif
enum class EitherDefaultConstructor {
    enable,
    disable
};

enum class EitherCopyConstructor {
    enable,
    disable
};

enum class EitherMoveConstructor {
    enable,
    disable
};

template <bool kt_b>
constexpr const auto kt_enable_copy_if =
    kt_b ? EitherCopyConstructor::enable : EitherCopyConstructor::disable;

template <bool kt_b>
constexpr const auto kt_enable_move_if =
    kt_b ? EitherMoveConstructor::enable : EitherMoveConstructor::disable;

struct EitherEmpty final {};
struct EitherConsumed final {};

template <typename LeftT, typename RightT,
    EitherDefaultConstructor,
    EitherCopyConstructor = kt_enable_copy_if<kt_are_all_copyable<LeftT, RightT>>,
    EitherMoveConstructor = kt_enable_move_if<kt_are_all_moveable<LeftT, RightT>>
    >
class EitherConstructors;

template <typename LeftT, typename RightT, typename EitherLeftOfRight>
using EnableIfLeftXorRightMs =
    std::enable_if_t<std::is_same_v<EitherLeftOfRight, LeftT> !=
                     std::is_same_v<EitherLeftOfRight, RightT>, std::monostate>;

template <typename LeftT, typename RightT, typename EitherLeftOfRight>
using EnableCopyForAmbiguousCtor =
    std::enable_if_t<(std::is_same_v<EitherLeftOfRight, LeftT> !=
                      std::is_same_v<EitherLeftOfRight, RightT>   ) &&
                     kt_are_all_copyable<LeftT, RightT>, std::monostate>;

template <typename LeftT, typename RightT, typename EitherLeftOfRight>
using EnableMoveForAmbiguousCtor =
    std::enable_if_t<(std::is_same_v<EitherLeftOfRight, LeftT> !=
                      std::is_same_v<EitherLeftOfRight, RightT>   ) &&
                     kt_are_all_moveable<LeftT, RightT>, std::monostate>;

template <typename ... Types>
using EnableForCopy =
    std::enable_if_t<kt_are_all_copyable<Types...>, std::monostate>;

template <typename ... Types>
using EnableForMove =
    std::enable_if_t<kt_are_all_moveable<Types...>, std::monostate>;

class DatumVariantUser {
protected:
    template <typename LeftT, typename RightT>
    using DatumVariant =
        std::variant<LeftT, RightT, EitherEmpty, EitherConsumed>;

    static constexpr const std::size_t k_left_idx     = 0;
    static constexpr const std::size_t k_right_idx    = 1;
    static constexpr const std::size_t k_empty_idx    = 2;
    static constexpr const std::size_t k_consumed_idx = 3;

    template <std::size_t k_idx, typename LeftT, typename RightT>
    static constexpr auto consume_datum(DatumVariant<LeftT, RightT> & datum) {
        if constexpr (kt_are_all_moveable<LeftT, RightT>) {
            auto t = std::get<k_idx>(std::move(datum));
            datum = DatumVariant<LeftT, RightT>{detail::EitherConsumed{}};
            return std::move(t);
        } else {
            auto t = std::get<k_idx>(datum);
            datum = DatumVariant<LeftT, RightT>{detail::EitherConsumed{}};
            return t;
        }
    }
};

template <typename LeftT, typename RightT,
    EitherDefaultConstructor,
    EitherCopyConstructor,
    EitherMoveConstructor>
class EitherConstructors : public DatumVariantUser {
public:

    // ---------------------------- !WARNING! -----------------------------
    //                     exposed to public interface

    static_assert(!kt_is_void<LeftT> , "LeftType may not be void" );
    static_assert(!kt_is_void<RightT>, "RightType may not be void");

    // I can't have this on Either
    constexpr EitherConstructors(): m_datum(EitherEmpty{}) {}

    constexpr EitherConstructors(const EitherConstructors & rhs):
        m_datum(rhs.m_datum) {}

    constexpr EitherConstructors(EitherConstructors && rhs):
        m_datum(std::move(rhs.m_datum)) {}

    template <typename EitherLeftOfRight>
    constexpr EitherConstructors(EitherLeftOfRight && left_or_right,
                                 EnableMoveForAmbiguousCtor<LeftT, RightT, EitherLeftOfRight> = std::monostate{}):
        m_datum(std::move(left_or_right)) {}

    template <typename EitherLeftOfRight>
    constexpr EitherConstructors(const EitherLeftOfRight & left_or_right,
                                 EnableCopyForAmbiguousCtor<LeftT, RightT, EitherLeftOfRight> = std::monostate{}):
        m_datum(left_or_right) {}

    constexpr EitherConstructors(LeftT && left, TypeTag<RightT>,
                                 EnableForMove<LeftT, RightT> = std::monostate{}):
        m_datum(std::in_place_index_t<k_left_idx>{}, std::move(left)) {}

    constexpr EitherConstructors(const LeftT & left, TypeTag<RightT>,
                                 EnableForCopy<LeftT, RightT> = std::monostate{}):
        m_datum(std::in_place_index_t<k_left_idx>{}, left) {}

    constexpr EitherConstructors(TypeTag<LeftT>, RightT && right,
                                 EnableForMove<LeftT, RightT> = std::monostate{}):
        m_datum(std::in_place_index_t<k_right_idx>{}, std::move(right)) {}

    constexpr EitherConstructors(TypeTag<LeftT>, const RightT & right,
                                 EnableForCopy<LeftT, RightT> = std::monostate{}):
        m_datum(std::in_place_index_t<k_right_idx>{}, right) {}

    constexpr EitherConstructors & operator = (const EitherConstructors & rhs) {
        if (this != & rhs)
            { m_datum = rhs.m_datum; }
        return *this;
    }

    constexpr EitherConstructors & operator = (EitherConstructors && rhs) {
        if (this != &rhs)
            { m_datum = std::move(rhs.m_datum); }
        return *this;
    }
#   if 0
    template <typename ... Types>
    static constexpr EitherConstructors make(Types &&... args) {
        return EitherConstructors{PrivateDummy{}, std::forward<Types>(args)...};
    }
#   endif
protected:

    constexpr bool is_left_() const { return m_datum.index() == k_left_idx; }

    constexpr bool is_right_() const { return m_datum.index() == k_right_idx; }

    constexpr bool is_empty_() const { return m_datum.index() == k_empty_idx; }

    constexpr LeftT left_() {
        if constexpr (kt_are_all_moveable<LeftT, RightT>) {
            auto t = std::get<k_left_idx>(std::move(m_datum));
            m_datum = DatumVariant<LeftT, RightT>{EitherConsumed{}};
            return std::move(t);
        } else {
            auto t = std::get<k_left_idx>(m_datum);
            m_datum = DatumVariant<LeftT, RightT>{EitherConsumed{}};
            return t;
        }
    }

    constexpr RightT right_() {
        if constexpr (kt_are_all_moveable<LeftT, RightT>) {
            auto t = std::get<k_right_idx>(std::move(m_datum));
            m_datum = DatumVariant<LeftT, RightT>{EitherConsumed{}};
            return std::move(t);
        } else {
            auto t = std::get<k_right_idx>(m_datum);
            m_datum = DatumVariant<LeftT, RightT>{EitherConsumed{}};
            return t;
        }
    }
#   if 0
    template <std::size_t k_inplace_idx, typename ... Types>
    EitherConstructors(std::in_place_index_t<k_inplace_idx>, Types &&... args):
        m_datum(std::in_place_index_t<k_inplace_idx>{}, std::forward<Types>(args)...) {}
#   endif
    constexpr EitherConstructors(DatumVariant<LeftT, RightT> && datum,
                                 EnableForMove<LeftT, RightT> = std::monostate{}):
        m_datum(std::move(datum)) {}

    constexpr EitherConstructors(const DatumVariant<LeftT, RightT> & datum,
                                 EnableForCopy<LeftT, RightT> = std::monostate{}):
        m_datum(datum) {}

    template <typename NewLeftType>
    constexpr DatumVariant<NewLeftType, RightT> with_new_left_type_() {
        using namespace exceptions_abbr;
        using NewDatumType = DatumVariant<NewLeftType, RightT>;
        switch (m_datum.index()) {
        case k_right_idx:
            if constexpr (kt_are_all_moveable<LeftT, RightT>) {
                auto temp = std::get<k_right_idx>(std::move(m_datum));
                m_datum = DatumVariant<LeftT, RightT>{EitherConsumed{}};
                return NewDatumType{std::in_place_index_t<k_right_idx>{}, std::move(temp)};
            } else {
                auto temp = std::get<k_right_idx>(m_datum);
                m_datum = DatumVariant<LeftT, RightT>{EitherConsumed{}};
                return NewDatumType{std::in_place_index_t<k_right_idx>{}, temp};
            }
        case k_empty_idx:
            return DatumVariant<NewLeftType, RightT>{EitherEmpty{}};
        case k_consumed_idx:
        case k_left_idx:
        default:
            throw RtError{""};
        }
    }

    template <typename NewRightType>
    constexpr DatumVariant<LeftT, NewRightType> with_new_right_type_() {
        using namespace exceptions_abbr;
        using NewDatumType = DatumVariant<LeftT, NewRightType>;
        switch (m_datum.index()) {
        case k_left_idx:
            if constexpr (kt_are_all_moveable<LeftT, RightT>) {
                auto temp = std::get<k_left_idx>(std::move(m_datum));
                m_datum = DatumVariant<LeftT, RightT>{EitherConsumed{}};
                return NewDatumType{std::in_place_index_t<k_left_idx>{}, std::move(temp)};
            } else {
                auto temp = std::get<k_left_idx>(m_datum);
                m_datum = DatumVariant<LeftT, RightT>{EitherConsumed{}};
                return NewDatumType{std::in_place_index_t<k_left_idx>{}, temp};
            }
        case k_empty_idx:
            return DatumVariant<LeftT, NewRightType>{EitherEmpty{}};
        case k_consumed_idx:
        case k_right_idx:
        default:
            throw RtError{""};
        }
    }

    DatumVariant<LeftT, RightT> m_datum;
#   if 0
private:
    struct PrivateDummy final {};

    template <typename ... Types>
    constexpr EitherConstructors(PrivateDummy, Types &&... args):
        m_datum(std::forward<Types>(args)...) {}
#   endif
};

#define EnableAllEitherConstructors \
EitherConstructors \
    <LeftT, RightT, \
     EitherDefaultConstructor::enable, \
     EitherCopyConstructor::enable, \
     EitherMoveConstructor::enable>

template <typename LeftT, typename RightT>
class EitherConstructors
    <LeftT, RightT,
     EitherDefaultConstructor::enable,
     EitherCopyConstructor::enable,
     EitherMoveConstructor::disable> :
        EnableAllEitherConstructors
{
public:
    using EnableAllEitherConstructors::EitherConstructors;

    EitherConstructors() = default;

    EitherConstructors(const EitherConstructors &) = default;

    EitherConstructors(EitherConstructors &&) = delete;

    EitherConstructors & operator = (const EitherConstructors &) = default;

    EitherConstructors & operator = (EitherConstructors &&) = delete;
};

template <typename LeftT, typename RightT>
class EitherConstructors
    <LeftT, RightT,
     EitherDefaultConstructor::enable,
     EitherCopyConstructor::disable,
     EitherMoveConstructor::enable> :
        EnableAllEitherConstructors
{
public:
    using EnableAllEitherConstructors::EitherConstructors;

    EitherConstructors() = default;

    EitherConstructors(const EitherConstructors &) = delete;

    EitherConstructors(EitherConstructors &&) = default;

    EitherConstructors & operator = (const EitherConstructors &) = delete;

    EitherConstructors & operator = (EitherConstructors &&) = default;
};

template <typename LeftT, typename RightT>
class EitherConstructors
    <LeftT, RightT,
     EitherDefaultConstructor::enable,
     EitherCopyConstructor::disable,
     EitherMoveConstructor::disable> :
        EnableAllEitherConstructors
{
public:
    using EnableAllEitherConstructors::EitherConstructors;

    EitherConstructors() = default;

    EitherConstructors(const EitherConstructors &) = delete;

    EitherConstructors(EitherConstructors &&) = delete;

    EitherConstructors & operator = (const EitherConstructors &) = delete;

    EitherConstructors & operator = (EitherConstructors &&) = delete;
};

#undef EnableAllEitherConstructors

template <typename LeftT, typename RightT,
          EitherCopyConstructor kt_copy_enabled,
          EitherMoveConstructor kt_move_enabled>
class EitherConstructors
    <LeftT, RightT,
     EitherDefaultConstructor::disable,
     kt_copy_enabled,
     kt_move_enabled> :
        EitherConstructors<LeftT, RightT,
        EitherDefaultConstructor::enable,
        kt_copy_enabled,
        kt_move_enabled>
{
public:
    using EitherConstructors<LeftT, RightT,
        EitherDefaultConstructor::enable,
        kt_copy_enabled,
        kt_move_enabled>::EitherConstructors;

    EitherConstructors() = delete;
};

class EitherHelpersBase {
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

    template <typename LeftT, typename RightT,
        bool kt_is_copy_enabled,
        bool kt_is_move_enabled>
    friend class BaseEitherCopyMove;
};

class FoldAttn final : public DatumVariantUser {
    template <typename LeftT, typename RightT, typename CommonT>
    friend class either::Fold;

    template <typename LeftT, typename RightT>
    friend class cul::Either;

    template <typename LeftT, typename RightT>
    friend class cul::OptionalEither;

    template <typename CommonT>
    using OptionalCommon_ = std::optional<CommonT>;

    template <typename LeftT, typename RightT, typename CommonT>
    static constexpr either::Fold<LeftT, RightT, CommonT>
        make_fold(OptionalCommon_<CommonT> && common,
                  DatumVariant<LeftT, RightT> && datum,
                  EnableForMove<LeftT, RightT, CommonT> = std::monostate{})
    {
        return either::Fold{std::move(common), std::move(datum)};
    }

    template <typename LeftT, typename RightT, typename CommonT>
    static constexpr either::Fold<LeftT, RightT, CommonT>
        make_fold(const OptionalCommon_<CommonT> & common,
                  const DatumVariant<LeftT, RightT> & datum,
                  EnableForCopy<LeftT, RightT, CommonT> = std::monostate{})
    {
        return either::Fold{common, datum};
    }
};

class FoldBase : public EitherHelpersBase {
protected:
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

} // end of detail namespace -> ::cul

} // end of cul namespace
