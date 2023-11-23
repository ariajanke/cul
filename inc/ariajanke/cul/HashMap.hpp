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
#include <optional>

#include <cassert>
#include <cstddef>
#include <cstdint>

namespace cul {

template <
    typename KeyT,
    typename ElementT,
    typename AllocatorT,
    typename KeyEqualityT
>
class HashMapBucketDefinitions {
public:
    using ElementSpace = std::aligned_storage_t<sizeof(ElementT)>;
    using KeyValuePairType = std::pair<KeyT, ElementT>;
    using Bucket = std::pair<KeyT, ElementSpace>;
    using BucketContainer = std::vector<Bucket, AllocatorT>;
};

template <
    typename KeyT,
    typename ElementT,
    typename AllocatorT,
    typename KeyEqualityT,
    bool kt_is_constant
>
class HashMapIteratorImpl;

template <
    typename KeyT,
    typename ElementT,
    typename HashT = std::hash<KeyT>,
    typename KeyEqualT = std::equal_to<void>,
    typename AllocatorT = std::allocator<std::pair<KeyT, std::aligned_storage_t<sizeof(ElementT)>>>
>
class HashMap final {
    using Defs = HashMapBucketDefinitions<KeyT, ElementT, AllocatorT, KeyEqualT>;
    template <bool kt_is_constant>
    using IteratorImpl = HashMapIteratorImpl
        <KeyT, ElementT, AllocatorT, KeyEqualT, kt_is_constant>;
public:
    using KeyType = KeyT;
    using ElementType = ElementT;
    using Hasher = HashT;
    using KeyEquality = KeyEqualT;
    using Allocator = AllocatorT;
    using Iterator      = IteratorImpl<false>;
    using ConstIterator = IteratorImpl<true>;

    struct Extraction final {
        Extraction(Iterator && nx_, ElementType && el_, KeyType && key_):
            next(std::move(nx_)), element(std::move(el_)), key(std::move(key_)) {}

        Iterator next;
        ElementType element;
        KeyType key;
    };

    struct Insertion final {
        Insertion(bool success_, Iterator && position_):
            success(success_), position(std::move(position_)) {}

        bool success;
        Iterator position;
    };

    explicit HashMap(KeyType empty_key,
                     const Allocator & = Allocator{});

    HashMap(const HashMap &);

    HashMap(HashMap &&);

    HashMap & operator = (const HashMap &);

    HashMap & operator = (HashMap &&);

    ConstIterator begin() const noexcept { return cbegin(); }

    Iterator begin() noexcept
        { return advance_past_empty(make_iterator(0)); }

    std::size_t bucket_count() const noexcept
        { return m_bucket_container.size(); }

    std::size_t capacity() const noexcept
        { return m_bucket_container.size() / k_load_factor; }

    ConstIterator cbegin() const noexcept
        { return make_iterator(0).advance_past_empty(); }

    ConstIterator cend() const noexcept
        { return make_iterator(m_bucket_container.size()); }

    void clear() noexcept;

    template <typename... Types>
    Insertion emplace(Types &&... args)
        { return emplace_impl(std::forward<Types>(args)...); }

    ConstIterator end() const noexcept { return cend(); }

    Iterator end() noexcept
        { return make_iterator(m_bucket_container.size()); }

    template <typename OtherKeyType, typename ... ArgTypes>
    Iterator ensure(const OtherKeyType & key, ArgTypes &&... args)
        { return std::get<0>(emplace_impl(OtherKeyType{key}, std::forward<ArgTypes>(args)...)); }

    template <typename OtherKeyType, typename ... ArgTypes>
    Iterator ensure(OtherKeyType && key, ArgTypes &&... args)
        { return std::get<0>(emplace_impl(std::move(key), std::forward<ArgTypes>(args)...)); }

    Iterator erase(const Iterator &);

    Extraction extract(const Iterator &);

    template <typename OtherKeyType>
    Iterator find(const OtherKeyType &);

    template <typename OtherKeyType>
    ConstIterator find(const OtherKeyType &) const;

    template <typename OtherKeyType>
    Insertion insert(const OtherKeyType & key, const ElementType & el)
        { return emplace_impl(key, el); }

