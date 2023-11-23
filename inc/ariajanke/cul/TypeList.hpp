/****************************************************************************

    MIT License

    Copyright (c) 2022 Aria Janke

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

#include <type_traits>

namespace cul {

template <typename T> struct TypeTag final {};

/** @brief Utility class used to define a sequential, non-unique list of types.
 *
 *  Various type aliases and template constants within are provided as
 *  utilities for working with lists of types.
 */
template <typename ... Types>
class TypeList {
#   ifndef DOXYGEN_SHOULD_SKIP_THIS
    template <typename ... OtherTypes>
    struct Concatenate_ {
        using List = TypeList<OtherTypes...>;
    };

    template <typename ... OtherTypes>
    struct EqualList_ {
        static constexpr bool k_value = true;
    };

    template <typename Head, typename ... OtherTypes>
    struct EqualList_<Head, OtherTypes...> {
        static constexpr bool k_value = false;
    };

    template <typename ... OtherTypes>
    struct EqualList_<TypeList<OtherTypes...>> :
        public EqualList_<OtherTypes...> {};

    template <int kt_begin, int kt_end>
    struct Slice_ {
        static_assert(kt_begin == 0 && kt_end == 0,
            "TypeList::Slice: primary template must end with begin and end going to 0.");
        using List = TypeList<>;
    };

    template <int kt_idx>
    struct TypeAtIndex_ {
        static_assert(kt_idx == 0,
            "TypeList::TypeAtIndex: index must be 0 at primary template.");
        using Type = TypeTag<void>;
    };
#   endif
public:

    /** A type alias, which maybe used to concatenate this and another type
     *  list.
     *
     *  @tparam OtherTypes Types appended at the tail of this list. If you
     *          specify a type list in place of "OtherTypes...", then the
     *          compiler will append it's "member" types.
     */
    template <typename ... OtherTypes>
    using Concatenate = typename Concatenate_<OtherTypes...>::List;

    /** The number of types in the list. */
    static constexpr int k_count = sizeof...(Types);

    /** Defines a type, with another template that takes k_count number of
     *  template.
     *
     * @tparam Other a template, e.g. std::tuple, std::variant
     */
    template <template <typename ...> class Other>
    using DefineWithTypes = Other<Types...>;

    /** Is true if either the given type list or "OtherType..." are the same
     *  sequence of types as this list.
     *
     *  False indicates a mismatch in types.
     */
    template <typename ... OtherTypes>
    static constexpr bool kt_equal_to_list = EqualList_<OtherTypes...>::k_value;

    /** The value of this is the index at which the given type is found in the
     *  typelist. If not found, it is -1.
     *
     *  @note zero based index
     */
    template <typename T>
    static constexpr int kt_find_index_for_type = -1;

    /** @brief Small utility for splitting the type list in half, with a middle
     *  type.
     *
     *  This could be useful in navigating/implementing features that go
     *  between template paramaters and runtime variables.
     */
    struct Fork final {

        /** Value of this constant indicates the index where the middle type is
         *  located on the parent type list.
         *
         *  If there is no such "middle type", then this value is -1.
         */
        static constexpr int k_middle_index = -1;

        /** The middle type, if there is no such type it defaults to
         *  TypeTag<void>.
         */
        using MiddleType = TypeTag<void>;

        /** TypeList of all types to the "left" of the middle type.
         *
         *  By left, meaning all types from index 0 up until (k_count / 2 - 1)
         *  (inclusive). If there are no such types to the left, then this type
         *  is an empty type list.
         */
        using Left = TypeList<>;

        /** TypeList of all types to the "right" of the middle type.
         *
         *  By right, meaning all types from index
         *  max(k_count / 2 + 1, k_count - 1) up until (k_count - 1)
         *  (inclusive). If there are no such types, then this is an empty type
         *  list.
         */
        using Right = TypeList<>;
    };

    /** Defines a type list, with T as the "head" or first type in the list.
     */
    template <typename T>
    using ListWithHead = TypeList<T>;

    /** Number of times the type "T" occur in the list */
    template <typename T>
    static constexpr int kt_occurance_count = 0;

    /** Defines a type list, with each type for whom the "PredicateTemplate"
     *  evaluates to true being absent.
     *
     *  @tparam PredicateTemplate a template boolean constant, define like:
     @code
     template <typename T>
     constexpr const bool kt_should_remove_type = ...;
     @endcode
     */
    template <template <typename> class PredicateTemplate>
    using RemoveIf = TypeList<>;

