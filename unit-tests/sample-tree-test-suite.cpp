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
#include <ariajanke/cul/FunctionTraits.hpp>
#include <ariajanke/cul/Either.hpp>

#include <variant>
#include <optional>

#include <cassert>

using namespace cul::exceptions_abbr;
using cul::Either;

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

void tests_execute_in_dependant_order();
void multiple_describes_work();
void failing_dependee_does_not_run_depender();
void depender_without_dependee_never_runs();
void typeless_describes();
void depends_on_type_mismatch_throws();
void it_cases_surpress_exceptions();
void it_handles_failures_correctly();
void it_does_not_stop_with_failed_describe();

// empty describes should not crash/break anything
void it_throws_on_empty_describes();

#define mark_it mark_source_position(__LINE__, __FILE__).it

int main() {
    assert(cul::tree_ts::run_tests() == 0);

    // the test program should freaking crash if any test fails
    tests_execute_in_dependant_order();
    multiple_describes_work();
    failing_dependee_does_not_run_depender();
    depender_without_dependee_never_runs();
    typeless_describes();
    depends_on_type_mismatch_throws();
    it_cases_surpress_exceptions();
    it_handles_failures_correctly();
    it_does_not_stop_with_failed_describe();
    it_throws_on_empty_describes();
    return 0;
}

void tests_execute_in_dependant_order() {
    using namespace cul::tree_ts;
    static ExecutionOrderEnforcer order;

    describe<BUsesA>("B handles something").depends_on<A>()([] {
        order.within_steps(1, 2);
        mark_it("does something", [] {
            order.enforce_step(1);
            return test_that(true);
        });
        mark_it("does something else", [] {
            order.enforce_step(2);
            return test_that(true);
        });
    });
    describe<A>("A in another describe block")([] {
        mark_it("does something unexpected", [] {
            order.enforce_step(0);
            return test_that(true);
        });
    });
    run_tests();
}

void multiple_describes_work() {
    using namespace cul::tree_ts;
    static ExecutionOrderEnforcer order;

    describe<A>("A does something else")([] {
        order.within_steps(0, 1);
        mark_it("does something", [] {
            order.enforce_step(0);
            return test_that(true);
        });
        mark_it("does something else", [] {
            order.enforce_step(1);
            return test_that(true);
        });
    });
    describe<A>("A in another describe block")([] {
        mark_it("does something unexpected", [] {
            return test_that(true);
        });
    });
    run_tests();
}

void failing_dependee_does_not_run_depender() {
    using namespace cul::tree_ts;
    describe<C>("has a failing test")([] {
        mark_it("fails a test", [] {
            return test_that(false);
        });
    });
    describe<DUsesC>("should not run at all").depends_on<C>()([] {
        // no worries, right?
        assert(false);
    });
    run_tests();
}

void depender_without_dependee_never_runs() {
    using namespace cul::tree_ts;
    describe<DUsesC>("should not run at all").depends_on<C>()([] {
        // no worries, right?
        assert(false);
    });
    run_tests();
}

void typeless_describes() {
    using namespace cul::tree_ts;
    static int hits = 0;
    // I want to call describe without a type
    describe("simple utility")([] {
        mark_it("does something useful", [] {
            ++hits;
            return test_that(true);
        });
    });

    describe("another utility")([] {
        mark_it("does something useful", [] {
            ++hits;
            return test_that(true);
        });
    });
    run_tests();
    assert(hits == 2);
}

void depends_on_type_mismatch_throws() {
    using namespace cul::tree_ts;
    try {
        describe<A>("A in another describe block").depends_on<D>()([] {
            assert(false);
            mark_it("does something unexpected", [] {
                return test_that(true);
            });
        });
        describe<A>("A in another describe block").depends_on<B>()([] {
            assert(false);
            mark_it("does something unexpected", [] {
                return test_that(true);
            });
        });
        run_tests();
    } catch (...) {
        return;
    }
    assert(false);
}

void it_cases_surpress_exceptions() {
    using namespace cul::tree_ts;
    static int hits = 0;
    describe<A>("A describe with a throwing test")([] {
        mark_it("this failing testing, because an exception was thrown", [] {
            throw RtError{"exception text"};
            return test_that(true);
        });
        mark_it("but this test case passes", [] {
            ++hits;
            return test_that(true);
        });
    });
    run_tests();
    assert(hits == 1);
}

void it_handles_failures_correctly() {
    using namespace cul::tree_ts;
    describe("a")([] {
        mark_it("has a passing test case", [] {
            return test_that(true);
        });
    });
    describe("z")([] {
        mark_it("has a failing test case", [] {
            return test_that(false);
        });
    });
    assert(run_tests() != 0);
}

void it_does_not_stop_with_failed_describe() {
    using namespace cul::tree_ts;
    static int count = 0;
    describe<A>("a")([] {
        ++count;
        mark_it("has a failing test case", [] {
            return test_that(false);
        });
    });
    describe<B>("b")([] {
        ++count;
        mark_it("has a failing test case", [] {
            return test_that(false);
        });
    });
    run_tests();
    assert(count == 2);
}

void it_throws_on_empty_describes() {
    using namespace cul::tree_ts;
    describe("empty describe")([] {});
    assert(run_tests() != 0);
}
