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

// ----------------------------- ORIGINAL LICENSE -----------------------------
//
// © 2017-2020 Erik Rigtorp <erik@rigtorp.se>
// SPDX-License-Identifier: MIT

/*
HashMap

A high performance hash map. Uses open addressing with linear
probing.

Advantages:
  - Predictable performance. Doesn't use the allocator unless load factor
    grows beyond 50%. Linear probing ensures cash efficency.
  - Deletes items by rearranging items and marking slots as empty instead of
    marking items as deleted. This is keeps performance high when there
    is a high rate of churn (many paired inserts and deletes) since otherwise
    most slots would be marked deleted and probing would end up scanning
    most of the table.

Disadvantages:
  - Significant performance degradation at high load factors.
  - Maximum load factor hard coded to 50%, memory inefficient.
  - Memory is not reclaimed on erase.
 */
// --------------------------- ORIGINAL LICENSE END ---------------------------

#pragma once

#include <ariajanke/cul/Util.hpp>

#include <algorithm>
#include <limits>
#include <stdexcept>
#include <vector>

#include <cassert>
#include <cstddef>
#include <cstdint>

namespace cul {

template <
    typename KeyT,
    typename ElementT,
    typename Hash = std::hash<KeyT>,
    typename KeyEqualT = std::equal_to<void>,
    typename AllocatorT = std::allocator<std::pair<KeyT, ElementT>>
>
class HashMap {
public:
    using KeyType = KeyT;
    using ElementType = ElementT;
    using KeyValuePairType = std::pair<KeyType, ElementType>;
    using Hasher = Hash;
    using KeyEquality = KeyEqualT;
    using Allocator = AllocatorT;
    using BucketContainer = std::vector<KeyValuePairType, Allocator>;
    using BucketIterator = typename BucketContainer::iterator;
    using BucketConstIterator = typename BucketContainer::const_iterator;

    template <bool kt_is_constant>
    class IteratorImpl final {
    public:
        using BucketIteratorImpl =
            std::conditional_t<kt_is_constant, BucketConstIterator, BucketIterator>;

        using Reference = std::conditional_t
            <kt_is_constant, const ElementType &, ElementType &>;

        using Pointer = std::conditional_t
            <kt_is_constant, const ElementType *, ElementType *>;

        static BucketIteratorImpl advance_past_empty
            (const BucketIteratorImpl & position,
             const BucketIteratorImpl & end,
             const KeyType & empty);

        IteratorImpl
            (const BucketIteratorImpl & position,
             const BucketIteratorImpl & end,
             const KeyType & empty);

        bool operator == (const IteratorImpl & rhs) const
            { return equal_to(rhs); }

        bool operator != (const IteratorImpl & rhs) const
            { return !equal_to(rhs); }

        IteratorImpl & operator++ ();

        IteratorImpl advance_past_empty() &&;

        Reference operator * () const { return element(); }

        Pointer operator -> () const { return &element(); }

    private:
        friend class HashMap;

        bool equal_to(const IteratorImpl &) const;

        Reference element() const { return m_iterator->second; }

        auto key() const { return m_iterator->first; }

        // not fixing now:
        // index was very purposely chosen...
        BucketIteratorImpl m_iterator;
        BucketIteratorImpl m_end_iterator;
        const KeyType & m_empty_key;
    };

    using Iterator      = IteratorImpl<false>;
    using ConstIterator = IteratorImpl<true>;

    HashMap(KeyType empty_key,
            BucketContainer && = BucketContainer{},
            const Allocator & = Allocator{});

    HashMap(const HashMap &);

    HashMap(HashMap &&);

    HashMap & operator = (const HashMap &);

    HashMap & operator = (HashMap &&);

    ConstIterator begin() const noexcept { return cbegin(); }

    Iterator begin() noexcept
        { return make_iterator(m_bucket_container.begin()); }

    ConstIterator cbegin() const noexcept
        { return make_iterator(m_bucket_container.cbegin()); }

    ConstIterator cend() const noexcept
        { return make_iterator(m_bucket_container.cend()); }

    void clear() noexcept;

    template <typename... Types>
    std::pair<Iterator, bool> emplace(Types &&... args)
        { return emplace_impl(std::forward<Types>(args)...); }

    ConstIterator end() const noexcept { return cend(); }

    Iterator end() noexcept
        { return make_iterator(m_bucket_container.end()); }