    /** Defines a type list containing the types whose indices are in the given
     *  range.
     *
     *  The interval given is treated like a STL range. That is all types
     *  contained in [k_begin k_end).
     *
     *  @tparam k_begin an integer that must be between 0 and k_count, it must
     *                  be not greater than k_end
     *  @tparam k_end an integer that must be between 0 and k_count,
     *          consequentially must not be less than k_begin
     *  @note Slice maybe thought up as similar to JavaScript Array's slice,
     *        however it is less flexible, and requires the use of non-negative
     *        integers only.
     */
    template <int k_begin, int k_end = sizeof...(Types)>
    using Slice = typename Slice_<k_begin, k_end>::List;

    /** This type list contains all the same types minus the first type.
     *
     *  This is an empty type list if this type list contains one or fewer
     *  types.
     */
    using TailList = TypeList<>;

    /** Defines a type list, with each "member" type being redefined according
     *  to the given type alias.
     *
     *  @tparam Transformer a type alias, which accepts one type as an argument,
     *          that type will be one type of the parent type list.
     */
    template <template <typename> class Transformer>
    using Transform = TypeList<>;

    /** Type alias to a type at the given index.
     *
     *  If there are no types in this TypeList, then it defines a
     *  TypeTag<void>. (Which I may change later)
     *
     *  @tparam k_idx The index of a sought type, it must be between 0 and
     *          (k_count - 1) (inclusive).
     */
    template <int k_idx>
    using TypeAtIndex = TypeTag<void>;
};

/** @copydoc cul::TypeList */
template <typename Head, typename ... Types>
class TypeList<Head, Types...> {
#   ifndef DOXYGEN_SHOULD_SKIP_THIS
    using TailList_ = TypeList<Types...>;

    template <typename ... OtherTypes>
    struct Concatenate_ {
        using List = TypeList<Head, Types..., OtherTypes...>;
    };

    template <typename ... OtherTypes>
    struct Concatenate_<TypeList<OtherTypes...>> :
        public Concatenate_<OtherTypes...> {};

    template <typename ... OtherTypes>
    struct EqualList_ {
        static constexpr bool k_value = false;
    };

    template <typename OtherHead, typename ... OtherTypes>
    struct EqualList_<OtherHead, OtherTypes...> {
        static constexpr bool k_value =
               std::is_same_v<Head, OtherHead>
            && TailList_::template kt_equal_to_list<OtherTypes...>;
    };

    template <typename ... OtherTypes>
    struct EqualList_<TypeList<OtherTypes...>> :
        public EqualList_<OtherTypes...> {};

    template <typename T>
    struct OccuranceCount_ {
        static constexpr int k_value =
              int(std::is_same_v<Head, T>)
            + TailList_::template kt_occurance_count<T>;
    };

    template <typename T>
    using ListWithHead_ = TypeList<T, Head, Types...>;

    // head is taken at index 0
    template <int kt_beg, int kt_end>
    struct Slice_ {
        static_assert(kt_end >= kt_beg && kt_beg >= 0 && kt_end >= 0,
            "TypeList::Slice: kt_beg and kt_end must be non-negative integers, "
            "kt_end must be equal to or greater than kt_beg.");
        static_assert(kt_end <= 1 + sizeof...(Types),
            "TypeList::Slice: kt_end may not be greater than the number of "
            "types in the list.");
        // does head belong in the slice?
        using Base_ = typename TailList_::template
            Slice<kt_beg == 0 ? 0 : kt_beg - 1,
                  kt_end == 0 ? 0 : kt_end - 1>;

        // off by one error somewhere
        using List = std::conditional_t<kt_beg == 0 && (kt_end > 0),
            typename Base_::template ListWithHead<Head>,
            Base_>;
    };

    template <template <typename> class PredicateTemplate>
    struct RemoveIf_ {
        static constexpr const bool k_should_remove_head =
            std::is_base_of_v<std::true_type, PredicateTemplate<Head>>;
        using RiTailList = typename TailList_::template RemoveIf<PredicateTemplate>;
        using List = std::conditional_t<k_should_remove_head,
            RiTailList,
            typename RiTailList::template ListWithHead<Head>
        >;
    };

    template <int kt_idx>
    struct TypeAtIndex_ {
        static_assert(kt_idx >= 0 && kt_idx <= sizeof...(Types),
            "TypeList::TypeAtIndex: index must be between 0 and last available "
            "index (number of types minus one).");
        using Type = std::conditional_t<kt_idx == 0,
            Head,
            typename TailList_::template TypeAtIndex<kt_idx == 0 ? 0 : kt_idx - 1>>;
    };

    template <typename T>
    struct FindIndexForType_ {
        static constexpr int k_value =
            std::is_same_v<Head, T> ? 0 : TailList_::template kt_find_index_for_type<T> + 1;
    };
#   endif
public:

