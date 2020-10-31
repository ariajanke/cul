/****************************************************************************

    File: FixedLengthArray.hpp
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

#include <array>
#include <iterator>
#include <iosfwd>

#include <common/TypeList.hpp>

/** An alternative to std::array, meant to be safe for use with non-trivially
 *  constructible objects.
 *  This provides many methods which both have the same name and function as
 *  their STL counter parts.
 */
template <typename T, std::size_t SIZE>
class FixedLengthArray {
public:
    using Storage = typename std::aligned_storage<sizeof(T), alignof(T)>::type;
    using StorageArray = std::array<Storage, SIZE>;
    using ValueType = T;

    FixedLengthArray();
    FixedLengthArray(const FixedLengthArray & rhs);
    // can't move!
    FixedLengthArray(FixedLengthArray && rhs) = delete;
    ~FixedLengthArray();

    FixedLengthArray & operator = (const FixedLengthArray & rhs);
    // can't move!
    FixedLengthArray & operator = (FixedLengthArray && rhs) = delete;

    std::size_t size() const { return SIZE; }

    T & operator [] (std::size_t index);

    const T & operator [] (std::size_t index) const;

    template <bool IS_CONST>
    class IterImpl {
    public:
        friend class FixedLengthArray<T, SIZE>;
        using PointerType = typename TypeSelect<IS_CONST, const T *, T *>::Type;
        using ReferenceType = typename TypeSelect<IS_CONST, const T &, T &>::Type;
        using StoragePointerType =
            typename TypeSelect<IS_CONST, const Storage *, Storage *>::Type;
        using DiffType = std::intptr_t;

        IterImpl(): m_itr(nullptr) {}

        PointerType operator -> () const
            { return reinterpret_cast<PointerType>(m_itr); }

        ReferenceType operator * () const
            { return *reinterpret_cast<PointerType>(m_itr); }

        IterImpl & operator += (DiffType n) { m_itr += n; return *this; }

        IterImpl & operator -= (DiffType n) { m_itr -= n; return *this; }

        bool operator == (const IterImpl & rhs) const
            { return m_itr == rhs.m_itr; }

        bool operator != (const IterImpl & rhs) const
            { return m_itr != rhs.m_itr; }

        IterImpl & operator -- () { --m_itr; return *this; }
        IterImpl & operator ++ () { ++m_itr; return *this; }

        IterImpl operator -- (int) { IterImpl rv(*this); --m_itr; return rv; }
        IterImpl operator ++ (int) { IterImpl rv(*this); ++m_itr; return rv; }

    private:
        explicit IterImpl(StoragePointerType ptr): m_itr(ptr) {}

        StoragePointerType m_itr;
    };

    using Iter      = IterImpl<false>;
    using ConstIter = IterImpl<true >;

    Iter      begin()       { return Iter     (&m_stores[0]); }
    ConstIter begin() const { return ConstIter(&m_stores[0]); }

    Iter      end()      ;
    ConstIter end() const;

    T & front() { return *reinterpret_cast<T *>(&m_stores[0       ]);  }
    T & back () { return *reinterpret_cast<T *>(&m_stores[SIZE - 1]);  }

    const T & front() const
        { return *reinterpret_cast<const T *>(&m_stores[0]); }

    const T & back() const
        { return *reinterpret_cast<const T *>(&m_stores[SIZE - 1]); }

private:

    void copy_objects_from_stores(const StorageArray & rhs);
    void destruct_objects_in_stores();

    // stores vs objects in terms of padding?
    // align and size should be OK...
    StorageArray m_stores;
};

// ----------------------------------------------------------------------------

template <typename T, std::size_t SIZE>
FixedLengthArray<T, SIZE>::FixedLengthArray() {
    // std::array doesn't call entries' constructors!
    for (Storage & store : m_stores) {
        new (&store) T();
    }
}

template <typename T, std::size_t SIZE>
FixedLengthArray<T, SIZE>::FixedLengthArray(const FixedLengthArray & rhs) {
    copy_objects_from_stores(rhs.m_stores);
}

template <typename T, std::size_t SIZE>
FixedLengthArray<T, SIZE>::~FixedLengthArray() {
    destruct_objects_in_stores();
}

template <typename T, std::size_t SIZE>
FixedLengthArray<T, SIZE> &
    FixedLengthArray<T, SIZE>::operator = (const FixedLengthArray & rhs)
{
    if (this != &rhs) {
        destruct_objects_in_stores();
        copy_objects_from_stores(rhs.m_stores);
    }
    return *this;
}

template <typename T, std::size_t SIZE>
T & FixedLengthArray<T, SIZE>::operator [] (std::size_t index) {
    return *reinterpret_cast<T *>(&m_stores[index]);
}

template <typename T, std::size_t SIZE>
const T & FixedLengthArray<T, SIZE>::operator [] (std::size_t index) const {
    return *reinterpret_cast<const T *>(&m_stores[index]);
}

template <typename T, std::size_t SIZE>
void FixedLengthArray<T, SIZE>::copy_objects_from_stores
    (const StorageArray & rhs)
{
    // :WARNING: Assumes that the objects in the stores have already been
    //           constructed
    for (std::size_t i = 0; i != SIZE; ++i) {
        const T * src_item = reinterpret_cast<const T *>(&rhs[i]);
        Storage & target_store = m_stores[i];
        new (&target_store) T(*src_item);
    }
}

template <typename T, std::size_t SIZE>
void FixedLengthArray<T, SIZE>::destruct_objects_in_stores() {
    // :WARNING: Assumes that the objects in the stores have already been
    //           constructed
    for (Storage & store : m_stores) {
        T * item = reinterpret_cast<T *>(&store);
        item->~T();
    }
}

template <typename T, std::size_t SIZE>
typename FixedLengthArray<T, SIZE>::Iter
    FixedLengthArray<T, SIZE>::end()
    { return Iter(&m_stores[0] + SIZE); }

template <typename T, std::size_t SIZE>
typename FixedLengthArray<T, SIZE>::ConstIter
    FixedLengthArray<T, SIZE>::end() const
    { return ConstIter(&m_stores[0] + SIZE); }
