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

public:

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
    static constexpr OptionalEither<LeftT, RightT> verify_non_empty
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
        m_obj(left_obj) {}

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

} // end of either namespace -> into ::cul

} // end of cul namespace
