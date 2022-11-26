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
    void enforce_step(int i) {
        if (m_step + 1 >= i) return;
        if (i != m_step) {
            throw RtError{"execution out of order"};
        }
        ++m_step;
    }
private:
    int m_step;
};

int main() {
    using namespace cul::tree_ts;
    static ExecutionOrderEnforcer order;
    describe<BUsesA>("B handles something").depends_on<A>()([] {
        order.enforce_step(3);
        it("does something", [] {
            order.enforce_step(4);
            return test_that(true);
        });
        it("does something else", [] {
            order.enforce_step(5);
            return test_that(true);
        });
    });
    describe<A>("A does something else")([] {
        order.enforce_step(0);
        it("does something", [] {
            order.enforce_step(1);
            return test_that(true);
        });
        it("does something else", [] {
            order.enforce_step(2);
            return test_that(true);
        });
    });
    run_tests();
    return 0;
}
