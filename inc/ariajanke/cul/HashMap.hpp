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
    typename Hash = std::hash<KeyT>,
    typename KeyEqualT = std::equal_to<void>,
    typename AllocatorT = std::allocator<std::pair<KeyT, std::aligned_storage_t<sizeof(ElementT)>>>
>
class HashMap {
public:
    using KeyType = KeyT;
    using ElementType = ElementT;
    using KeyValuePairType = std::pair<KeyType, ElementType>;
    using Hasher = Hash;
    using KeyEquality = KeyEqualT;
    using Allocator = AllocatorT;
    using Bucket = std::pair<KeyType, std::aligned_storage_t<sizeof(ElementType)>>;
    using BucketContainer = std::vector<Bucket, Allocator>;
    using BucketIterator = typename BucketContainer::iterator;
    using BucketConstIterator = typename BucketContainer::const_iterator;
    template <typename T>
    using Optional = std::optional<T>;

    template <bool kt_is_constant>
    struct PairWrapper final {
    public:
        using DataPtr = std::conditional_t
            <kt_is_constant, const void *, void *>;

        using ElementPtr = std::conditional_t
            <kt_is_constant, const ElementType *, ElementType *>;

        using ElementReference = std::conditional_t
            <kt_is_constant, const ElementType &, ElementType &>;

        using PairType = std::pair<const KeyType &, ElementReference>;

        PairWrapper(const KeyType & key, DataPtr el_ref):
            m_proxy(key, *reinterpret_cast<ElementPtr>(el_ref)) {}

        const PairType * operator -> () const { return &m_proxy; }

    private:
        PairType m_proxy;
    };

    template <bool kt_is_constant>
    class IteratorImpl final {
    public:
        using PairWrapperImpl = PairWrapper<kt_is_constant>;

        using ElementReference = typename PairWrapperImpl::ElementReference;

        using BucketContainerPtr = std::conditional_t
            <kt_is_constant, const BucketContainer *, BucketContainer *>;

        using Reference = std::conditional_t
            <kt_is_constant, const KeyValuePairType &, KeyValuePairType &>;

        using Pointer = std::conditional_t
            <kt_is_constant, const KeyValuePairType *, KeyValuePairType *>;

        // defs for std algorithms
        using difference_type = std::ptrdiff_t;
        using value_type = PairWrapperImpl;
        using pointer = PairWrapperImpl;
        using reference = typename PairWrapperImpl::PairType;
        using iterator_category = std::bidirectional_iterator_tag;

        IteratorImpl
            (std::size_t index,
             BucketContainerPtr container,
             const KeyType & empty);

        IteratorImpl(const IteratorImpl &);

        IteratorImpl(IteratorImpl &&);

        IteratorImpl & operator = (const IteratorImpl &);

        IteratorImpl & operator = (IteratorImpl &&);

        bool operator == (const IteratorImpl & rhs) const
            { return equal_to(rhs); }

        bool operator != (const IteratorImpl & rhs) const
            { return !equal_to(rhs); }

        IteratorImpl & operator++ ();

        IteratorImpl advance_past_empty() &&;

        PairWrapperImpl operator * () const { return element(); }

        // a huge problem actually...
        // It's possible with a proxy type
        PairWrapperImpl operator -> () const { return element(); }

    private:
        friend class HashMap;

        bool equal_to(const IteratorImpl &) const;

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

    HashMap(KeyType empty_key,
            BucketContainer && = BucketContainer{},
            const Allocator & = Allocator{});

    HashMap(const HashMap &);

    HashMap(HashMap &&);

    HashMap & operator = (const HashMap &);

    HashMap & operator = (HashMap &&);

    ConstIterator begin() const noexcept { return cbegin(); }

    Iterator begin() noexcept { return make_iterator(0).advance_past_empty(); }

    std::size_t capacity() const noexcept
        { return m_bucket_container.size() / k_load_factor; }

    ConstIterator cbegin() const noexcept
        { return make_iterator(0).advance_past_empty(); }

    ConstIterator cend() const noexcept
        { return make_iterator(m_bucket_container.size() - 1); }

    void clear() noexcept;

    template <typename... Types>
    Insertion emplace(Types &&... args)
        { return emplace_impl(std::forward<Types>(args)...); }

