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

namespace cul {

namespace either {

// feature request, allow folding to void
template <typename LeftT, typename RightT, typename CommonT>
class Fold final : public detail::FoldBase {
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

    constexpr CommonType value() const
        { return m_value.value(); }

// -------------------------- END OF PUBLIC INTERFACE -------------------------

private:
    using OptionalCommon = std::optional<CommonT>;
    friend class ::cul::detail::FoldAttn;

    constexpr Fold
        (OptionalCommon && default_value, BareEither<LeftT, RightT> && datum):
        m_value(std::move(default_value)),
        m_datum(std::move(datum)) {}

    OptionalCommon m_value;
    BareEither<LeftT, RightT> m_datum;
};

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

} // end of either namespace -> into ::cul

} // end of cul namespace
