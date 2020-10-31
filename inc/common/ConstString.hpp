/****************************************************************************

    File: ConstString.hpp
    Author: Aria Janke
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

#include <string>

/** This is a c-string class wrapper. It maybe treated much like std::string.
 *  This class is essentially syntax sugar. I find that this "sugar" is better
 *  This class is out-moded by string_view which is too recent for my projects.
 *  :c
 *  at making code easier to read, so in my mind at least it is not "sugar".
 *  @note "sugar" carries a negative conntation, and IMO does not belong in
 *        discussions about readability.
 */
class ConstString {
public:
    ConstString(): m_str("") {}
    ConstString(const char * const str_): m_str(str_) {}

#   define MACRO_OP_DEFINE(tok) \
    bool operator tok (const ConstString & lhs) const \
        { return operator tok (lhs.m_str); }

    MACRO_OP_DEFINE(==) MACRO_OP_DEFINE(!=)
    MACRO_OP_DEFINE(<=) MACRO_OP_DEFINE(>=)
    MACRO_OP_DEFINE(< ) MACRO_OP_DEFINE( >)

#   undef MACRO_OP_DEFINE

#   define MACRO_OP_DEFINE(tok) \
    bool operator tok (const std::string & lhs) const \
        { return operator tok (lhs.c_str()); }

    MACRO_OP_DEFINE(==) MACRO_OP_DEFINE(!=)
    MACRO_OP_DEFINE(<=) MACRO_OP_DEFINE(>=)
    MACRO_OP_DEFINE(< ) MACRO_OP_DEFINE( >)

#   undef MACRO_OP_DEFINE

    bool operator == (const char * lhs) const
        { return compare_result(ConstString(lhs)) == 0; }

    bool operator != (const char * lhs) const
        { return compare_result(ConstString(lhs)) != 0; }

    bool operator <= (const char * lhs) const
        { return compare_result(ConstString(lhs)) <= 0; }

    bool operator >= (const char * lhs) const
        { return compare_result(ConstString(lhs)) >= 0; }

    bool operator <  (const char * lhs) const
        { return compare_result(ConstString(lhs)) < 0; }

    bool operator >  (const char * lhs) const
        { return compare_result(ConstString(lhs)) > 0; }

    const char * as_cstring() const noexcept { return m_str; }

    // "this - rhs"
    int compare_result(const ConstString & rhs) const noexcept;

    unsigned length() const noexcept;
private:
    const char * m_str;
};

inline bool operator == (const char * rhs, const ConstString & lhs)
    { return lhs == rhs; }

inline bool operator != (const char * rhs, const ConstString & lhs)
    { return lhs != rhs; }

inline bool operator <= (const char * rhs, const ConstString & lhs)
    { return lhs > rhs; }

inline bool operator >= (const char * rhs, const ConstString & lhs)
    { return lhs < rhs; }

inline bool operator < (const char * rhs, const ConstString & lhs)
    { return lhs >= rhs; }

inline bool operator > (const char * rhs, const ConstString & lhs)
    { return lhs <= rhs; }