    template <typename OtherKeyType, typename ... ArgTypes>
    Iterator ensure(const OtherKeyType & key, ArgTypes &&... args)
        { return std::get<0>(emplace_impl(OtherKeyType{key}, std::forward<ArgTypes>(args)...)); }

    template <typename OtherKeyType, typename ... ArgTypes>
    Iterator ensure(OtherKeyType && key, ArgTypes &&... args)
        { return std::get<0>(emplace_impl(std::move(key), std::forward<ArgTypes>(args)...)); }

    Iterator erase(const Iterator &);

    template <typename KeyType>
    Iterator find(const KeyType &);

    template <typename KeyType>
    ConstIterator find(const KeyType &) const;

    bool is_empty() const noexcept { return m_size == 0; }

    std::pair<Iterator, bool> insert(const KeyValuePairType & pair)
        { return emplace_impl(KeyType{pair.first}, pair.second); }

    std::pair<Iterator, bool> insert(KeyValuePairType && pair)
        { return emplace_impl(std::move(pair.first), std::move(pair.second)); }

    BucketContainer move_out_buckets();

    void rehash(std::size_t for_at_least_this_many_elements = 0);

    void reserve(std::size_t for_at_least_this_many_elements);

    std::size_t size() const noexcept { return m_size; }

    void swap(HashMap &);

private:
    Iterator make_iterator(BucketIterator itr);

    ConstIterator make_iterator(BucketConstIterator itr) const;

    template <typename OtherKeyType, typename ... ArgTypes>
    std::pair<Iterator, bool> emplace_impl(OtherKeyType &&, ArgTypes &&...);

    template <typename OtherKeyType>
    std::size_t to_index(const OtherKeyType &) const;

    BucketIterator probe_next(const BucketIterator &) const noexcept;

    template <typename OtherKeyType>
    std::size_t key_to_index(const OtherKeyType & key) const noexcept
        { return Hasher{}(key) & size_mask(); }

    std::size_t size_mask() const noexcept
        // why we need base 2 numbers
        { return m_bucket_container.size() - 1; }

