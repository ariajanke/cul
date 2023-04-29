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

describe("cul::make_right") ([] {
    mark_it("creates a right either", [] {
        return test_that(make_right<int>(SomeThing{}).is_right());
    }).
    mark_it("makes a gettable right", [] {
        make_right<SomeError>(SomeThing{}).right();
        return test_that(true);
    }).
    mark_it("preserves initial value of the right", [] {
        auto rv = make_right<SomeError>(int(10)).right();
        return test_that(rv == 10);
    });
});

describe("cul::make_left") ([] {
    mark_it("creates a left either", [] {
        return test_that(make_left<SomeThing>(SomeError{}).is_left());
    });
});

describe("Either#map*") ([] {
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
        int gv = make_right<SomeError>(SomeThing{}).
            map([] (SomeThing) { return int(2); }).
            right();
        return test_that(gv == 2);
    }).
    mark_it("does not affect left's value when a right", [] {
        auto gv = make_left<SomeThing>(int(10)).
            map([] (SomeThing) { return int(2); }).
            left();
        return test_that(gv == 10);
    }).
    mark_it("changes left's to return value of the map function", [] {
        int gv = make_left<SomeThing>(SomeError{}).
            map_left([] (SomeError) { return int(2); }).
            left();
        return test_that(gv == 2);
    }).
    mark_it("does not affect right's value when a left", [] {
        auto gv = make_right<SomeThing>(int(10)).
            map_left([] (SomeThing) { return int(2); }).
            right();
        return test_that(gv == 10);
    });
});

describe("Either#chain") ([] {
    mark_it("chains a right returning a right either", [] {
        auto a = make_right<SomeError>(SomeThing{}).
            chain([] (SomeThing) { return make_right<SomeError>(int(10)); }).
            right();
        return test_that(a == 10);
    }).
    mark_it("chains a right returning a left either", [] {
        auto a = make_right<int>(SomeThing{}).
            chain([] (SomeThing) { return make_left<SomeError>(int(10)); }).
            left();
        return test_that(a == 10);
    }).
    mark_it("chains a left returning a right either", [] {
        auto a = make_left<SomeThing>(SomeError{}).
            chain_left([] (SomeError) { return make_right<SomeThing>(int(10)); }).
            right();
        return test_that(a == 10);
    }).
    mark_it("chains a left returning a left either", [] {
        auto a = make_left<SomeThing>(SomeError{}).
            chain_left([] (SomeError) { return make_left<SomeError>(int(10)); }).
            left();
        return test_that(a == 10);
    });
});

describe("")([] {


    static_assert
        (std::is_same_v<decltype(Either<SomeError, SomeThing>{SomeThing{}}.
         fold<int>()),
         Either<SomeError, SomeThing>::Fold<int>>,
         "Either#fold returns a Either::Fold type");
    static_assert
        (std::is_same_v<decltype(Either<SomeError, SomeThing>{SomeThing{}}.
         fold<int>()()),
         int>,
         "Either#fold#operator () returns the common type");
    {
    auto a = Either<SomeError, SomeThing>{SomeThing{}}.
         fold<int>().map([] (SomeThing &&) { return int(0); });
    static_assert
        (std::is_same_v<decltype(a),
         Either<SomeError, SomeThing>::Fold<int>>,
         "Either#fold#map returns appropriate Fold type");
    }
    {
    auto a = Either<SomeError, SomeThing>{SomeThing{}}.
         fold<int>().map_left([] (SomeError &&) { return int(0); });
    static_assert
        (std::is_same_v<decltype(a),
         Either<SomeError, SomeThing>::Fold<int>>,
         "Either#fold#map_left returns appropriate Fold type");
    }
    int i = 0;
    auto rv = make_right<SomeError>(SomeThing{}).
        map([&i] (SomeThing &&) { ++i; return 0; }).
        fold<int>().
        map([&i] (int i_) { return ++i; i_; }).
        map_left([] (SomeError &&) { return 10; })();
});

return [] {};

} ();
