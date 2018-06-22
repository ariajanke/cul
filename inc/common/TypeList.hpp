/****************************************************************************

    File: TypeList.hpp
    Author: Andrew "Kip" Janke
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

#include <type_traits>
#include <bitset>
#include <stdexcept>

template <bool TF_VAL, int A, int B>
struct ConstIntSelect { static const constexpr int VALUE = A; };

template <int A, int B>
struct ConstIntSelect<false, A, B> { static const constexpr int VALUE = B; };

template <typename T1, typename ... Types>
struct UniqueTypes {
    static constexpr const bool VALUE = true;
};

template <typename T1, typename T2, typename ... Types>
struct UniqueTypes<T1, T2, Types...> : UniqueTypes<T2, Types...> {
    static constexpr const bool VALUE =
        !std::is_same<T1, T2>::value &&
        UniqueTypes<T2, Types...>::VALUE && UniqueTypes<T1, Types...>::VALUE;
};

template <typename T> struct TypeTag {};

template <typename ... Types>
struct TypeList {
    static constexpr const int NOT_ANY_TYPE = -1;
    static const std::size_t COUNT = sizeof...(Types);

    template <typename Type>
    struct HasType {
        static constexpr const bool VALUE = false;
    };

    template <typename Type>
    struct GetTypeId {
        static constexpr const int VALUE = NOT_ANY_TYPE;
    };

    template <template <typename ...> class Other>
    struct DefineWithListTypes {
        using Type = Other<Types...>;
    };

    using HeadType = TypeTag<void>;
    using InheritedType = TypeTag<void>;
};

template <typename Head, typename ... Types>
struct TypeList<Head, Types...> : TypeList<Types...> {
    static constexpr const int NOT_ANY_TYPE = TypeList<Types...>::NOT_ANY_TYPE;
    template <typename Type>
    struct HasType {
        static constexpr const bool VALUE =
            (   std::is_same<Head, Type>::value
             || TypeList<Types...>::template HasType<Type>::VALUE);
    };

    template <typename Type>
    struct GetTypeId {
        static constexpr const int VALUE =
            ConstIntSelect<std::is_same<Head, Type>::value,
            sizeof...(Types),
            TypeList<Types...>::template GetTypeId<Type>::VALUE>::
            VALUE;
    };

    template <template <typename ...> class Other>
    struct DefineWithListTypes {
        using Type = Other<Head, Types...>;
    };
    // trying a new linear approach, to save complier time
    static_assert(!TypeList<Types...>::template HasType<Head>::VALUE,
                  "All types in type list, must be unique.");

    static const std::size_t COUNT = sizeof...(Types) + 1;
    using HeadType = Head;
    using InheritedType = TypeList<Types...>;
};

template <std::size_t SIZE, typename ... ReadTypes, typename ... HostTypes>
void find_bitset_intersection
    (std::bitset<SIZE> &, TypeList<ReadTypes...>, TypeList<HostTypes...>)
{}

template <std::size_t SIZE, typename Head, typename ... ReadTypes,
          typename ... HostTypes>
void find_bitset_intersection
    (std::bitset<SIZE> & bs, TypeList<Head, ReadTypes...>, TypeList<HostTypes...>)
{
    // this is now evaluated at compile time :D
    static_assert(TypeList<HostTypes...>::template HasType<Head>::VALUE,
                  "Head type is not in the component type list.");
    bs.set(TypeList<HostTypes...>::template GetTypeId<Head>::VALUE, true);
    find_bitset_intersection
        (bs, TypeList<ReadTypes...>(), TypeList<HostTypes...>());
}

template <bool TF_VAL, typename A, typename B>
struct TypeSelect { using Type = A; };

template <typename A, typename B>
struct TypeSelect<false, A, B> { using Type = B; };