    bool is_empty() const noexcept { return m_size == 0; }

    void rehash(std::size_t for_at_least_this_many_elements = 0);

    void reserve(std::size_t for_at_least_this_many_elements);

    std::size_t size() const noexcept { return m_size; }

    void swap(HashMap &);

private:
    using ElementSpace = typename Defs::ElementSpace;
    using Bucket = typename Defs::Bucket;
    using BucketContainer = typename Defs::BucketContainer;
    using BucketIterator = typename BucketContainer::iterator;

    static constexpr const std::size_t k_load_factor = 2;

    static Iterator advance_past_empty(Iterator && itr)
        { return Iterator::detail_advance_past_empty(std::move(itr)); }

    static void swap_buckets(BucketIterator & lhs, BucketIterator & rhs) noexcept;

    Iterator make_iterator(std::size_t);

    ConstIterator make_iterator(std::size_t) const;

    template <typename OtherKeyType, typename ... ArgTypes>
    Insertion emplace_impl(OtherKeyType &&, ArgTypes &&...);

    std::size_t probe_next(std::size_t) const noexcept;

    template <typename OtherKeyType>
    std::size_t key_to_index(const OtherKeyType & key) const noexcept
        { return Hasher{}(key) & size_mask(); }

    std::size_t size_mask() const noexcept
        // why we need base 2 numbers
        { return m_bucket_container.size() - 1; }

    Extraction extract_bucket_and_advance
        (BucketIterator && bucket, const Iterator & iterator) noexcept;

    template <typename OtherKeyType>
    std::size_t find_impl(const OtherKeyType &) const noexcept;

    KeyType m_empty_key;
    BucketContainer m_bucket_container;
    std::size_t m_size = 0;
};

// ----------------------------------------------------------------------------

template <typename KeyT, typename ElementT, bool kt_is_constant>
class HashMapPairWrapper;

template <
    typename KeyT,
    typename ElementT,
    typename AllocatorT,
    typename KeyEqualityT,
    bool kt_is_constant
>
class HashMapIteratorImpl final {
    using Defs = HashMapBucketDefinitions<KeyT, ElementT, AllocatorT, KeyEqualityT>;
public:
    using ElementType = ElementT;
    using KeyType = KeyT;
    using PairWrapperImpl = HashMapPairWrapper<KeyType, ElementType, kt_is_constant>;
    using KeyValuePairType = std::pair<KeyType, ElementType>;
    using Allocator = AllocatorT;
    using KeyEquality = KeyEqualityT;
    using BucketContainer = typename Defs::BucketContainer;
    using ElementReference = typename PairWrapperImpl::ElementReference;
    using BucketContainerPtr = std::conditional_t
        <kt_is_constant, const BucketContainer *, BucketContainer *>;
    using Reference = typename PairWrapperImpl::PairType;
    using Pointer = PairWrapperImpl;

    // defs for std algorithms
    using difference_type = std::ptrdiff_t;
    using value_type = PairWrapperImpl;
    using pointer = PairWrapperImpl;
    using reference = Reference;
    using iterator_category = std::bidirectional_iterator_tag;

    static std::size_t detail_bucket_index_of(const HashMapIteratorImpl & itr)
        { return itr.m_index; }

    static HashMapIteratorImpl detail_advance_past_empty
        (HashMapIteratorImpl && itr)
    {
        itr.advance_past_empty_();
        return std::move(itr);
    }

    HashMapIteratorImpl
        (std::size_t index,
         BucketContainerPtr container,
         const KeyType & empty);

    HashMapIteratorImpl(const HashMapIteratorImpl &);

    HashMapIteratorImpl(HashMapIteratorImpl &&);

    HashMapIteratorImpl & operator = (const HashMapIteratorImpl &);

    HashMapIteratorImpl & operator = (HashMapIteratorImpl &&);

    bool operator == (const HashMapIteratorImpl & rhs) const
        { return equal_to(rhs); }

    bool operator != (const HashMapIteratorImpl & rhs) const
        { return !equal_to(rhs); }

    HashMapIteratorImpl & operator++ ();

    Reference operator * () const { return *element(); }

