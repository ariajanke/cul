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
#include <ariajanke/cul/Util.hpp>
#include <ariajanke/cul/EitherFold.hpp>

namespace cul {

/// An implementation of the either monad.
///
/// Care was taken to reduce exceptions as much as possible. Unfortunately
/// there's no way to guard against incorrect usage. If an exception is *ever*
/// thrown, then that indicates that the either is being used incorrectly by
/// the client.
template <typename LeftT, typename RightT>
class Either final :
    public detail::EitherConstructors
        <LeftT, RightT, detail::EitherDefaultConstructor::disable>,
    public detail::EitherHelpersBase
{
    using Super = detail::EitherConstructors
        <LeftT, RightT, detail::EitherDefaultConstructor::disable>;
public:
    using LeftType = LeftT;
    using RightType = RightT;
    using Super::Super;

    /// Transforms right value into an either preserving the left side's type.
    ///
    /// If this either is not a right, then this either's type is transformed
    /// into Either<LeftType, *AnyNewRightType*> and is returned.
    /// @note either is consumed after this call
    /// @param f given function that transforms the right value, must take the
    ///          following form: \n
    ///         [*anything*] (RightType *any qualifier*)
    ///         { return Either<LeftType, *AnyNewRightType*>{...}; }
    /// @returns instance of Either<LeftType, *AnyNewRightType*>
    template <typename Func>
    [[nodiscard]] constexpr EnableIfReturnsEither<Func> chain(Func && f);

    /// Transforms left value into an either preserving the right side's type.
    ///
    /// If this either is not a left, then this either's type is transformed
    /// into Either<*AnyNewLeftType*, RightType> and is returned.
    /// @note either is consumed after this call
    /// @param f given function that transforms the left value, must take the
    ///          following form: \n
    ///         [*anything*] (LeftType *any qualifier*)
    ///         { return Either<*AnyNewLeftType*, RightType>{...}; }
    /// @returns instance of Either<*AnyNewLeftType*, RightType>
    template <typename Func>
    [[nodiscard]] constexpr EnableIfReturnsEither<Func> chain_left(Func && f);

    template <typename CommonT>
    [[nodiscard]] constexpr either::Fold<LeftType, RightType, CommonT> fold();

    /// @returns true if the either contains a left value
    constexpr bool is_left() const;

    /// @returns true if the either contains a right value
    constexpr bool is_right() const;

    /// @returns contained left value.
    ///
    /// @note either is consumed after this call
    /// @throws if the either does not contain a left
    constexpr LeftT left();

    /// @returns contained left value, or the specified default.
    ///
    /// @param default_ casted to left type and returned if this is not a
    ///                 left
    /// @note either is consumed after this call if it is a left
    template <typename U>
    constexpr LeftT left_or(U && obj)
        { return is_left() ? left() : static_cast<LeftT>(std::move(obj)); }

    /// @returns contained left value, or a default value returned by a given
    ///          function.
    ///
    /// @param f function to call if this either is not a left, it must take
    ///          no arguments
    /// @note either is consumed after this call if it is a left
    template <typename Func>
    constexpr LeftT left_or_call(Func && f)
        { return is_left() ? left() : static_cast<LeftT>(f()); }

    /// Transforms the right value if it exists according to the given function.
    ///
    /// @param f given function that transforms the right value, must take the
    ///          following form: \n
    ///         [*anything*] (RightType *any qualifier*) { return *any*; }
    /// @returns a new either containing the transformed value if the previous
    ///          either contained a right
    /// @note either is consumed after this call
    template <typename Func>
    [[nodiscard]] constexpr Either<LeftType, ReturnTypeOf<Func>>
        map(Func && f);

    /// Transforms the left value if it exists according to the given function.
    ///
    /// @param f given function that transforms the left value, must take the
    ///          following form: \n
    ///         [*anything*] (LeftType *any qualifier*) { return *any*; }
    /// @returns a new either containing the transformed value if the previous
    ///          either contained a left
    /// @note either is consumed after this call
    template <typename Func>
    [[nodiscard]] constexpr Either<ReturnTypeOf<Func>, RightType>
        map_left(Func && f);

    /// @returns contained right value
    ///
    /// @note either is consumed after this call
    /// @throws if the either does not contain a right
    constexpr RightT right();

    /// @returns contained right value, or the specified default.
    ///
    /// @param default_ casted to right type and returned if this is not a
    ///                 right
    /// @note either is consumed after this call if it is a right
    template <typename U>
    constexpr RightT right_or(U && obj)
        { return is_right() ? right() : static_cast<RightT>(std::move(obj)); }

    /// @returns contained right value, or a default value returned by a given
    ///          function.
    ///
    /// @param f function to call if this either is not a right, it must take
    ///          no arguments
    /// @note either is consumed after this call if it is a right
    template <typename Func>
    constexpr RightT right_or_call(Func && f)
        { return is_right() ? right() : static_cast<RightT>(f()); }

// -------------------------- PAUSE PUBLIC INTERFACE -------------------------

private:
    static constexpr OptionalEither<LeftT, RightT> verify_non_empty
        (const char * caller, OptionalEither<LeftT, RightT> && opt_either);

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
    constexpr Either<LeftType, NewRightType> with_new_right_type();

    template <typename NewLeftType>
    constexpr Either<NewLeftType, RightType> with_new_left_type();
};

// ------------------------- PUBLIC INTERFACE CONTINUES -----------------------

namespace either {

/// temporary returned by cul::either::left
template <typename LeftType>
class EitherRightMaker final {
public:
    /// @returns an either initialized with a right value
    ///
    /// @param obj initial right value
    template <typename RightType>
    constexpr Either<LeftType, RightType> with(RightType && right) const
        { return Either<LeftType, RightType>{TypeTag<LeftType>{}, std::move(right)}; }

private:
    EitherRightMaker() {}

