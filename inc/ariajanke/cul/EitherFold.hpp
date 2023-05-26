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

/// temporary for folding an either
template <typename LeftT, typename RightT, typename CommonT>
class Fold final : public detail::DatumVariantUser, public detail::FoldBase {
public:
    using CommonType = CommonT;
    using LeftType = LeftT;
    using RightType = RightT;

    /// transform a right value into the common type
    ///
    /// @param f must take the following form:
    ///          [*anything*] (RightType *any qualifiers*) { return *common type*; }
    template <typename Func>
    constexpr Fold map(Func && f);

    /// transform a left value into the common type
    ///
    /// @param f must take the following form:
    ///          [*anything*] (LeftType *any qualifiers*) { return *common type*; }
    template <typename Func>
    constexpr Fold map_left(Func && f);

    /// retrieves the value resulting from the fold
    ///
    /// @throws if the either was not properly transformed
    constexpr CommonType operator () ()
        { return std::move(m_value.value()); }

    /// retrieves the value resulting from the fold
    ///
    /// @throws if the either was not properly transformed
    constexpr CommonType value()
        { return std::move(m_value.value()); }

// -------------------------- END OF PUBLIC INTERFACE -------------------------

private:
    using OptionalCommon = std::optional<CommonT>;
    friend class ::cul::detail::FoldAttn;

    constexpr Fold(OptionalCommon && common,
                   DatumVariant<LeftT, RightT> && datum):
        m_value(std::move(common)),
        m_datum(std::move(datum )) {}

    template <typename OtherCommonT, typename OtherLeftType, typename OtherRightType>
    constexpr Fold(const std::optional<OtherCommonT> & common,
                   const DatumVariant<LeftT, RightT> & datum,
                   std::enable_if_t<
                       detail::kt_are_all_copyable<OtherCommonT, OtherLeftType, OtherRightType> &&
                       std::is_same_v<OtherCommonT, CommonT> &&
                       std::is_same_v<OtherLeftType, LeftT> &&
                       std::is_same_v<OtherRightType, RightT>, std::monostate> = std::monostate{}):
        m_value(common),
        m_datum(datum ) {}

    OptionalCommon m_value;
    DatumVariant<LeftT, RightT> m_datum;
};

// ----------------------------------------------------------------------------

template <typename LeftT, typename RightT, typename CommonT>
template <typename Func>
constexpr Fold<LeftT, RightT, CommonT>
    Fold<LeftT, RightT, CommonT>::map(Func && f)
{
    VerifyFoldFunctionForSide<CommonT, Func, RightType>{};
    if (m_datum.index() == k_right_idx) {
        OptionalCommon rv = f(consume_datum<k_right_idx>(m_datum));
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
    if (m_datum.index() == k_left_idx) {
        OptionalCommon rv = f(consume_datum<k_left_idx>(m_datum));
        return Fold{std::move(rv), std::move(m_datum)};
    }
    return Fold{std::move(m_value), std::move(m_datum)};
}

} // end of either namespace -> into ::cul

} // end of cul namespace
