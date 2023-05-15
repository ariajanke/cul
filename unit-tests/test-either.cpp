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

#include <ariajanke/cul/TreeTestSuite.hpp>
#include <ariajanke/cul/detail/either-helpers.hpp>
#include <ariajanke/cul/OptionalEither.hpp>
#include <ariajanke/cul/Either.hpp>

#include <memory>

namespace {

using namespace cul;
using namespace tree_ts;

#define mark_it mark_source_position(__LINE__, __FILE__).it

class SomeError final {};
class SomeThing final {};

}

// I have to rebuild the whole damn thing, because of one fucking caveat with
// constructors. I'm still stuck with C++17, because we're too cool to update
// our compilers, right Apple? Fuck you

// I'll be stuck with a fixed number of template parameters, but it's what I'm
// stuck with. I don't want to spend three hours trying to find an obsecure
// work around for n types.


template <typename LeftT, typename RightT>
class VisisibleShit;

class ShitWrapper final {

template <typename LeftT, typename RightT>
friend class VisisibleShit;

template <typename LeftT, typename RightT,
          bool kt_is_copy_enabled = std::is_copy_constructible_v<LeftT> &&
                                    std::is_copy_constructible_v<RightT>,
          bool kt_is_move_enabled = std::is_move_constructible_v<LeftT> &&
                                    std::is_move_constructible_v<RightT>>
struct Shit {
    static_assert(std::is_copy_constructible_v<LeftT> ==
                  std::is_copy_assignable_v<LeftT>      );
    static_assert(std::is_copy_constructible_v<RightT> ==
                  std::is_copy_assignable_v<RightT>      );

    Shit(): stored_shit(std::monostate{}) {}
    Shit(const Shit & shit): stored_shit(shit.stored_shit) {}
    Shit(Shit && shit): stored_shit(std::move(shit.stored_shit)) {}

    Shit & operator = (const Shit & shit) {
        if (this != &shit) {
            stored_shit = shit.stored_shit;
        }
        return *this;
    }

    Shit & operator = (Shit && shit) {
        if (this != &shit) {
            stored_shit = std::move(shit.stored_shit);
        }
        return *this;
    }
protected:
    template <std::size_t I, typename ... Types>
    Shit(std::in_place_index_t<I>, Types &&... args):
        stored_shit(std::in_place_index_t<I>{}, std::forward<Types>(args)...) {}

    std::variant<LeftT, RightT, std::monostate> stored_shit;
};

template <typename LeftT, typename RightT>
struct Shit<LeftT, RightT, true, false> : Shit<LeftT, RightT, true, true> {
    using Shit<LeftT, RightT, true, true>::Shit;

    Shit(const Shit &) = default;
    Shit(Shit &&) = delete;

    Shit & operator = (const Shit &) = default;
    Shit & operator = (Shit &&) = delete;
};

template <typename LeftT, typename RightT>
struct Shit<LeftT, RightT, false, true> : Shit<LeftT, RightT, true, true> {
    using Shit<LeftT, RightT, true, true>::Shit;

    Shit(const Shit &) = delete;
    Shit(Shit &&) = default;

    Shit & operator = (const Shit &) = delete;
    Shit & operator = (Shit &&) = default;
};


template <typename LeftT, typename RightT>
struct Shit<LeftT, RightT, false, false> : Shit<LeftT, RightT, true, true> {
    using Shit<LeftT, RightT, true, true>::Shit;

    Shit(const Shit &) = delete;
    Shit(Shit &&) = delete;

    Shit & operator = (const Shit &) = delete;
    Shit & operator = (Shit &&) = delete;
};

};

// OPE there goes making the interface visible at all lmfao
template <typename LeftT, typename RightT>
class VisisibleShit final : public ShitWrapper::Shit<LeftT, RightT> {
    using Super = ShitWrapper::Shit<LeftT, RightT>;
public:
    using Super::Super;

    template <std::size_t I, typename ... Types>
    VisisibleShit(std::in_place_index_t<I>,
                  Types &&... args):
        Super(std::in_place_index_t<I>{}, std::forward<Types>(args)...) {}

    LeftT left() {
        return std::get<LeftT>(std::move(this->stored_shit));
    }

    // OMG I LOVE this->
    bool is_left() const { return this->stored_shit.index() == 0; }

};