    template <typename LeftType2>
    friend constexpr EitherRightMaker<LeftType2> right();
};

/// temporary returned by cul::either::right
template <typename LeftType>
class EitherLeftMaker final {
public:
    /// @returns an either initialized with a right value
    ///
    /// @param obj initial right value
    template <typename RightType>
    constexpr Either<LeftType, RightType> with()
        { return Either<LeftType, RightType>{std::move(m_obj), TypeTag<RightType>{}}; }

private:
    explicit constexpr EitherLeftMaker(LeftType && left_obj):
        m_obj(std::move(left_obj)) {}

    template <typename LeftType2>
    friend constexpr EitherLeftMaker<LeftType2> left(LeftType &&);

    LeftType m_obj;
};

/// begins construction of a either with a right value
///
/// @note this is the preferred method of constructing an either
template <typename LeftType>
constexpr EitherRightMaker<LeftType> right()
    { return EitherRightMaker<LeftType>{}; }

/// begins construction of a either with a right value
///
/// @param obj initial right value
/// @note this is the preferred method of constructing an either
template <typename LeftType>
constexpr EitherLeftMaker<LeftType> left(LeftType && obj)
    { return EitherLeftMaker<LeftType>{std::move(obj)}; }

} // end of either namespace -> into ::cul

// -------------------------- END OF PUBLIC INTERFACE -------------------------

template <typename LeftT, typename RightT>
template <typename CommonT>
constexpr either::Fold<LeftT, RightT, CommonT>
    Either<LeftT, RightT>::fold()
{
    auto t = detail::FoldAttn::make_fold<LeftType, RightType, CommonT>
        (std::optional<CommonT>{}, std::move(this->m_datum));
    this->mark_as_consumed();
    return t;
}

template <typename LeftT, typename RightT>
template <typename Func>
[[nodiscard]] constexpr
    Either<LeftT,
           typename detail::EitherHelpersBase::template ReturnTypeOf<Func>>
    Either<LeftT, RightT>::map(Func && f)
{
    using NewRight = ReturnTypeOf<Func>;
    static_assert(!std::is_same_v<void, NewRight>,
                  "Return type of given function may not be void");
    (void)VerifyArgumentsForSide<Func, RightType>{};
    if (is_right())
        return Either<LeftType, NewRight>{TypeTag<LeftType>{}, f(right())};
    return with_new_right_type<NewRight>();
}

template <typename LeftT, typename RightT>
template <typename Func>
[[nodiscard]] constexpr
    Either<typename detail::EitherHelpersBase::template ReturnTypeOf<Func>,
           RightT>
    Either<LeftT, RightT>::map_left(Func && f)
{
    using NewLeft = ReturnTypeOf<Func>;
    static_assert(!std::is_same_v<void, NewLeft>,
                  "Return type of given function may not be void");
    (void)VerifyArgumentsForSide<Func, LeftType>{};
    if (is_left())
        return Either<NewLeft, RightType>{f(left()), TypeTag<RightType>{}};
    return with_new_left_type<NewLeft>();
}

template <typename LeftT, typename RightT>
template <typename Func>
[[nodiscard]] constexpr
    typename detail::EitherHelpersBase::template EnableIfReturnsEither<Func>
    Either<LeftT, RightT>::chain(Func && f)
{
    (void)RightChainFunctionReturnRequirements<Func>{};
    (void)VerifyArgumentsForSide<Func, RightType>{};
    if (is_right())
        { return f(right()); }
    return with_new_right_type<EitherTypeRight<ReturnTypeOf<Func>>>();
}

template <typename LeftT, typename RightT>
template <typename Func>
[[nodiscard]] constexpr
    typename detail::EitherHelpersBase::template EnableIfReturnsEither<Func>
    Either<LeftT, RightT>::chain_left(Func && f)
{
    (void)LeftChainFunctionReturnRequirements<Func>{};
    (void)VerifyArgumentsForSide<Func, LeftType>{};
    if (is_left())
        { return f(left()); }
    return with_new_left_type<EitherTypeLeft<ReturnTypeOf<Func>>>();
}

template <typename LeftT, typename RightT>
constexpr LeftT Either<LeftT, RightT>::left()
    { return this->left_(); }

template <typename LeftT, typename RightT>
constexpr RightT Either<LeftT, RightT>::right()
    { return this->right_(); }

template <typename LeftT, typename RightT>
constexpr bool Either<LeftT, RightT>::is_left() const
    { return this->is_left_(); }

template <typename LeftT, typename RightT>
constexpr bool Either<LeftT, RightT>::is_right() const
    { return this->is_right_(); }

template <typename LeftT, typename RightT>
/* private static */ constexpr OptionalEither<LeftT, RightT>
    Either<LeftT, RightT>::verify_non_empty
    (const char * caller, OptionalEither<LeftT, RightT> && opt_either)
{
    using namespace exceptions_abbr;
    if (!opt_either.is_empty()) return std::move(opt_either);
    throw RtError{"Either::" + std::string{caller} +
                  ": given optional either must not be empty."};
}

template <typename LeftT, typename RightT>
template <typename NewRightType>
constexpr /* private */ Either<LeftT, NewRightType>
    Either<LeftT, RightT>::with_new_right_type()
{
    return Either<LeftType, NewRightType>
        { Super::template with_new_right_type_<NewRightType>() };
}

template <typename LeftT, typename RightT>
template <typename NewLeftType>
constexpr /* private */ Either<NewLeftType, RightT>
    Either<LeftT, RightT>::with_new_left_type()
{
    return Either<NewLeftType, RightT>
        { Super::template with_new_left_type_<NewLeftType>() };
}

// ----------------------------------------------------------------------------

template <typename LeftT, typename RightT>
constexpr Either<LeftT, RightT> OptionalEither<LeftT, RightT>::require() {
    if (std::holds_alternative<detail::EitherEmpty   >(this->m_datum) ||
        std::holds_alternative<detail::EitherConsumed>(this->m_datum)   )
    {
        using namespace exceptions_abbr;
        throw RtError{"OptionalEither::require: cannot require on a "
                      "consumed/empty either"};
    }
    auto t = std::move(this->m_datum);
    this->mark_as_consumed();
    return Either<LeftT, RightT>{std::move(t)};
}

} // end of cul namespace