    KeyType m_empty_key;
    BucketContainer m_bucket_container;
    std::size_t m_size = 0;
};

// ----------------------------------------------------------------------------

#define MACRO_HASHMAP_TEMPLATES \
    template < \
        typename KeyT, \
        typename ElementT, \
        typename Hash, \
        typename KeyEqualT, \
        typename AllocatorT \
    >

#define MACRO_HASHMAP_CLASSNAME \
    HashMap<KeyT,ElementT, Hash, KeyEqualT, AllocatorT>

MACRO_HASHMAP_TEMPLATES
template <bool kt_is_constant>
MACRO_HASHMAP_CLASSNAME::IteratorImpl<kt_is_constant>::IteratorImpl
    (const BucketIteratorImpl & iter_impl_,
     const BucketIteratorImpl & end_,
     const KeyType & empty_key_):
    m_iterator(iter_impl_),
    m_end_iterator(end_),
    m_empty_key(empty_key_) {}

MACRO_HASHMAP_TEMPLATES
template <bool kt_is_constant>
typename MACRO_HASHMAP_CLASSNAME::template IteratorImpl<kt_is_constant> &
    MACRO_HASHMAP_CLASSNAME::IteratorImpl<kt_is_constant>::operator++ ()
{
    for (/* blank */;
         KeyEquality{}(key(), m_empty_key) &&
            m_iterator != m_end_iterator;
         ++m_iterator) {}
    return *this;
}

MACRO_HASHMAP_TEMPLATES
template <bool kt_is_constant>
/* private */ bool MACRO_HASHMAP_CLASSNAME::IteratorImpl<kt_is_constant>::
    equal_to(const IteratorImpl & rhs) const
{ return KeyEquality{}(key(), rhs.key()); }

// ----------------------------------------------------------------------------

MACRO_HASHMAP_TEMPLATES
MACRO_HASHMAP_CLASSNAME::HashMap
    (KeyType empty_key_,
     BucketContainer && buckets_,
     const Allocator & allocator_):
    m_empty_key(empty_key_),
    m_bucket_container(std::move(buckets_), allocator_)
{ rehash(); }

MACRO_HASHMAP_TEMPLATES
MACRO_HASHMAP_CLASSNAME::HashMap(const HashMap & rhs):
    m_empty_key(rhs.m_empty_key),
    m_bucket_container(rhs.m_bucket_container),
    m_size(rhs.m_size) {}

MACRO_HASHMAP_TEMPLATES
MACRO_HASHMAP_CLASSNAME::HashMap(HashMap && rhs):
    m_empty_key(std::move(rhs.m_empty_key)),
    m_bucket_container(std::move(rhs.m_bucket_container)),
    m_size(std::move(rhs.m_size)) {}

MACRO_HASHMAP_TEMPLATES
MACRO_HASHMAP_CLASSNAME & MACRO_HASHMAP_CLASSNAME::operator =
    (const HashMap & rhs)
{
    if (rhs != &rhs) {
        HashMap temp{rhs};
        swap(rhs);
    }
    return *this;
}

MACRO_HASHMAP_TEMPLATES
MACRO_HASHMAP_CLASSNAME & MACRO_HASHMAP_CLASSNAME::operator =
    (HashMap && rhs)
{
    if (rhs != &rhs)
        { swap(rhs); }
    return *this;
}

MACRO_HASHMAP_TEMPLATES
void MACRO_HASHMAP_CLASSNAME::clear() noexcept {
    for (auto & bucket : m_bucket_container) {
        if (KeyEquality{}(bucket.first, m_empty_key))
            { continue; }
        bucket.first = m_empty_key;
        bucket.second = ElementType{};
    }
    m_size = 0;
}

MACRO_HASHMAP_TEMPLATES
typename MACRO_HASHMAP_CLASSNAME::Iterator
    MACRO_HASHMAP_CLASSNAME::erase
    (const Iterator & iterator)
{
    auto bucket = iterator.m_iterator;
    decltype(bucket) itr;
    while (true) {
        itr = probe_next(iterator.m_iterator);
        if (KeyEquality{}(itr->first, m_empty_key) || itr < bucket) {
            *bucket = std::make_pair(m_empty_key, ElementType{});
            --m_size;
            return Iterator{iterator}.advance_past_empty();
        }

        auto ideal = m_bucket_container.begin() + key_to_index(itr->first);
        // is bucket_iterator closer to ideal than itr?
        if (magnitude(bucket - ideal) < magnitude(itr - ideal)) {
            *bucket = *ideal; // how is this a swap?
            bucket = itr;
        }
    }
}

MACRO_HASHMAP_TEMPLATES
template <typename KeyType>
typename MACRO_HASHMAP_CLASSNAME::Iterator
    MACRO_HASHMAP_CLASSNAME::find(const KeyType & key)
{
    if (KeyEquality{}(key, m_empty_key))
        { return m_bucket_container.end(); }
    for (size_t idx = key_to_idx(key);; idx = probe_next(idx)) {
      if (key_equal()(buckets_[idx].first, key)) {
        return iterator(this, idx);
      }
      if (key_equal()(buckets_[idx].first, empty_key_)) {
        return end();
      }
    }
}

MACRO_HASHMAP_TEMPLATES
template <typename KeyType>
typename MACRO_HASHMAP_CLASSNAME::ConstIterator
    MACRO_HASHMAP_CLASSNAME::find(const KeyType & key) const
{
    // still dumb
    Iterator itr = const_cast<HashMap *>(this)->find(key);
    return ConstIterator
        {std::move(itr.m_iterator),
         std::move(itr.m_end_iterator),
         itr.m_empty_key};
}

MACRO_HASHMAP_TEMPLATES
typename MACRO_HASHMAP_CLASSNAME::BucketContainer
    MACRO_HASHMAP_CLASSNAME::move_out_buckets()
{
    auto t = std::move(m_bucket_container);
    m_bucket_container.clear();
    return t;
}

MACRO_HASHMAP_TEMPLATES
void MACRO_HASHMAP_CLASSNAME::rehash
    (std::size_t for_at_least_this_many_elements)
{}

MACRO_HASHMAP_TEMPLATES
void MACRO_HASHMAP_CLASSNAME::reserve
    (std::size_t for_at_least_this_many_elements)
{}

MACRO_HASHMAP_TEMPLATES
void swap(HashMap &);

/* private */ Iterator make_iterator(BucketIterator itr);

/* private */ ConstIterator make_iterator(BucketConstIterator itr) const;

template <typename OtherKeyType, typename ... ArgTypes>
/* private */ std::pair<Iterator, bool> emplace_impl(OtherKeyType &&, ArgTypes &&...);

#undef MACRO_HASHMAP_TEMPLATES
#undef MACRO_HASHMAP_CLASSNAME

} // end of cul namespace
