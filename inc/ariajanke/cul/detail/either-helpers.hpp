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
        AreAllMoveable<HeadTwo, Types...>::k_value;
};

template <typename ... Types>
constexpr const bool kt_are_all_copyable = AreAllCopyable<Types...>::k_value;

template <typename ... Types>
constexpr const bool kt_are_all_moveable = AreAllMoveable<Types...>::k_value;

template <typename Head, typename ... Types>
struct AreAnyCopyable {
    static constexpr const bool k_value = std::is_copy_constructible_v<Head>;
};

template <typename Head, typename HeadTwo, typename ... Types>
struct AreAnyCopyable<Head, HeadTwo, Types...> :
    public AreAllCopyable<HeadTwo, Types...>
{
    static constexpr const bool k_value =
        std::is_copy_constructible_v<Head> ||
        AreAllCopyable<HeadTwo, Types...>::k_value;
};

template <typename Head, typename ... Types>
struct AreAnyMoveable {
    static constexpr const bool k_value = std::is_move_constructible_v<Head>;
};

template <typename Head, typename HeadTwo, typename ... Types>
struct AreAnyMoveable<Head, HeadTwo, Types...> :
    public AreAnyMoveable<HeadTwo, Types...>
{
    static constexpr const bool k_value =
        std::is_move_constructible_v<Head> &&
        AreAnyMoveable<HeadTwo, Types...>::k_value;
};

template <typename T>
constexpr const bool kt_is_void = std::is_same_v<void, T>;

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
            return t;
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
    template <typename OtherLeft, typename OtherRight>
    using EnableMovableFor = std::enable_if_t<
        kt_are_all_moveable<OtherLeft, OtherRight> &&
        std::is_same_v<OtherLeft, LeftT> &&
        std::is_same_v<OtherRight, RightT>,
        std::monostate>;
    template <typename OtherLeft, typename OtherRight>
    using EnableCopyableFor = std::enable_if_t<
        kt_are_all_copyable<OtherLeft, OtherRight> &&
            std::is_same_v<OtherLeft, LeftT> &&
            std::is_same_v<OtherRight, RightT>,
        std::monostate>;
public:

    // ---------------------------- !WARNING! -----------------------------
    //                     exposed to public interface

    static_assert(!kt_is_void<LeftT> , "LeftType may not be void" );
    static_assert(!kt_is_void<RightT>, "RightType may not be void");

    constexpr EitherConstructors(): m_datum(EitherEmpty{}) {}

    constexpr EitherConstructors(const EitherConstructors & rhs):
        m_datum(rhs.m_datum) {}

    constexpr EitherConstructors(EitherConstructors && rhs):
        m_datum(std::move(rhs.m_datum))
    { rhs.mark_as_consumed(); }

    template <typename EitherLeftOfRight>
    constexpr EitherConstructors(EitherLeftOfRight && left_or_right,
                                 EnableMoveForAmbiguousCtor<LeftT, RightT, EitherLeftOfRight> = std::monostate{}):
        m_datum(std::move(left_or_right)) {}


    template <typename OtherLeft, typename OtherRight>
    constexpr EitherConstructors(TypeTag<OtherLeft>, OtherRight && right,
                                 EnableMovableFor<OtherLeft, OtherRight> = std::monostate{}):
        m_datum(std::in_place_index_t<k_right_idx>{}, std::move(right)) {}

    template <typename OtherLeft, typename OtherRight>
    constexpr EitherConstructors(TypeTag<OtherLeft>, const OtherRight & right,
                                 EnableCopyableFor<OtherLeft, OtherRight> = std::monostate{}):
        m_datum(std::in_place_index_t<k_right_idx>{}, right) {}


    template <typename EitherLeftOfRight>
    constexpr EitherConstructors(const EitherLeftOfRight & left_or_right,
                                 EnableCopyForAmbiguousCtor<LeftT, RightT, EitherLeftOfRight> = std::monostate{}):
        m_datum(left_or_right) {}

    template <typename OtherLeft, typename OtherRight>
    constexpr EitherConstructors(OtherLeft && left, TypeTag<OtherRight>,
                                 EnableMovableFor<OtherLeft, OtherRight> = std::monostate{}):
        m_datum(std::in_place_index_t<k_left_idx>{}, std::move(left)) {}

    template <typename OtherLeft, typename OtherRight>
    constexpr EitherConstructors(const OtherLeft & left, TypeTag<OtherRight>,
                                 EnableCopyableFor<OtherLeft, OtherRight> = std::monostate{}):
        m_datum(std::in_place_index_t<k_left_idx>{}, left) {}

    constexpr EitherConstructors & operator = (const EitherConstructors & rhs) {
        if (this != & rhs)
            { m_datum = rhs.m_datum; }
        return *this;
    }

    constexpr EitherConstructors & operator = (EitherConstructors && rhs) {
        if (this != &rhs) {
            m_datum = std::move(rhs.m_datum);
            rhs.mark_as_consumed();
        }
        return *this;
    }