    ConstIterator end() const noexcept { return cend(); }

    Iterator end() noexcept
        { return make_iterator(m_bucket_container.size() - 1); }

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

    bool is_empty() const noexcept { return m_size == 0; }

    BucketContainer move_out_buckets();

    void rehash(std::size_t for_at_least_this_many_elements = 0);

    void reserve(std::size_t for_at_least_this_many_elements);

    std::size_t size() const noexcept { return m_size; }

    void swap(HashMap &);

private:
    static constexpr const std::size_t k_load_factor = 2;

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

    KeyType m_empty_key;
    BucketContainer m_bucket_container;
    std::size_t m_size = 0;
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
        typename Hash, \
        typename KeyEqualT, \
        typename AllocatorT \
    >

#define MACRO_HASHMAP_CLASSNAME \
    HashMap<KeyT, ElementT, Hash, KeyEqualT, AllocatorT>

MACRO_HASHMAP_TEMPLATES
template <bool kt_is_constant>
MACRO_HASHMAP_CLASSNAME::IteratorImpl<kt_is_constant>::IteratorImpl
    (std::size_t index_,
     BucketContainerPtr container_,
     const KeyType & empty_key_):
    m_index(index_),
    m_container(container_),
    m_empty_key(&empty_key_) {}

MACRO_HASHMAP_TEMPLATES
template <bool kt_is_constant>
MACRO_HASHMAP_CLASSNAME::IteratorImpl<kt_is_constant>::IteratorImpl
    (const IteratorImpl & rhs):
    m_index(rhs.m_index),
    m_container(rhs.m_container),
    m_empty_key(rhs.m_empty_key) {}

MACRO_HASHMAP_TEMPLATES
template <bool kt_is_constant>
MACRO_HASHMAP_CLASSNAME::IteratorImpl<kt_is_constant>::IteratorImpl
    (IteratorImpl && rhs):
    m_index(std::move(rhs.m_index)),
    m_container(std::move(rhs.m_container)),
    m_empty_key(std::move(rhs.m_empty_key)) {}

MACRO_HASHMAP_TEMPLATES
template <bool kt_is_constant>
typename MACRO_HASHMAP_CLASSNAME::template IteratorImpl<kt_is_constant> &
    MACRO_HASHMAP_CLASSNAME::template IteratorImpl<kt_is_constant>::
    operator = (const IteratorImpl & rhs)
{
    if (this != &rhs) {
        m_index = rhs.m_index;
        m_container = rhs.m_container;
        m_empty_key = rhs.m_empty_key;
    }
    return *this;
}

MACRO_HASHMAP_TEMPLATES
template <bool kt_is_constant>
typename MACRO_HASHMAP_CLASSNAME::template IteratorImpl<kt_is_constant> &
    MACRO_HASHMAP_CLASSNAME::template IteratorImpl<kt_is_constant>::
    operator = (IteratorImpl && rhs)
{
    if (this != &rhs) {
        std::swap(m_index    , rhs.m_index    );
        std::swap(m_container, rhs.m_container);
        std::swap(m_empty_key, rhs.m_empty_key);
    }
    return *this;
}

MACRO_HASHMAP_TEMPLATES
template <bool kt_is_constant>
typename MACRO_HASHMAP_CLASSNAME::template IteratorImpl<kt_is_constant> &
    MACRO_HASHMAP_CLASSNAME::IteratorImpl<kt_is_constant>::operator++ ()
{
    ++m_index;
    advance_past_empty_();
    return *this;
}

MACRO_HASHMAP_TEMPLATES
template <bool kt_is_constant>
typename MACRO_HASHMAP_CLASSNAME::template IteratorImpl<kt_is_constant>
    MACRO_HASHMAP_CLASSNAME::template IteratorImpl<kt_is_constant>::
    advance_past_empty() &&
{
    advance_past_empty_();
    return IteratorImpl{m_index, m_container, *m_empty_key};
}

MACRO_HASHMAP_TEMPLATES
template <bool kt_is_constant>
/* private */ void MACRO_HASHMAP_CLASSNAME::template IteratorImpl<kt_is_constant>::
    advance_past_empty_()
{
    while (m_index < m_container->size() && KeyEquality{}(key(), *m_empty_key))
        { ++m_index; }
}

MACRO_HASHMAP_TEMPLATES
template <bool kt_is_constant>
/* private */ bool MACRO_HASHMAP_CLASSNAME::IteratorImpl<kt_is_constant>::
    equal_to(const IteratorImpl & rhs) const
{ return m_index == rhs.m_index && m_container == rhs.m_container; }

// ----------------------------------------------------------------------------

MACRO_HASHMAP_TEMPLATES
MACRO_HASHMAP_CLASSNAME::HashMap
    (KeyType empty_key_,
     BucketContainer && buckets_,
     const Allocator & allocator_):
    m_empty_key(empty_key_),
    m_bucket_container(std::move(buckets_), allocator_)
{
    if (!buckets_.empty())
        { rehash(); }
}

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
{ return extract(iterator).first; }

MACRO_HASHMAP_TEMPLATES
typename MACRO_HASHMAP_CLASSNAME::Extraction
    MACRO_HASHMAP_CLASSNAME::extract(const Iterator & iterator)
{
    auto index  = iterator.m_index;
    auto bucket = m_bucket_container.begin() + index;
    const auto beg_ = m_bucket_container.begin();
    while (true) {
        index = probe_next(index);
        assert(index < m_bucket_container.size());
        auto itr = beg_ + index;
        // Why did I add this or? This is why tests are a thing
        if (KeyEquality{}(itr->first, m_empty_key) /* || index < (bucket - ...)*/) {
            --m_size;
            auto & el = *reinterpret_cast<ElementType *>(&bucket->second);
            auto ex = Extraction
                {Iterator{iterator}.advance_past_empty(),
                 std::move(el),
                 std::move(bucket->first)};
            bucket->first = m_empty_key;
            el.~ElementType();
            return ex;
        }

        auto ideal = m_bucket_container.begin() + key_to_index(itr->first);
        // is bucket_iterator closer to ideal than itr?
        if (magnitude(bucket - ideal) < magnitude(itr - ideal)) {
            // uh oh, can't do this anymore...
            // we'll have to change this behavior fundementally...

#           if 0 // save until after tests (and NTS get a test to hit this branch)
            *bucket = *ideal; // how is this a swap?
#           endif
            // purpose this to be "unsafe move"
            {
            auto & bucket_el = *reinterpret_cast<ElementType *>(&bucket->second);
            auto & ideal_el  = *reinterpret_cast<ElementType *>(&ideal ->second);
            bucket->first = std::move(ideal->first);
            ideal->first = m_empty_key;
            bucket_el = std::move(ideal_el);
            ideal_el.~ElementType();
            }
            bucket = itr;
        }
    }
}

MACRO_HASHMAP_TEMPLATES
template <typename OtherKeyType>
typename MACRO_HASHMAP_CLASSNAME::Iterator
    MACRO_HASHMAP_CLASSNAME::find(const OtherKeyType & key)
{
    if (KeyEquality{}(key, m_empty_key))
        { return end(); }

    auto index = key_to_index(key);
    while (true) {
        auto & bucket = m_bucket_container[index];
        if (KeyEquality{}(bucket.first, key))
            { return make_iterator(index); }
        if (KeyEquality{}(bucket.first, m_empty_key))
            { return end(); }
        index = probe_next(index);
    }
}

MACRO_HASHMAP_TEMPLATES
template <typename OtherKeyType>
typename MACRO_HASHMAP_CLASSNAME::ConstIterator
    MACRO_HASHMAP_CLASSNAME::find(const OtherKeyType & key) const
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
    m_bucket_container.resize
        (detail::nearest_base2_number
            (for_at_least_this_many_elements*k_load_factor));
}

MACRO_HASHMAP_TEMPLATES
void MACRO_HASHMAP_CLASSNAME::swap(HashMap & rhs) {
    m_bucket_container.swap(rhs.m_bucket_container);
    std::swap(m_empty_key, rhs.m_empty_key);
    std::swap(m_size     , rhs.m_size     );
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
std::size_t MACRO_HASHMAP_CLASSNAME::probe_next(std::size_t idx) const noexcept
    { return (idx + 1) & size_mask(); }

#undef MACRO_HASHMAP_TEMPLATES
#undef MACRO_HASHMAP_CLASSNAME

} // end of cul namespace
