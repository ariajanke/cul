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

#include <ariajanke/cul/detail/either-helpers.hpp>
#include <ariajanke/cul/EitherFold.hpp>

namespace cul {

/// An optional either, is an either with a "third not present" state.
///
/// Care was taken to reduce exceptions as much as possible. Unfortunately
/// there's no way to guard against incorrect usage. If an exception is *ever*
/// thrown, then that indicates that the either is being used incorrectly by
/// the client.
///
/// @warning There is a fourth hidden state called "consumed". An either is
///          considered "consumed" if it has been moved. A consumed either is
///          not usable and will throw on any attempt to use it. The client is
///          expected to not use/refer to an either when they are finished
///          with it.
template <typename LeftT, typename RightT>
class OptionalEither final : public detail::EitherBase {
    template <typename EitherLeftOrRight>
    using EnableIfLeftXorRightPtr =
        EnableIfLeftXorRightPtr_<LeftT, RightT, EitherLeftOrRight>;
public:
    using LeftType = LeftT;
    using RightType = RightT;

    static_assert(!kt_is_void<LeftType> , "LeftType may not be void" );
    static_assert(!kt_is_void<RightType>, "RightType may not be void");

    /// Construct an empty either, which contains neither a left or right.
    ///
    /// This makes optional either special, it maybe empty.
    constexpr OptionalEither(): m_datum() {}

    /// Constructs a right or left based on the type.
    ///
    /// available when left and right types are distinct
    template <typename EitherLeftOrRight>
    constexpr OptionalEither(EitherLeftOrRight && left_or_right,
                             EnableIfLeftXorRightPtr<EitherLeftOrRight> = nullptr);

    /// unambiguously constructs a right using typetag as a stand in for the
    /// left type
    constexpr OptionalEither(TypeTag<LeftType>, RightType && right_);

    /// unambiguously constructs a left using typetag as a stand in for the
    /// right type
    constexpr OptionalEither(LeftType && left_, TypeTag<RightType>);

    constexpr OptionalEither(OptionalEither && rhs);

    constexpr OptionalEither(const OptionalEither & rhs);

    constexpr OptionalEither & operator = (OptionalEither && rhs);

    constexpr OptionalEither & operator = (const OptionalEither & rhs);

    /// @note either is consumed after this call
    template <typename Func>
    [[nodiscard]] constexpr EnableIfReturnsOptionalEither<Func>
        chain(Func && f);

    /// @note either is consumed after this call
    template <typename Func>
    [[nodiscard]] constexpr EnableIfReturnsOptionalEither<Func>
        chain_left(Func && f);

    /// @note either is consumed after this call
    template <typename CommonT>
    [[nodiscard]] constexpr either::Fold<LeftT, RightT, CommonT> fold
        (CommonT && default_value);

    /// @returns true if the either contains a left value
    constexpr bool is_left() const;

    /// @returns true if the either contains a right value
    constexpr bool is_right() const;

    /// @returns true if the optional either contains no value
    constexpr bool is_empty() const;

    /// @returns contained left value.
    ///
    /// @note either is consumed after this call
    /// @throws if the either does not contain a left
    constexpr LeftT left();

    /// Transforms the right value if it exists according to the given function.
    ///
    /// @param f given function that transforms the right value, must take the
    ///          following form: \n
    ///         [*anything*] (RightType *any qualifier*) { return *any*; }
    /// @returns a new either containing the transformed value if the previous
    ///          either contained a right
    /// @note either is consumed after this call
    template <typename Func>
    [[nodiscard]] constexpr OptionalEither<LeftType, ReturnTypeOf<Func>>
        map(Func && f);

    /// @note either is consumed after this call
    template <typename Func>
    [[nodiscard]] constexpr OptionalEither<ReturnTypeOf<Func>, RightType>
        map_left(Func && f);

    /// @returns contained right value
    ///
    /// @note either is consumed after this call
    /// @throws if the either does not contain a right
    constexpr RightT right();

// -------------------------- PAUSE PUBLIC INTERFACE -------------------------

private:
    using BareEither_ = BareEither<LeftT, RightT>;

    template <typename OtherLeft, typename OtherRight>
    friend class ::cul::OptionalEither;

    friend class cul::detail::OptionalEitherToEitherAttn;
    friend class cul::detail::FoldAttn;

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

    constexpr explicit OptionalEither(BareEither_ &&);

    template <typename NewRightType>
    constexpr OptionalEither<LeftType, NewRightType> with_new_right_type();

    template <typename NewLeftType>
    constexpr OptionalEither<NewLeftType, RightType> with_new_left_type();