    PairWrapperImpl operator -> () const { return element(); }

private:
    bool equal_to(const HashMapIteratorImpl &) const;

    PairWrapperImpl element() const {
        auto & bucket = (*m_container)[m_index];
        return PairWrapperImpl{bucket.first, &bucket.second};
    }

    auto key() const { return (*m_container)[m_index].first; }

    void advance_past_empty_();

    std::size_t m_index = 0;
    BucketContainerPtr m_container = nullptr;
    const KeyType * m_empty_key = nullptr;
};

// ----------------------------------------------------------------------------

template <typename KeyT, typename ElementT, bool kt_is_constant>
class HashMapPairWrapper final {
public:
    using DataPtr = std::conditional_t
        <kt_is_constant, const void *, void *>;

    using ElementPtr = std::conditional_t
        <kt_is_constant, const ElementT *, ElementT *>;

    using ElementReference = std::conditional_t
        <kt_is_constant, const ElementT &, ElementT &>;

    using PairType = std::pair<const KeyT &, ElementReference>;

    HashMapPairWrapper(const KeyT & key, DataPtr el_ref):
        m_proxy(key, *reinterpret_cast<ElementPtr>(el_ref)) {}

    const PairType * operator -> () const { return &m_proxy; }

    PairType & operator * () { return m_proxy; }

private:
    PairType m_proxy;
};

// ----------------------------------------------------------------------------

namespace detail {

inline std::size_t nearest_base2_number(std::size_t n) {
    std::size_t pow2 = 1;
    while (pow2 < n)
        { pow2 *= 2; }
    return pow2;
}

} // end of cul::detail namespace -> into cul namespace

#define MACRO_HASHMAP_TEMPLATES \
    template < \
        typename KeyT, \
        typename ElementT, \
        typename HashT, \
        typename KeyEqualT, \
        typename AllocatorT \
    >

#define MACRO_HASHMAP_CLASSNAME \
    HashMap<KeyT, ElementT, HashT, KeyEqualT, AllocatorT>

#define MACRO_ITERATOR_TEMPLATES \
    template < \
        typename KeyT, \
        typename ElementT, \
        typename AllocatorT, \
        typename KeyEqualT, \
        bool kt_is_constant \
    >

#define MACRO_ITERATOR_CLASSNAME \
    HashMapIteratorImpl<KeyT, ElementT, AllocatorT, KeyEqualT, kt_is_constant>

MACRO_ITERATOR_TEMPLATES
MACRO_ITERATOR_CLASSNAME::HashMapIteratorImpl
    (std::size_t index_,
     BucketContainerPtr container_,
     const KeyType & empty_key_):
    m_index(index_),
    m_container(container_),
    m_empty_key(&empty_key_) {}

MACRO_ITERATOR_TEMPLATES
MACRO_ITERATOR_CLASSNAME::HashMapIteratorImpl
    (const HashMapIteratorImpl & rhs):
    m_index(rhs.m_index),
    m_container(rhs.m_container),
    m_empty_key(rhs.m_empty_key) {}

MACRO_ITERATOR_TEMPLATES
MACRO_ITERATOR_CLASSNAME::HashMapIteratorImpl
    (HashMapIteratorImpl && rhs):
    m_index(std::move(rhs.m_index)),
    m_container(std::move(rhs.m_container)),
    m_empty_key(std::move(rhs.m_empty_key)) {}

MACRO_ITERATOR_TEMPLATES
MACRO_ITERATOR_CLASSNAME &
    MACRO_ITERATOR_CLASSNAME::operator = (const HashMapIteratorImpl & rhs)
{
    if (this != &rhs) {
        m_index = rhs.m_index;
        m_container = rhs.m_container;
        m_empty_key = rhs.m_empty_key;
    }
    return *this;
}

MACRO_ITERATOR_TEMPLATES
MACRO_ITERATOR_CLASSNAME &
    MACRO_ITERATOR_CLASSNAME::operator = (HashMapIteratorImpl && rhs)
{
    if (this != &rhs) {
        std::swap(m_index    , rhs.m_index    );
        std::swap(m_container, rhs.m_container);
        std::swap(m_empty_key, rhs.m_empty_key);
    }
    return *this;
}

MACRO_ITERATOR_TEMPLATES
MACRO_ITERATOR_CLASSNAME &
    MACRO_ITERATOR_CLASSNAME::operator++ ()
{
    ++m_index;
    advance_past_empty_();
    return *this;
}

MACRO_ITERATOR_TEMPLATES
/* private */ void MACRO_ITERATOR_CLASSNAME::advance_past_empty_() {
    while (m_index < m_container->size() && KeyEquality{}(key(), *m_empty_key))
        { ++m_index; }
}

MACRO_ITERATOR_TEMPLATES
/* private */ bool MACRO_ITERATOR_CLASSNAME::equal_to
    (const HashMapIteratorImpl & rhs) const
{ return m_index == rhs.m_index && m_container == rhs.m_container; }

// ----------------------------------------------------------------------------

MACRO_HASHMAP_TEMPLATES
MACRO_HASHMAP_CLASSNAME::HashMap
    (KeyType empty_key_,
     const Allocator & allocator_):
    m_empty_key(empty_key_),
    m_bucket_container(allocator_) {}

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
{ return extract(iterator).next; }

MACRO_HASHMAP_TEMPLATES
typename MACRO_HASHMAP_CLASSNAME::Extraction
    MACRO_HASHMAP_CLASSNAME::extract(const Iterator & iterator)
{
    if (iterator == end()) {
        throw std::invalid_argument
            {"Cannot extract/erase at the end position of the container"};
    }
    auto index = Iterator::detail_bucket_index_of(iterator);
    const auto beg_ = m_bucket_container.begin();
    auto bucket = beg_ + index;

    for (index = probe_next(index); true; index = probe_next(index)) {
        assert(index < m_bucket_container.size());
        auto index_itr = beg_ + index;
        if (KeyEquality{}(index_itr->first, m_empty_key))
            { return extract_bucket_and_advance(std::move(bucket), iterator); }

        auto ideal = m_bucket_container.begin() + key_to_index(index_itr->first);
        if (magnitude(bucket - ideal) < magnitude(index_itr - ideal)) {
            swap_buckets(bucket, index_itr);
            bucket = index_itr;
        }
    }
}

MACRO_HASHMAP_TEMPLATES
template <typename OtherKeyType>
typename MACRO_HASHMAP_CLASSNAME::Iterator
    MACRO_HASHMAP_CLASSNAME::find(const OtherKeyType & key)
{ return make_iterator(find_impl(key)); }

MACRO_HASHMAP_TEMPLATES
template <typename OtherKeyType>
typename MACRO_HASHMAP_CLASSNAME::ConstIterator
    MACRO_HASHMAP_CLASSNAME::find(const OtherKeyType & key) const
{ return make_iterator(find_impl(key)); }

MACRO_HASHMAP_TEMPLATES
void MACRO_HASHMAP_CLASSNAME::rehash
    (std::size_t for_at_least_this_many_elements)
{
    if (is_empty()) {
        reserve(2);
        return;
    }

    HashMap temp{m_empty_key};
    temp.reserve(std::min(m_bucket_container.size()*k_load_factor,
                          for_at_least_this_many_elements));
    auto itr = begin();
    while (itr != end()) {
        assert(!KeyEquality{}(itr->first, m_empty_key));
        auto extraction = extract(itr);
        temp.emplace(std::move(extraction.key),
                     std::move(extraction.element));
        itr = extraction.next;
    }
    swap(temp);
}

MACRO_HASHMAP_TEMPLATES
void MACRO_HASHMAP_CLASSNAME::reserve
    (std::size_t for_at_least_this_many_elements)
{
    Bucket empty_bucket = std::make_pair(m_empty_key, ElementSpace{});
    m_bucket_container.resize
        (detail::nearest_base2_number
            (for_at_least_this_many_elements*k_load_factor),
         empty_bucket);
}

MACRO_HASHMAP_TEMPLATES
void MACRO_HASHMAP_CLASSNAME::swap(HashMap & rhs) {
    m_bucket_container.swap(rhs.m_bucket_container);
    std::swap(m_empty_key, rhs.m_empty_key);
    std::swap(m_size     , rhs.m_size     );
}

MACRO_HASHMAP_TEMPLATES
/* private static */ void MACRO_HASHMAP_CLASSNAME::swap_buckets
    (BucketIterator & lhs, BucketIterator & rhs) noexcept
{
    auto & lhs_el = *reinterpret_cast<ElementType *>(&lhs->second);
    auto & rhs_el = *reinterpret_cast<ElementType *>(&rhs->second);
    std::swap(lhs->first, rhs->first);
    std::swap(lhs_el, rhs_el);
}

MACRO_HASHMAP_TEMPLATES
/* private */ typename MACRO_HASHMAP_CLASSNAME::Iterator
    MACRO_HASHMAP_CLASSNAME::make_iterator(std::size_t index)
{ return Iterator{index, &m_bucket_container, m_empty_key}; }

MACRO_HASHMAP_TEMPLATES
/* private */ typename MACRO_HASHMAP_CLASSNAME::ConstIterator
    MACRO_HASHMAP_CLASSNAME::make_iterator(std::size_t index) const
{ return ConstIterator{index, &m_bucket_container, m_empty_key}; }

MACRO_HASHMAP_TEMPLATES
template <typename OtherKeyType, typename ... ArgTypes>
/* private */
    typename MACRO_HASHMAP_CLASSNAME::Insertion
    MACRO_HASHMAP_CLASSNAME::emplace_impl
    (OtherKeyType && key, ArgTypes &&... element_args)
{
    if (KeyEquality{}(m_empty_key, key)) {
        throw std::invalid_argument
            {"Cannot use empty key for inserting elements in hash map"};
    }

    if (size() + 1 > capacity())
        { rehash(size()*2); }

    auto index = key_to_index(key);
    while (true) {
        auto & bucket = m_bucket_container[index];
        if (KeyEquality{}(bucket.first, m_empty_key)) {
            bucket.first = std::move(key);
            new (&bucket.second) ElementType{std::forward<ArgTypes>(element_args)...};
            ++m_size;
            return Insertion{true, make_iterator(index)};
        } else if (KeyEquality{}(bucket.first, key)) {
            return Insertion{false, make_iterator(index)};
        }
        index = probe_next(index);
    }
}

MACRO_HASHMAP_TEMPLATES
/* private */ std::size_t MACRO_HASHMAP_CLASSNAME::probe_next
    (std::size_t idx) const noexcept
    { return (idx + 1) & size_mask(); }

MACRO_HASHMAP_TEMPLATES
/* private */ typename MACRO_HASHMAP_CLASSNAME::Extraction
    MACRO_HASHMAP_CLASSNAME::extract_bucket_and_advance
    (BucketIterator && bucket, const Iterator & iterator) noexcept
{
    // note the order:
    // extract bucket contents before
    // destroying the element object and setting the key to empty before
    // advancing the iterator
    --m_size;

    auto & el = *reinterpret_cast<ElementType *>(&bucket->second);
    auto returned_el = std::move(el);
    el.~ElementType();

    auto key = std::move(bucket->first);
    bucket->first = m_empty_key;

    return Extraction
        {advance_past_empty(Iterator{iterator}),
         std::move(returned_el),
         std::move(key)};
}

MACRO_HASHMAP_TEMPLATES
template <typename OtherKeyType>
/* private */ std::size_t MACRO_HASHMAP_CLASSNAME::find_impl
    (const OtherKeyType & key) const noexcept
{
    if (KeyEquality{}(key, m_empty_key))
        { return m_bucket_container.size(); }

    auto index = key_to_index(key);
    while (true) {
        auto & bucket = m_bucket_container[index];
        if (KeyEquality{}(bucket.first, key))
            { return index; }
        if (KeyEquality{}(bucket.first, m_empty_key))
            { return m_bucket_container.size(); }
        index = probe_next(index);
    }
}

#undef MACRO_ITERATOR_TEMPLATES
#undef MACRO_ITERATOR_CLASSNAME
#undef MACRO_HASHMAP_TEMPLATES
#undef MACRO_HASHMAP_CLASSNAME

} // end of cul namespace
