/****************************************************************************

    File: BitMaskable.hpp
    Author: Andrew Janke
    License: GPLv3

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*****************************************************************************/

#pragma once

#include <common/TypeList.hpp>

template <typename T>
struct CompoundBitwise {
    friend T & operator |= (T & lhs, const T & rhs) { return lhs = lhs | rhs; }
    friend T & operator &= (T & lhs, const T & rhs) { return lhs = lhs & rhs; }
    friend T & operator ^= (T & lhs, const T & rhs) { return lhs = lhs ^ rhs; }
protected:
    CompoundBitwise(){}
};

template <typename T>
struct InequalityOperator {
    friend bool operator != (const T & lhs, const T & rhs)
        { return !(lhs == rhs); }
protected:
    InequalityOperator(){}
};

template <typename ... Types>
class BitMaskable;

// test: semantics, value consistency
template <typename ... Types>
class BitMaskable :
    public CompoundBitwise<BitMaskable<Types...>>,
    public InequalityOperator<BitMaskable<Types...>>
{
public:
    template <typename T>
    friend typename std::enable_if<TypeList<Types...>::template HasType<T>::VALUE, T>::
    type cast_bit_maskable_to(const BitMaskable<Types...> &);

    BitMaskable(): m_state(0) {}

    template <typename T>
    using HasType = typename TypeList<Types...>::template HasType<T>;

    template <typename T,
              typename = typename std::enable_if<HasType<T>::VALUE>::type>
    BitMaskable(T t): m_state(static_cast<int>(t)) {}

    explicit operator bool() const { return m_state != 0; }
    explicit operator int () const { return m_state;      }

    BitMaskable operator | (const BitMaskable & rhs) const
        { return m_state | rhs.m_state; }

    BitMaskable operator & (const BitMaskable & rhs) const
        { return m_state & rhs.m_state; }

    BitMaskable operator ^ (const BitMaskable & rhs) const
        { return m_state ^ rhs.m_state; }

    bool operator == (const BitMaskable & rhs) const
        { return m_state == rhs.m_state; }

protected:
    BitMaskable(int m_): m_state(m_) {}
private:
    int m_state;
};

template <typename T, typename ... Types>
    typename std::enable_if<TypeList<Types...>::template HasType<T>::VALUE, T>::
type cast_bit_maskable_to(const BitMaskable<Types...> & bm) {
    static_assert(!std::is_same<T, int>::value, "");
    return T(bm.m_state);
}