auto x = [] {

// no copy, no move, no default
{
using TestConstructors =
    detail::EitherConstructors
        <SomeError, SomeThing,
         detail::EitherDefaultConstructor::disable,
         detail::EitherCopyConstructor::disable,
         detail::EitherMoveConstructor::disable>;
#if 1 == MACRO_ARIAJANKE_CUL_EXPECT_FAILED_COMPILATION
TestConstructors{};
#elif 2 == MACRO_ARIAJANKE_CUL_EXPECT_FAILED_COMPILATION
TestConstructors{SomeThing{}};
#elif 3 == MACRO_ARIAJANKE_CUL_EXPECT_FAILED_COMPILATION
TestConstructors{SomeError{}};
#endif
(void)TypeTag<TestConstructors>{};
}
// no copy, no move, default
{
using TestConstructors =
    detail::EitherConstructors
        <SomeError, SomeThing,
         detail::EitherDefaultConstructor::enable,
         detail::EitherCopyConstructor::disable,
         detail::EitherMoveConstructor::disable>;
TestConstructors{};
}
// no copy, move, no default
{
using TestConstructors =
    detail::EitherConstructors
        <SomeError, SomeThing,
         detail::EitherDefaultConstructor::disable,
         detail::EitherCopyConstructor::disable,
         detail::EitherMoveConstructor::enable>;
#if 0
(void)TestConstructors{}; // <- default
#endif
(void)TestConstructors{SomeThing{}}; // <- right move
(void)TestConstructors{SomeError{}}; // <- left move
(void)TestConstructors{TypeTag<SomeError>{}, SomeThing{}}; // <- unambiguous left move
(void)TestConstructors{SomeError{}, TypeTag<SomeThing>{}}; // <- unambiguous right move
(void)TestConstructors{TestConstructors{SomeThing{}}}; // <- regular move
#if 0
{
    TestConstructors a{SomeThing{}};
    TestConstructors{a}; // <- regular copy ctor
}
#elif 0
{
    SomeThing a;
    TestConstructors{a}; // <- right copy ctor
}
#elif 0
{
    SomeError a;
    TestConstructors{a}; // <- left copy ctor
}
#elif 0
{
    SomeThing a;
    TestConstructors{TypeTag<SomeError>{}, a}; // <- unambiguous right copy ctor
}
#elif 0
{
    SomeError a;
    TestConstructors{a, TypeTag<SomeThing>{}}; // <- unambiguous left copy ctor
}
#endif
{
    TestConstructors a{SomeThing{}};
    TestConstructors b{SomeThing{}};
    b = std::move(a); // <- move assignment
}
#if 0
{
    TestConstructors a{SomeThing{}};
    TestConstructors b{SomeThing{}};
    b = a; // <- copy assignment
}
#endif
}
// no copy, move, default
{
using TestConstructors =
    detail::EitherConstructors
        <SomeError, SomeThing,
         detail::EitherDefaultConstructor::enable,
         detail::EitherCopyConstructor::disable,
         detail::EitherMoveConstructor::enable>;

(void)TestConstructors{}; // <- default
(void)TestConstructors{SomeThing{}}; // <- right move
(void)TestConstructors{SomeError{}}; // <- left move
(void)TestConstructors{TypeTag<SomeError>{}, SomeThing{}}; // <- unambiguous left move
(void)TestConstructors{SomeError{}, TypeTag<SomeThing>{}}; // <- unambiguous right move
(void)TestConstructors{TestConstructors{SomeThing{}}}; // <- regular move
#if 0
{
    TestConstructors a{SomeThing{}};
    TestConstructors{a}; // <- regular copy ctor
}
#elif 0
{
    SomeThing a;
    TestConstructors{a}; // <- right copy ctor
}
#elif 0
{
    SomeError a;
    TestConstructors{a}; // <- left copy ctor
}
#elif 0
{
    SomeThing a;
    TestConstructors{TypeTag<SomeError>{}, a}; // <- unambiguous right copy ctor
}
#elif 0
{
    SomeError a;
    TestConstructors{a, TypeTag<SomeThing>{}}; // <- unambiguous left copy ctor
}
#endif
{
    TestConstructors a{SomeThing{}};
    TestConstructors b{SomeThing{}};
    b = std::move(a); // <- move assignment
}
#if 0
{
    TestConstructors a{SomeThing{}};
    TestConstructors b{SomeThing{}};
    b = a; // <- copy assignment
}
#endif
}
// copy, no move, no default
{
using TestConstructors =
    detail::EitherConstructors
        <SomeError, SomeThing,
         detail::EitherDefaultConstructor::disable,
         detail::EitherCopyConstructor::enable,
         detail::EitherMoveConstructor::disable>;
#if 0
(void)TestConstructors{}; // <- default
#endif
(void)TestConstructors{SomeThing{}}; // <- right move
(void)TestConstructors{SomeError{}}; // <- left move
(void)TestConstructors{TypeTag<SomeError>{}, SomeThing{}}; // <- unambiguous left move
(void)TestConstructors{SomeError{}, TypeTag<SomeThing>{}}; // <- unambiguous right move
(void)TestConstructors{TestConstructors{SomeThing{}}}; // <- regular move
//#if 0
{
    TestConstructors a{SomeThing{}};
    TestConstructors{a}; // <- regular copy ctor
}
//#elif 0
{
    SomeThing a;
    TestConstructors{a}; // <- right copy ctor
}
//#elif 0
{
    SomeError a;
    TestConstructors{a}; // <- left copy ctor
}
//#elif 0
{
    SomeThing a;
    TestConstructors{TypeTag<SomeError>{}, a}; // <- unambiguous right copy ctor
}
//#elif 0
{
    SomeError a;
    TestConstructors{a, TypeTag<SomeThing>{}}; // <- unambiguous left copy ctor
}
//#endif
{
    TestConstructors a{SomeThing{}};
    TestConstructors b{SomeThing{}};
    b = std::move(a); // <- move assignment
}
{
    TestConstructors a{SomeThing{}};
    TestConstructors b{SomeThing{}};
    b = a; // <- copy assignment
}
}
// copy, no move, default
// copy, move, no default
// copy, move, default


VisisibleShit<std::unique_ptr<int>, int> a;
VisisibleShit<std::unique_ptr<int>, int> b;
a = std::move(b);
a = VisisibleShit<std::unique_ptr<int>, int>{std::in_place_index_t<0>{}, std::make_unique<int>(60)};
bool ccccc = a.is_left();
auto boomitsgone = a.left();

#if 1
describe("cul::either::right") ([] {
    using namespace either;
    mark_it("creates a right either", [] {
        return test_that(right<int>().with(SomeThing{}).is_right());
    }).
    mark_it("makes a gettable right", [] {
        right<int>()(SomeThing{}).right();
        return test_that(true);
    }).
    mark_it("preserves initial value of the right", [] {
        auto rv = right<SomeError>().with(int(10)).right();
        return test_that(rv == 10);
    });
    static_assert(either::right<char>().with(int(8)).right() == 8);
    static_assert
        (either::right<char>().with(int(8)).
         map([](int i) constexpr { return i*2; }).right() == 16);
    static_assert
        (either::right<char>().with(int(8)).
         map([](int i) constexpr { return i*2; }).
         map_left([] (char) constexpr { return 'a'; }).
         right() == 16);
    static_assert
        (either::right<char>().with(int(8)).
         map([](int i) constexpr { return i*2; }).
         fold<int>().
            map([](int i) constexpr { return i + 4; })() ==
         20);
    static_assert
        (either::right<char>().with(int(8)).
         map([](int i) constexpr { return i*2; }).
         map_left([] (char) constexpr { return 'a'; }).
         fold<int>().
            map([](int i) constexpr { return i + 4; }).
            map_left([] (char) { return 3; })() == 20);
});
#endif
#if 0
describe("cul::either::left") ([] {
    mark_it("creates a left either", [] {
        return test_that(either::left(SomeError{}).with<SomeThing>().is_left());
    }).
    mark_it("makes a gettable left", [] {
        either::left(SomeError{}).with<SomeThing>().left();
        return test_that(true);
    }).
    mark_it("preserves initial value of the left", [] {
        int rv = either::left(int(10)).with<SomeThing>().left();
        return test_that(rv == 10);
    });
    static_assert(either::left<char>('a').with<int>().left() == 'a');
    static_assert
        (either::left<char>('a').with<int>().
         map_left([](char c) constexpr { return char(c + 2); }).
         left() == 'c');
    static_assert
        (either::left<char>('a').with<int>().
         map([](int) constexpr { return 2; }).
         map_left([] (char c) constexpr { return char(c + 4); }).
         left() == 'e');
    static_assert
        (either::left<char>('a').with<int>().
         map_left([](char c) constexpr { return char(c + 4); }).
         fold<char>().
            map_left([](char c) constexpr { return char(c - 2); })() ==
         'c');
    static_assert
        (either::left<char>('a').with<int>().
         map([](int i) constexpr { return i*2; }).
         map_left([] (char) constexpr { return 'a'; }).
         fold<int>().
            map([](int i) constexpr { return i + 4; }).
            map_left([] (char) { return 3; })() == 3);

});

describe("Either{}") ([] {
    mark_it("disambiguates when both types are the same", [] {
        return test_that(
            Either<int, int>{TypeTag<int>{}, int(0)}.is_right() &&
            Either<char, char>{'h', TypeTag<char>{}}.is_left());
    }).
    mark_it("allows construction with one argument when types are distinct", [] {
        return test_that(
            Either<int, char>{int(0)}.is_left() &&
            Either<int, char>{'i'}.is_right());
    }).
    mark_it("throws if constructing from an empty optional either", [] {
        try {
            Either<int, char>{OptionalEither<int, char>{}};
        } catch (...) {
            return test_that(true);
        }
        return test_that(false);
    });
    // copy and move?
});

describe("(Optional)Either#map*") ([] {
    using namespace either;

    {
    auto a = Either<SomeError, SomeThing>{SomeThing{}}.
         map([] (SomeThing &&) { return int(1); });
    static_assert
        (std::is_same_v<decltype(a), Either<SomeError, int>>,
         "Either<Left, Right>#map (function returning Other) is Either<Left, Other>");
    }
    {
    auto a = Either<SomeError, SomeThing>{SomeThing{}}.
         map_left([] (SomeError &&) { return int(1); });
    static_assert
         (std::is_same_v<decltype(a), Either<int, SomeThing>>,
         "Either<Left, Right>#map_left (function returning Other) is "
         "Either<Other, Right>");
    }
    mark_it("changes right's to return value of the map function", [] {
        int gv = right<SomeError>().with(SomeThing{}).
            map([] (SomeThing) { return int(2); }).
            right();
        return test_that(gv == 2);
    }).
    mark_it("does not affect left's value when a right", [] {
        auto gv = left(int(10)).with<SomeThing>().
            map([] (SomeThing) { return int(2); }).
            left();
        return test_that(gv == 10);
    }).
    mark_it("changes left's to return value of the map function", [] {
        left(SomeError{}).with<SomeThing>();
        int gv = left(SomeError{}).with<SomeThing>().
            map_left([] (SomeError) { return int(2); }).
            left();
        return test_that(gv == 2);
    }).
    mark_it("does not affect right's value when a left", [] {
        auto gv = right<SomeThing>().with(int(10)).
            map_left([] (SomeThing) { return int(2); }).
            right();
        return test_that(gv == 10);
    });
});

describe("Either#chain") ([] {
    using namespace either;
    mark_it("chains a right returning a right either", [] {
        auto a = right<SomeError>().with(SomeThing{}).
            chain([] (SomeThing) { return right<SomeError>()(int(10)); }).
            right();
        return test_that(a == 10);
    }).
    mark_it("chains a right returning a left either", [] {
        auto a = right<int>().with(SomeThing{}).
            chain([] (SomeThing) { return left(int(10)).with<SomeThing>(); }).
            left();
        return test_that(a == 10);
    }).
    mark_it("chains a left returning a right either", [] {
        auto a = left(SomeError{}).with<int>().
            chain_left([] (SomeError) { return right<SomeThing>()(int(10)); }).
            right();
        return test_that(a == 10);
    }).
    mark_it("chains a left returning a left either", [] {
        auto a = left(SomeError{}).with<SomeThing>().
            chain_left([] (SomeError) { return left(int(10)).with<SomeThing>(); }).
            left();
        return test_that(a == 10);
    }).
    mark_it("handles conversion of return values", [] {
        right<SomeError>().with(SomeThing{}).
            chain([] (SomeThing) -> Either<SomeError, int> {
                bool something = false;
                if (something) return int(1);
                return SomeError{};
            }).
            left();
        return test_that(true);
    });
});
#endif
#if 1
describe("either::optional_left") ([] {
    using namespace either;
    mark_it("can create an empty either", [] {
        return test_that(optional_left<SomeError>().with<SomeThing>().is_empty());
    }).
    mark_it("creates a left either", [] {
        return test_that(optional_left(SomeThing{}).with<int>   ().is_left());
    }).
    mark_it("makes a gettable left", [] {
        optional_left<int>(1).with<SomeThing>().left();
        return test_that(true);
    }).
    mark_it("preserves initial value of the left", [] {
        auto rv = optional_left<int>(10).with<SomeThing>().left();
        return test_that(rv == 10);
    });
    static_assert(either::optional_left<char>().with<int>().is_empty());
    static_assert(either::optional_left<char>('a').with<int>().left() == 'a');
    static_assert
        (either::optional_left<char>('a').with<int>().
         map_left([](char c) constexpr { return char(c + 2); }).left() == 'c');
    static_assert
        (either::optional_left<int>(8).with<char>().
         map_left([](int i) constexpr { return i*2; }).
         map([] (char) constexpr { return 'a'; }).
         left() == 16);
    static_assert
        (either::optional_left<int>(8).with<char>().
         map_left([](int i) constexpr { return i*2; }).
         fold<int>(0).
            map_left([](int i) constexpr { return i + 4; })() ==
         20);
    static_assert
        (either::optional_left<int>(8).with<char>().
         map_left([](int i) constexpr { return i*2; }).
         map([] (char) constexpr { return 'a'; }).
         fold<int>(0).
            map_left([](int i) constexpr { return i + 4; }).
            map([] (char) { return 3; })() == 20);
    static_assert
        (either::optional_left<int>().with<char>().
         fold<int>(int(10)).
            map([] (char) constexpr { return int(0); }).
            map_left([] (int) constexpr { return int(1); })()
             == 10);
});
#endif
#if 0
describe("either::optional_right") ([] {
    using namespace either;
    mark_it("can create an empty either", [] {
        return test_that(optional_right<SomeError>().with<SomeThing>().is_empty());
    }).
    mark_it("creates a right either", [] {
        return test_that(optional_right<int>().with(SomeThing{}).is_right());
    }).
    mark_it("makes a gettable right", [] {
        optional_right<int>()(SomeThing{}).right();
        return test_that(true);
    }).
    mark_it("preserves initial value of the right", [] {
        auto rv = optional_right<SomeError>().with(int(10)).right();
        return test_that(rv == 10);
    });
    static_assert(either::optional_right<char>().with<int>().is_empty());
    static_assert(either::optional_right<char>().with(int(8)).right() == 8);
    static_assert
        (either::optional_right<char>().with(int(8)).
         map([](int i) constexpr { return i*2; }).right() == 16);
    static_assert
        (either::optional_right<char>().with(int(8)).
         map([](int i) constexpr { return i*2; }).
         map_left([] (char) constexpr { return 'a'; }).
         right() == 16);
    static_assert
        (either::optional_right<char>().with(int(8)).
         map([](int i) constexpr { return i*2; }).
         fold<int>(0).
            map([](int i) constexpr { return i + 4; })() ==
         20);
    static_assert
        (either::optional_right<char>().with(int(8)).
         map([](int i) constexpr { return i*2; }).
         map_left([] (char) constexpr { return 'a'; }).
         fold<int>(0).
            map([](int i) constexpr { return i + 4; }).
            map_left([] (char) { return 3; })() == 20);
    static_assert
        (either::optional_right<int>().with<char>().
         fold<int>(int(10)).
            map([] (char) constexpr { return int(0); }).
            map_left([] (int) constexpr { return int(1); })()
             == 10);
});

describe("OptionalEither::chain") ([] {
    using namespace either;
    mark_it("chains a right returning a right either", [] {
        auto a = optional_right<SomeError>().with(SomeThing{}).
            chain([] (SomeThing) { return optional_right<SomeError>()(int(10)); }).
            right();
        return test_that(a == 10);
    }).
    mark_it("chains a right returning a left either", [] {
        auto a = optional_right<int>().with(SomeThing{}).
            chain([] (SomeThing) { return optional_left(int(10)).with<SomeThing>(); }).
            left();
        return test_that(a == 10);
    }).
    mark_it("does not chains on an empty", [] {
        int i = 0;
        (void)optional_right<int>().with<SomeThing>().
            chain([&i] (SomeThing &&) { ++i; return optional_left<int>().with<char>(); });
        return test_that(i == 0);
    }).
    mark_it("handles conversion of return values", [] {
        optional_right<SomeError>().with(SomeThing{}).
            chain([] (SomeThing) -> OptionalEither<SomeError, int> {
                bool something = false;
                if (something) return int(1);
                return SomeError{};
            }).
            left();
        return test_that(true);
    });;
});

describe("OptionalEither::chain_left") ([] {
    using namespace either;
    mark_it("chains a left returning a right either", [] {
        auto a = optional_left(SomeError{}).with<int>().
            chain_left([] (SomeError) { return optional_right<int>()(int(10)); }).
            right();
        return test_that(a == 10);
    }).
    mark_it("chains a left returning a left either", [] {
        auto a = optional_left(SomeThing{}).with<int>().
            chain_left([] (SomeThing) { return optional_left(int(10)).with<int>(); }).
            left();
        return test_that(a == 10);
    }).
    mark_it("does not chains on an empty", [] {
        int i = 0;
        (void)optional_right<int>().with<SomeThing>().
            chain_left([&i] (int) { ++i; return optional_left<int>().with<SomeThing>(); });
        return test_that(i == 0);
    });
});

describe("Either#operator=/copying") ([] {
    mark_it("move constructs", [] {
        auto ei = either::right<std::unique_ptr<int>>().with<int>(10);
        return test_that(Either{std::move(ei)}.is_right());
    }).
    mark_it("copy constructs", [] {
        auto ei = either::right<char>().with<int>(19);
        Either new_ei{ei};
        return test_that(ei.right() == new_ei.right());
    }).
    mark_it("copy assigns", [] {
        Either<char, int> new_ei{'a'};
        auto ei = either::right<char>().with<int>(19);
        new_ei = ei;
        return test_that(ei.right() == new_ei.right());
    }).
    mark_it("move assigns", [] {
        Either<char, int> new_ei{'a'};
        auto ei = either::right<char>().with<int>(19);
        new_ei = std::move(ei);
        return test_that(new_ei.right() == 19);
    }).
    mark_it("move assigned becomes 'consumed'", [] {
        Either<char, int> new_ei{'a'};
        auto ei = either::right<char>().with<int>(19);
        new_ei = std::move(ei);
        return expect_exception<std::runtime_error>([&ei] {
            ei.right();
        });
    });
});

describe("OptionalEither#operator=/copying") ([] {
    mark_it("move constructs", [] {
        auto ei = either::optional_right<std::unique_ptr<int>>().with<int>(10);
        return test_that(OptionalEither{std::move(ei)}.is_right());
    }).
    mark_it("copy constructs", [] {
        auto ei = either::optional_right<char>().with<int>(19);
        OptionalEither new_ei{ei};
        return test_that(ei.right() == new_ei.right());
    }).
    mark_it("copy assigns", [] {
        OptionalEither<char, int> new_ei{'a'};
        auto ei = either::optional_right<char>().with<int>(19);
        new_ei = ei;
        return test_that(ei.right() == new_ei.right());
    }).
    mark_it("move assigns", [] {
        OptionalEither<char, int> new_ei{'a'};
        auto ei = either::optional_right<char>().with<int>(19);
        new_ei = std::move(ei);
        return test_that(new_ei.right() == 19);
    }).
    mark_it("move assigned becomes 'consumed'", [] {
        OptionalEither<char, int> new_ei{'a'};
        auto ei = either::optional_right<char>().with<int>(19);
        new_ei = std::move(ei);
        return expect_exception<std::runtime_error>([&ei] {
            ei.right();
        });
    });
});

describe("Either is 'consumed'") ([] {
    mark_it("is consumed on call to left", [] {
        auto ei = either::left(SomeError{}).with<SomeThing>();
        (void)ei.left();
        return expect_exception<std::runtime_error>([&ei] {
            (void)ei.left();
        });
    }).
    mark_it("is consumed on call to right", [] {
        auto ei = either::right<SomeError>().with(SomeThing{});
        (void)ei.right();
        return expect_exception<std::runtime_error>([&ei] {
            (void)ei.right();
        });
    }).
    mark_it("is consumed on call to fold", [] {
        auto ei = either::right<SomeError>().with(SomeThing{});
        (void)ei.fold<int>();
        return expect_exception<std::runtime_error>([&ei] {
            (void)ei.fold<int>().map([](SomeThing) { return 0; });
        });
    }).
    mark_it("is consumed on call to map", [] {
        auto ei = either::right<SomeError>().with(SomeThing{});
        (void)ei.right();
        return expect_exception<std::runtime_error>([&ei] {
            (void)ei.map([] (SomeThing) { return SomeThing{}; });
        });
    });
    mark_it("is consumed on call to chain", [] {
        auto ei = either::right<SomeError>().with(SomeThing{});
        (void)ei.right();
        return expect_exception<std::runtime_error>([&ei] {
            (void)ei.chain([] (SomeThing)
            {
                return either::right<SomeError>().with(SomeThing{});
            });
        });
    });
    mark_it("is consumed on call to map_left", [] {
        auto ei = either::right<SomeError>().with(SomeThing{});
        (void)ei.right();
        return expect_exception<std::runtime_error>([&ei] {
            (void)ei.map_left([] (SomeError) { return SomeThing{}; });
        });
    });
    mark_it("is consumed on call to chain_left", [] {
        auto ei = either::right<SomeError>().with(SomeThing{});
        (void)ei.right();
        return expect_exception<std::runtime_error>([&ei] {
            (void)ei.chain_left([] (SomeError)
            {
                return either::right<SomeError>().with(SomeThing{});
            });
        });
    });
});

describe("unique_ptr compatible") ([] {
    using IntUPtr = std::unique_ptr<int>;
    mark_it("can do right things with a unique pointer", [] {
        int gv = either::right<SomeError>().
            with(std::make_unique<int>(10)).
            map([] (IntUPtr && uptr) {
                *uptr *= 2;
                return std::move(uptr);
            }).
            chain([] (IntUPtr && uptr)
            {
                if (uptr) return either::right<SomeError>().with(std::move(uptr));
                return either::left<SomeError>(SomeError{}).with<IntUPtr>();
            }).fold<int>().
                map([] (IntUPtr && uptr) { return *uptr; }).
                map_left([] (SomeError) { return 0; })();
        return test_that(gv == 20);
    }).
    mark_it("can do left things with a unique pointer", [] {
        int gv = either::left<IntUPtr>(std::make_unique<int>(10)).
            with<SomeThing>().
            map_left([] (IntUPtr && uptr) {
                *uptr *= 2;
                return std::move(uptr);
            }).
            chain_left([] (IntUPtr && uptr)
            {
                if (uptr) return either::left<IntUPtr>(std::move(uptr)).with<SomeThing>();
                return either::right<IntUPtr>().with(SomeThing{});
            }).fold<int>().
                map([] (SomeThing) { return 0; }).
                map_left([] (IntUPtr && uptr) { return *uptr; })();
        return test_that(gv == 20);
    }).
    mark_it("is storable in vector with unique pointers", [] {
        std::vector<Either<SomeError, std::unique_ptr<int>>> vec;
        vec.emplace_back(std::make_unique<int>(10));
        return test_that(true);
    });
});

//describe("")([] {


    static_assert
        (std::is_same_v<decltype(Either<SomeError, SomeThing>{SomeThing{}}.
         fold<int>()),
         either::Fold<SomeError, SomeThing, int>>,
         "Either#fold returns a Either::Fold type");
    static_assert
        (std::is_same_v<decltype(Either<SomeError, SomeThing>{SomeThing{}}.
         fold<int>()()),
         int>,
         "Either#fold#operator () returns the common type");
    {
    auto a = Either<SomeError, SomeThing>{SomeThing{}}.
         fold<int>().map([] (SomeThing &&) constexpr { return int(0); });
    static_assert
        (std::is_same_v<decltype(a),
         either::Fold<SomeError, SomeThing, int>>,
         "Either#fold#map returns appropriate Fold type");
    }
    {
    auto a = Either<SomeError, SomeThing>{SomeThing{}}.
         fold<int>().map_left([] (SomeError &&) { return int(0); });
    static_assert
        (std::is_same_v<decltype(a),
         either::Fold<SomeError, SomeThing, int>>,
         "Either#fold#map_left returns appropriate Fold type");
    }
//});
#endif
return [] {};

} ();

int main() { return cul::tree_ts::run_tests(); }
