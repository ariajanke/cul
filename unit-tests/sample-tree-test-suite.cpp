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

#include <ariajanke/cul/TreeTestSuite.hpp>
#include <ariajanke/cul/Util.hpp>

using namespace cul::exceptions_abbr;

class A final {

};

class BUsesA final {

};

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

int main() {
    using namespace cul::tree_ts;
    using std::cout, std::endl;
    static ExecutionOrderEnforcer order;
    auto & inst = TreeTestSuite::instance();
    describe<BUsesA>("B handles something").depends_on<A>()([] {
        cout << "B handles something" << endl;
        order.within_steps(2, 3);
        it("does something", [] {
            //cout << "does something" << endl;
            order.enforce_step(2);
            return test_that(true);
        });
        it("does something else", [] {
            //cout << "does something else" << endl;
            order.enforce_step(3);
            return test_that(true);
        });
    });
    describe<A>("A does something else")([] {
        cout << "A does something else" << endl;
        order.within_steps(0, 1);
        it("does something", [] {
            //cout << "does something" << endl;
            order.enforce_step(0);
            return test_that(true);
        });
        it("does something else", [] {
            //cout << "does something else" << endl;
            order.enforce_step(1);
            return test_that(true);
        });
    });
    describe<A>("A in another describe block")([] {
        //cout << "does something unexpected" << endl;
        it("does something unexpected", [] {
            return test_that(true);
        });
    });
    // I want to call describe without a type
    return run_tests();
}