protected:

    template <typename AnyLeft, typename AnyRight>
    friend class cul::Either;

    template <typename AnyLeft, typename AnyRight>
    friend class cul::OptionalEither;

    constexpr bool is_left_() const { return m_datum.index() == k_left_idx; }

    constexpr bool is_right_() const { return m_datum.index() == k_right_idx; }

    constexpr bool is_empty_() const { return m_datum.index() == k_empty_idx; }

    constexpr LeftT left_() {
        if (std::holds_alternative<EitherConsumed>(m_datum)) {
            throw std::runtime_error{""};
        }
        if constexpr (std::is_move_constructible_v<LeftT>) {
            auto t = std::get<k_left_idx>(std::move(m_datum));
            mark_as_consumed();
            return t;
        } else {
            auto t = std::get<k_left_idx>(m_datum);
            mark_as_consumed();
            return t;
        }
    }

    constexpr RightT right_() {        
        if (std::holds_alternative<EitherConsumed>(m_datum)) {
            throw std::runtime_error{""};
        }
        if constexpr (std::is_move_constructible_v<RightT>) {
            auto t = std::get<k_right_idx>(std::move(m_datum));
            mark_as_consumed();
            return t;
        } else {
            auto t = std::get<k_right_idx>(m_datum);
            mark_as_consumed();
            return t;
        }
    }

    template <typename OtherLeft, typename OtherRight>
    constexpr EitherConstructors(DatumVariant<OtherLeft, OtherRight> && datum,
                                 EnableMovableFor<OtherLeft, OtherRight> = std::monostate{}):
        m_datum(std::move(datum)) {}

    template <typename OtherLeft, typename OtherRight>
    constexpr EitherConstructors(const DatumVariant<OtherLeft, OtherRight> & datum,
                                 EnableCopyableFor<OtherLeft, OtherRight> = std::monostate{}):
        m_datum(datum) {}

    template <typename NewLeftType>
    constexpr DatumVariant<NewLeftType, RightT> with_new_left_type_() {
        using namespace exceptions_abbr;
        using NewDatumType = DatumVariant<NewLeftType, RightT>;
        switch (m_datum.index()) {
        case k_right_idx:
            if constexpr (kt_are_all_moveable<LeftT, RightT>) {
                auto temp = std::get<k_right_idx>(std::move(m_datum));
                mark_as_consumed();
                return NewDatumType{std::in_place_index_t<k_right_idx>{}, std::move(temp)};
            } else {
                auto temp = std::get<k_right_idx>(m_datum);
                mark_as_consumed();
                return NewDatumType{std::in_place_index_t<k_right_idx>{}, temp};
            }
        case k_empty_idx:
            return DatumVariant<NewLeftType, RightT>{EitherEmpty{}};
        case k_consumed_idx:
        case k_left_idx:
        default:
            throw RtError{"*Either::with_new_left_type: cannot call on "
                          "left/consumed"};
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
                mark_as_consumed();
                return NewDatumType{std::in_place_index_t<k_left_idx>{}, std::move(temp)};
            } else {
                auto temp = std::get<k_left_idx>(m_datum);
                mark_as_consumed();
                return NewDatumType{std::in_place_index_t<k_left_idx>{}, temp};
            }
        case k_empty_idx:
            return DatumVariant<LeftT, NewRightType>{EitherEmpty{}};
        case k_consumed_idx:
        case k_right_idx:
        default:
            throw RtError{"*Either::with_new_right_type: cannot call on "
                          "right/consumed"};

        }
    }

    constexpr void mark_as_consumed()
        { m_datum = DatumVariant<LeftT, RightT>{EitherConsumed{}}; }

    DatumVariant<LeftT, RightT> m_datum;
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
        public EnableAllEitherConstructors
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
        public EnableAllEitherConstructors
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
        public EnableAllEitherConstructors
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
        public EitherConstructors<LeftT, RightT,
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
        verify_not_consumed(datum);
        return either::Fold{std::move(common), std::move(datum)};
    }

    template <typename LeftT, typename RightT, typename CommonT>
    static constexpr either::Fold<LeftT, RightT, CommonT>
        make_fold(const OptionalCommon_<CommonT> & common,
                  const DatumVariant<LeftT, RightT> & datum,
                  EnableForCopy<LeftT, RightT, CommonT> = std::monostate{})
    {
        verify_not_consumed(datum);
        return either::Fold{common, datum};
    }

    template <typename LeftT, typename RightT>
    static constexpr void verify_not_consumed
        (const DatumVariant<LeftT, RightT> & datum)
    {
        if (!std::holds_alternative<detail::EitherConsumed>(datum))
            { return; }
        using namespace exceptions_abbr;
        throw RtError{"Fold::Fold: May not fold on consumed either"};
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

namespace either {

template <typename LeftType>
class OptionalEitherRightMaker;

template <typename LeftType>
class OptionalEitherLeftMaker;


template <typename LeftType>
class EitherRightMaker;

template <typename LeftType>
class EitherLeftMaker;

template <typename LeftType>
constexpr OptionalEitherRightMaker<LeftType> optional_right();

template <typename LeftType>
constexpr OptionalEitherLeftMaker<LeftType> optional_left(LeftType &&);

template <typename LeftType>
constexpr EitherRightMaker<LeftType> right();

template <typename LeftType>
constexpr EitherLeftMaker<LeftType> left(LeftType &&);

} // end of either namespace -> into ::cul

} // end of cul namespace