    BareEither_ m_datum;
};

// ------------------------- PUBLIC INTERFACE CONTINUES -----------------------

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

// -------------------------- END OF PUBLIC INTERFACE -------------------------

template <typename LeftT, typename RightT>
template <typename EitherLeftOrRight>
constexpr OptionalEither<LeftT, RightT>::OptionalEither
    (EitherLeftOrRight && obj,
     EnableIfLeftXorRightPtr<EitherLeftOrRight>):
    m_datum(std::move(obj)) {}

template <typename LeftT, typename RightT>
constexpr OptionalEither<LeftT, RightT>::OptionalEither
    (TypeTag<LeftType>, RightType && right_):
    m_datum(InPlaceRight{}, std::move(right_)) {}

template <typename LeftT, typename RightT>
constexpr OptionalEither<LeftT, RightT>::OptionalEither
    (LeftType && left_, TypeTag<RightType>):
    m_datum(InPlaceLeft{}, std::move(left_)) {}

template <typename LeftT, typename RightT>
constexpr OptionalEither<LeftT, RightT>::OptionalEither(OptionalEither && rhs):
    m_datum(std::move(rhs.m_datum)) {}

template <typename LeftT, typename RightT>
constexpr OptionalEither<LeftT, RightT>::OptionalEither(const OptionalEither & rhs):
    m_datum(rhs.m_datum) {}

template <typename LeftT, typename RightT>
constexpr OptionalEither<LeftT, RightT> &
    OptionalEither<LeftT, RightT>::operator = (OptionalEither && rhs)
{
    if (this != &rhs) {
        m_datum = std::move(rhs.m_datum);
    }
    return *this;
}

template <typename LeftT, typename RightT>
constexpr OptionalEither<LeftT, RightT> &
    OptionalEither<LeftT, RightT>::operator = (const OptionalEither & rhs)
{
    if (this != &rhs) {
        m_datum = rhs.m_datum;
    }
    return *this;
}

template <typename LeftT, typename RightT>
template <typename Func>
[[nodiscard]] constexpr
    OptionalEither<LeftT,
                   typename OptionalEither<LeftT, RightT>::
                       template ReturnTypeOf<Func>>
    OptionalEither<LeftT, RightT>::map(Func && f)
{
    using NewRight = ReturnTypeOf<Func>;
    static_assert(!std::is_same_v<void, NewRight>,
                  "Return type of given function may not be void");
    VerifyArgumentsForSide<Func, RightType>{};
    if (is_right())
        return OptionalEither<LeftType, NewRight>{TypeTag<LeftType>{}, f(right())};
    return with_new_right_type<NewRight>();
}

template <typename LeftT, typename RightT>
template <typename Func>
[[nodiscard]] constexpr
    OptionalEither<typename OptionalEither<LeftT, RightT>::
                       template ReturnTypeOf<Func>,
                   RightT>
    OptionalEither<LeftT, RightT>::map_left(Func && f)
{
    using NewLeft = ReturnTypeOf<Func>;
    static_assert(!std::is_same_v<void, NewLeft>,
                  "Return type of given function may not be void");
    VerifyArgumentsForSide<Func, LeftType>{};
    if (is_left())
        return OptionalEither<NewLeft, RightType>{f(left()), TypeTag<RightType>{}};
    return with_new_left_type<NewLeft>();
}

template <typename LeftT, typename RightT>
template <typename CommonT>
[[nodiscard]] constexpr
    either::Fold<LeftT, RightT, CommonT>
    OptionalEither<LeftT, RightT>::fold
    (CommonT && default_value)
{
    return detail::FoldAttn::make_fold<LeftT, RightT, CommonT>
        (std::move(default_value), std::move(m_datum));
}

template <typename LeftT, typename RightT>
template <typename Func>
[[nodiscard]] constexpr
    typename OptionalEither<LeftT, RightT>::
        template EnableIfReturnsOptionalEither<Func>
    OptionalEither<LeftT, RightT>::chain(Func && f)
{
    RightChainFunctionReturnRequirements<Func>{};
    VerifyArgumentsForSide<Func, RightType>{};
    if (is_right())
        { return f(right()); }
    return with_new_right_type<EitherTypeRight<ReturnTypeOf<Func>>>();
}

template <typename LeftT, typename RightT>
template <typename Func>
[[nodiscard]] constexpr
    typename OptionalEither<LeftT, RightT>::
        template EnableIfReturnsOptionalEither<Func>
    OptionalEither<LeftT, RightT>::chain_left(Func && f)
{
    LeftChainFunctionReturnRequirements<Func>{};
    VerifyArgumentsForSide<Func, LeftType>{};
    if (is_left())
        { return f(left()); }
    return with_new_left_type<EitherTypeLeft<ReturnTypeOf<Func>>>();
}

template <typename LeftT, typename RightT>
constexpr LeftT OptionalEither<LeftT, RightT>::left()
    { return m_datum.left(); }

template <typename LeftT, typename RightT>
constexpr RightT OptionalEither<LeftT, RightT>::right()
    { return m_datum.right(); }

template <typename LeftT, typename RightT>
constexpr bool OptionalEither<LeftT, RightT>::is_left() const
    { return m_datum.is_left(); }

template <typename LeftT, typename RightT>
constexpr bool OptionalEither<LeftT, RightT>::is_right() const
    { return m_datum.is_right(); }

template <typename LeftT, typename RightT>
constexpr bool OptionalEither<LeftT, RightT>::is_empty() const
    { return m_datum.is_empty(); }

template <typename LeftT, typename RightT>
constexpr /* private */ OptionalEither<LeftT, RightT>::OptionalEither
    (BareEither_ && either_datum_):
    m_datum(std::move(either_datum_)) {}

template <typename LeftType, typename RightType>
template <typename NewRightType>
constexpr /* private */ OptionalEither<LeftType, NewRightType>
    OptionalEither<LeftType, RightType>::with_new_right_type()
{
    return OptionalEither<LeftType, NewRightType>
        {m_datum.template with_new_right_type<NewRightType>()};
}

template <typename LeftType, typename RightType>
template <typename NewLeftType>
constexpr /* private */ OptionalEither<NewLeftType, RightType>
    OptionalEither<LeftType, RightType>::with_new_left_type()
{
    return OptionalEither<NewLeftType, RightType>
        {m_datum.template with_new_left_type<NewLeftType>()};
}

} // end of cul namespace
