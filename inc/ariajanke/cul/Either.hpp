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
#   if 0
private:
    template <typename EitherLeftOrRight>
    using EnableIfLeftXorRightPtr =
        EnableIfLeftXorRightPtr_<LeftType, RightType, EitherLeftOrRight>;
    using EnableIfCopyConstructible_ =
        std::enable_if_t<std::is_copy_constructible_v<LeftT> &&
                         std::is_copy_constructible_v<RightT>,
                         const Either &>;
#endif
public:
#   if 0
    constexpr explicit Either(OptionalEither<LeftT, RightT> && opt_either);

    template <typename EitherLeftOrRight>
    constexpr Either(EitherLeftOrRight && obj,
                     EnableIfLeftXorRightPtr<EitherLeftOrRight> = nullptr);

    // more move related tests with unique pointers
    // vector of eithers of unique pointers?
    // how do vector of unique pointers normally work?
    constexpr Either(TypeTag<LeftType>, RightType && right);

    constexpr Either(LeftType && left, TypeTag<RightType>);

    constexpr Either(EnableIfCopyConstructible_ rhs):
        m_datum(rhs.m_datum) {}
#   endif
#   if 0
    template <typename ... Types>
    static constexpr Either<LeftT, RightT> make_left(Types &&...);

    template <typename ... Types>
    static constexpr Either<LeftT, RightT> make_right(Types &&...);
#   endif
    template <typename CommonT>
    constexpr either::Fold<LeftType, RightType, CommonT> fold();

    template <typename Func>
    [[nodiscard]] constexpr Either<LeftType, ReturnTypeOf<Func>>
        map(Func && f);

    template <typename Func>
    [[nodiscard]] constexpr Either<ReturnTypeOf<Func>, RightType>
        map_left(Func && f);

    template <typename Func>
    [[nodiscard]] constexpr EnableIfReturnsEither<Func> chain(Func && f);

    template <typename Func>
    [[nodiscard]] constexpr EnableIfReturnsEither<Func> chain_left(Func && f);

    constexpr LeftT left();

    constexpr RightT right();

    constexpr bool is_left() const;

    constexpr bool is_right() const;

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

namespace either {

template <typename LeftType>
class EitherRightMaker final {
public:
    template <typename RightType>
    constexpr Either<LeftType, RightType> with(RightType && right) const
        { return Either<LeftType, RightType>{TypeTag<LeftType>{}, std::move(right)}; }

    template <typename RightType>
    constexpr Either<LeftType, RightType> operator() (RightType && right) const
        { return with<RightType>(std::move(right)); }
};

template <typename LeftType>
class EitherLeftMaker final {
public:
    explicit constexpr EitherLeftMaker(LeftType && left_obj):
        m_obj(std::move(left_obj)) {}

    template <typename RightType>
    constexpr Either<LeftType, RightType> with()
        { return Either<LeftType, RightType>{std::move(m_obj), TypeTag<RightType>{}}; }

private:
    LeftType m_obj;
};

template <typename LeftType>
constexpr EitherRightMaker<LeftType> right() {
    return EitherRightMaker<LeftType>{};
}

template <typename LeftType>
constexpr EitherLeftMaker<LeftType> left(LeftType && obj) {
    return EitherLeftMaker<LeftType>{std::move(obj)};
}
#if 0 // might not be necessary?
// feature request
// include implicitly convertible wrappers
template <typename LeftType>
class AsLeft final {
public:
};

template <typename LeftType>
AsLeft<LeftType> as_left(LeftType && obj);
#endif
} // end of either namespace -> into ::cul

// ----------------------------------------------------------------------------

#if 0
template <typename LeftT, typename RightT>
template <typename ... Types>
/* static */ constexpr Either<LeftT, RightT>
    Either<LeftT, RightT>::make_left(Types &&... args)
{

}

template <typename LeftT, typename RightT>
template <typename ... Types>
static constexpr OptionalEither<LeftT, RightT> make_right(Types &&...);

template <typename LeftT, typename RightT>
constexpr Either<LeftT, RightT>::
    Either(OptionalEither<LeftT, RightT> && opt_either):
    m_datum(verify_non_empty("Either", std::move(opt_either))) {}

template <typename LeftT, typename RightT>
template <typename EitherLeftOrRight>
constexpr Either<LeftT, RightT>::Either
    (EitherLeftOrRight && obj,
     EnableIfLeftXorRightPtr<EitherLeftOrRight>):
    m_datum(std::move(obj)) {}

template <typename LeftT, typename RightT>
constexpr Either<LeftT, RightT>::Either(TypeTag<LeftType>, RightType && right):
    m_datum(TypeTag<LeftType>{}, std::move(right)) {}

template <typename LeftT, typename RightT>
constexpr Either<LeftT, RightT>::Either(LeftType && left, TypeTag<RightType>):
    m_datum(std::move(left), TypeTag<RightType>{}) {}
#endif
template <typename LeftT, typename RightT>
template <typename CommonT>
constexpr either::Fold<LeftT, RightT, CommonT>
    Either<LeftT, RightT>::fold()
{
    return detail::FoldAttn::make_fold<LeftType, RightType, CommonT>
        (std::optional<CommonT>{}, std::move(this->m_datum));
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
    if (std::holds_alternative<detail::EitherEmpty>(this->m_datum) ||
        std::holds_alternative<detail::EitherConsumed>(this->m_datum))
    {
        throw std::runtime_error{""};
    }
    auto t = std::move(this->m_datum);
    this->m_datum = detail::DatumVariantUser::DatumVariant<LeftT, RightT>{detail::EitherConsumed{}};
    return Either<LeftT, RightT>{std::move(t)};
}

} // end of cul namespace
