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
#include <ariajanke/cul/Util.hpp>

#include <cassert>

using namespace cul::exceptions_abbr;
using namespace cul::tree_ts;

class A final {};
class C final {};
class B final {};
class D final {};

class BUsesA final {};
class DUsesC final {};

class ExecutionOrderEnforcer final {
public:
    void within_steps(int at_least_step, int at_most_step) {
        if (m_step >= at_least_step && m_step <= at_most_step)
            return;
        throw RtError{""};
    }

    void enforce_step(int expected_step) {
        if (expected_step != m_step) {
            throw RtError{"execution out of order"};
        }
        ++m_step;
    }
private:
    int m_step = 0;
};

class HitAtMost final {
public:
    void hit_at_most(int n) {
        assert(m_hits <= n);
        ++m_hits;
    }

private:
    int m_hits = 0;
};

#define mark_it mark_source_position(__LINE__, __FILE__).it

class Dummy final {};

[[maybe_unused]] static Dummy dummy = [] {

describe("Next works as intended")([] {
    ExecutionOrderEnforcer eoe;
    int i = 0;
    mark_it("test one", [&] {
        eoe.enforce_step(0);
        return test_that(++i == 1);
    }).next([&] {
        static HitAtMost ham;
        eoe.enforce_step(0);
        eoe.within_steps(1, 4);
        ham.hit_at_most(2);
        ++i;
    }).mark_it("test two", [&] {
        eoe.enforce_step(1);
        return test_that(++i == 2);
    }).next([&] {
        static HitAtMost ham;
        eoe.enforce_step(1);
        eoe.within_steps(2, 4);
        ham.hit_at_most(1);
        ++i;
    }).mark_it("test three", [&] {
        eoe.enforce_step(2);
        return test_that(++i == 3);
    });
});

describe("Expect throw works as intended")([] {
    mark_it("test one", [&] {
        class Exp final {};
        return expect_exception<Exp>([] {
            throw Exp{};
        });
    });
});

return Dummy{};

} ();
