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
#include <ariajanke/cul/Either.hpp>

namespace {

using namespace cul;
using namespace tree_ts;

#define mark_it mark_source_position(__LINE__, __FILE__).it

class SomeError final {};
class SomeThing final {};

}

auto x = [] {

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
    });
});
#if 0 // already done?
describe("Either#fold") ([] {
    mark_it()
});
#endif
describe("either::optional_left") ([] {


});

describe("either::optional_right") ([] {
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
    // optional specifics
    static_assert
        (either::optional_right<int>().with<char>().
         fold<int>(int(10)).
            map([] (char) constexpr { return int(0); }).
            map_left([] (int) constexpr { return int(1); })()
             == 10);
});
#if 0 // tested too
describe("OptionalEither{}") ([] {});
#endif
describe("OptionalEither::chain") ([] {});

describe("OptionalEither::chain_left") ([] {});

describe("the 'consumed' either") ([] {});

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

return [] {};

} ();

int main() { return cul::tree_ts::run_tests(); }