    /** @copydoc cul::TypeList::Concatenate */
    template <typename ... OtherTypes>
    using Concatenate = typename Concatenate_<OtherTypes...>::List;

    /** @copydoc cul::TypeList::k_count */
    static constexpr int k_count = 1 + sizeof...(Types);

    /** @copydoc cul::TypeList::DefineWithTypes */
    template <template <typename ...> class Other>
    using DefineWithTypes = Other<Head, Types...>;

    /** @copydoc cul::TypeList::kt_equal_to_list */
    template <typename ... OtherTypes>
    static constexpr bool kt_equal_to_list = EqualList_<OtherTypes...>::k_value;

    /** @copydoc cul::TypeList::kt_find_index_for_type */
    template <typename T>
    static constexpr int kt_find_index_for_type = FindIndexForType_<T>::k_value;

    /** @copydoc cul::TypeList::Fork */
    struct Fork final {
        /** @copydoc cul::TypeList::Fork::k_middle_index */
        static constexpr int k_middle_index = k_count / 2;

        /** @copydoc cul::TypeList::Fork::MiddleType */
        using MiddleType = typename TypeAtIndex_<k_middle_index>::Type;

        /** @copydoc cul::TypeList::Fork::Left */
        using Left = typename Slice_<0, k_middle_index>::List;

        /** @copydoc cul::TypeList::Fork::Right */
        using Right = typename Slice_<
            (k_middle_index + 1 > k_count ? k_count : k_middle_index + 1),
             k_count>::List;
    };

    /** @copydoc cul::TypeList::ListWithHead */
    template <typename T>
    using ListWithHead = ListWithHead_<T>;

    /** @copydoc cul::TypeList::kt_occurance_count */
    template <typename T>
    static constexpr int kt_occurance_count = OccuranceCount_<T>::k_value;

    /** @copydoc cul::TypeList::RemoveIf */
    template <template <typename> class PredicateTemplate>
    using RemoveIf = typename RemoveIf_<PredicateTemplate>::List;

    /** @copydoc cul::TypeList::Slice */
    template <int k_begin, int k_end = sizeof...(Types) + 1>
    using Slice = typename Slice_<k_begin, k_end>::List;

    /** @copydoc cul::TypeList::TailList */
    using TailList = TailList_;

    /** @copydoc cul::TypeList::Transform */
    template <template <typename> class Transformer>
    using Transform = typename TailList_::template Transform<Transformer>
        ::template ListWithHead<Transformer<Head>>;

    /** @copydoc cul::TypeList::TypeAtIndex */
    template <int kt_idx>
    using TypeAtIndex = typename TypeAtIndex_<kt_idx>::Type;
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS

namespace detail {

class TypeListTestsPrivMore {
    template <typename T>
    struct RemoveIfTest1 final : public std::false_type {};
};

template <>
struct TypeListTestsPrivMore::RemoveIfTest1<int> final :
    public std::true_type {};

class TypeListTestsPriv : public TypeListTestsPrivMore {
    struct A final {};
    struct B final {};
    struct C final {};
    struct D final {};
    struct E final {};
    struct F final {};

    TypeListTestsPriv() {}
#   ifdef MACRO_ARIAJANKE_CUL_ENABLE_TYPELIST_TESTS
    // "equal to list" first, as other tests depend on it
    static_assert(TypeList<>::kt_equal_to_list<>);
    static_assert(TypeList<>::kt_equal_to_list<TypeList<>>);
    static_assert(!TypeList<A>::kt_equal_to_list<TypeList<>>);
    static_assert(!TypeList<>::kt_equal_to_list<TypeList<A>>);
    static_assert(TypeList<A>::kt_equal_to_list<TypeList<A>>);
    static_assert(!TypeList<A>::kt_equal_to_list<TypeList<A, B>>);
    static_assert(TypeList<A, B>::kt_equal_to_list<A, B>);
    static_assert(TypeList<A, B>::kt_equal_to_list<TypeList<A, B>>);
    static_assert(!TypeList<B, A>::kt_equal_to_list<TypeList<A, B>>);
    static_assert(TypeList<A, C, B>::kt_equal_to_list<A, C, B>);
    static_assert(TypeList<A, C, B>::kt_equal_to_list<TypeList<A, C, B>>);
    static_assert(!TypeList<A, B, C>::kt_equal_to_list<TypeList<A, C, B>>);
    // Concatenate
    static_assert(TypeList<>::Concatenate<C>::kt_equal_to_list<C>);
    static_assert(TypeList<A>::Concatenate<>::kt_equal_to_list<A>);
    static_assert(TypeList<A, C>::Concatenate<>::kt_equal_to_list<A, C>);
    static_assert(TypeList<A>::Concatenate<C>::kt_equal_to_list<A, C>);
    static_assert(TypeList<A, C>::Concatenate<TypeList<A, B>>::kt_equal_to_list<A, C, A, B>);
    static_assert(!TypeList<A, C>::Concatenate<TypeList<A, B>>::kt_equal_to_list<A, C, A, B, E>);
    static_assert(!TypeList<A, C>::Concatenate<TypeList<A, E, B>>::kt_equal_to_list<A, C, A, E>);
    static_assert(!TypeList<A, C, E>::Concatenate<TypeList<A, B>>::kt_equal_to_list<A, C, E, A>);
    // kt_find_index_for_type
    static_assert(TypeList<A>::kt_find_index_for_type<A> == 0);
    static_assert(TypeList<>::kt_find_index_for_type<A> == -1);
    static_assert(TypeList<A, B, C, C>::kt_find_index_for_type<C> == 2);
    // Slice
    static_assert(TypeList<>::Slice<0, 0>::kt_equal_to_list<>);
    static_assert(TypeList<A>::Slice<0, 0>::kt_equal_to_list<>);
    static_assert(TypeList<A, B, C>::Slice<1>::kt_equal_to_list<B, C>);
    static_assert(TypeList<A, B, C>::Slice<1, 2>::kt_equal_to_list<B>);
    static_assert(TypeList<A, B, C>::Slice<0, 2>::kt_equal_to_list<A, B>);
    static_assert(TypeList<A, B, C, E, F>::Slice<0, 3>::kt_equal_to_list<A, B, C>);
    static_assert(TypeList<A, B, C, E, F>::Slice<2, 5>::kt_equal_to_list<C, E, F>);
    static_assert(TypeList<A, B, C, E, F>::Slice<4, 5>::kt_equal_to_list<F>);
    static_assert(TypeList<A, B, C, E, F>::Slice<2, 3>::kt_equal_to_list<C>);
    static_assert(TypeList<A, B, C, E, F>::Slice<2, 4>::kt_equal_to_list<C, E>);
    static_assert(!TypeList<A, B, C, E, F>::Slice<2, 3>::kt_equal_to_list<C, E>);
    static_assert(!TypeList<A, B, C, E, F>::Slice<2, 4>::kt_equal_to_list<C, E, F>);
    // RemoveIf
    template <typename T>
    using RemoveIfTest1 = TypeListTestsPrivMore::RemoveIfTest1<T>;
    static_assert(TypeList<>::RemoveIf<RemoveIfTest1>::kt_equal_to_list<>);
    static_assert(TypeList<int>::RemoveIf<RemoveIfTest1>::kt_equal_to_list<>);
    // Fork
    static_assert(std::is_same_v<TypeList<>::Fork::MiddleType, TypeTag<void>>);
    static_assert(TypeList<>::Fork::Left::kt_equal_to_list<>);
    static_assert(TypeList<>::Fork::Right::kt_equal_to_list<>);
    static_assert(std::is_same_v<TypeList<A, B, C, E, F>::Fork::MiddleType, C>);
    static_assert(TypeList<A, B, C, E, F>::Fork::Left::kt_equal_to_list<A, B>);
    static_assert(TypeList<A, B, C, E, F>::Fork::Right::kt_equal_to_list<E, F>);
    static_assert(std::is_same_v<TypeList<A, B>::Fork::MiddleType, B>);
    static_assert(TypeList<A, B>::Fork::Right::kt_equal_to_list<>);
    static_assert(TypeList<A, B, C, E, F>
        ::Fork::Left // get <A, B>, as test before
        ::Fork::Right::kt_equal_to_list<>);
    static_assert(TypeList<A, B, C, E, F>::Fork::Left::Fork::Right::Fork::Left::kt_equal_to_list<>);
    // kt_occurance_count
    static_assert(TypeList<A>::kt_occurance_count<B> == 0);
    static_assert(TypeList<>::kt_occurance_count<B> == 0);
    static_assert(TypeList<B, B, B>::kt_occurance_count<B> == 3);
    static_assert(TypeList<A, B, B>::kt_occurance_count<B> == 2);
    static_assert(TypeList<B, A, B, B>::kt_occurance_count<B> == 3);
    static_assert(TypeList<B, A, C, B, F, B>::kt_occurance_count<B> == 3);
#   endif
};

} // end of cul::detail namespace -> into ::cul

#endif

} // end of cul namespace
