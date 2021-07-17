/****************************************************************************

    MIT License

    Copyright (c) 2021 Aria Janke

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

#include <common/TestSuite.hpp>
#include <common/Util.hpp>

#include <iostream>
#include <algorithm>

#include <cmath>
#include <cstring>

namespace {

using namespace cul::exceptions_abbr;

std::string to_padded_string(int);

std::string source_position_to_string(const char * filename, int line);

} // end of <anonymous> namespace

namespace cul {

namespace ts {

void set_context(TestSuite & suite, void (*make_context)(TestSuite &, Unit &)) {
    Unit unit = UnitAttn::make_unit();

    // caller must call "start" on unit, or the loop exits
    // does result in a "wasted" iteration
    make_context(suite, unit);

    while (UnitAttn::index_hit(unit)) {
        UnitAttn::increment(unit);
        make_context(suite, unit);
    }
}

/* private static */ void UnitAttn::increment(Unit & unit) {
    unit.m_hit = false;
    ++unit.m_index;
    unit.m_starts = 0;
}

// ----------------------------------------------------------------------------

TestAssertion test(bool v) {
   TestAssertion ta;
   ta.value = v;
   return ta;
}

TestSuite::TestSuite(): m_out(&std::cout) {}

TestSuite::TestSuite(const char * series_name):
    TestSuite()
{
    start_series(series_name);
}

TestSuite::~TestSuite()
    { finish_up(); }

void TestSuite::start_series(const char * desc) {
    finish_up();
    *m_out << desc << std::endl;
    m_test_count = 0;
}
#if 0
void TestSuite::test(TestAssertion (*test_func)()) {
    do_test_back([test_func]() {
        return test_func().value;
    });
}

void TestSuite::test(TestAssertion (*test_func)(TestFunc)) {
    do_test_back([test_func]() {
        return test_func(ts::test).value;
    });
}
#endif
void TestSuite::assign_output_stream(std::ostream & out) { m_out = &out; }

void TestSuite::mark_source_position(const char * filename, int line) {
    if (!filename || line < 0) {
        throw InvArg("TestSuite::mark_source_position: Source filename must "
                     "be a non-null pointer and line must be a non-negative "
                     "integer.");
    }
    m_source_position = line;
    m_source_file     = filename;
}

void TestSuite::unmark_source_position() {
    // is enough to revert behavior
    m_source_file = nullptr;
}

void TestSuite::finish_up() noexcept {
    if (m_test_count == 0) return;
    *m_out << "[ Passed " << m_test_successes << " / " << m_test_count
           << " test cases (" << std::to_string(int(double(m_test_successes)/double(m_test_count)*100.))
           << "%) ]" << std::endl;
    m_test_count = m_test_successes = 0;
}

bool TestSuite::has_successes_only() const
    { return m_test_successes == m_test_count; }

/* private */ void TestSuite::print_failure(const char * exception_text) {
    *m_out << "[ FAILED TEST " << to_padded_string(m_test_count) << " ]";
    if (m_source_file) {
        *m_out << "\nTest location: " << source_position_to_string(m_source_file, m_source_position);
    }
    if (exception_text) {
        *m_out << "\nTest threw an exception with the following description:\n"
               << exception_text;
    }
    *m_out << std::endl;
}

/* private */ void TestSuite::print_success() {
    *m_out << "[ Passed TEST " << to_padded_string(m_test_count) << " ]"
           << std::endl;
}

} // end of ts namespace -> into ::cul

} // end of cul namespace

namespace {

std::string to_padded_string(int x) {
    // what if I have a 1000 test cases?
    if (x == 0) return "   ";
    std::string rv = std::to_string(x);
    return std::string(std::size_t(2 - std::floor(std::log10(double(x)))), ' ') + rv;
}

std::string source_position_to_string(const char * filename, int line) {
    // should hopefully be good enough for my needs
    using std::make_reverse_iterator;
    auto fn_end = filename + ::strlen(filename);
    const auto k_fn_rev_end = make_reverse_iterator(filename);
    auto find_last = [k_fn_rev_end, fn_end](char c)
        { return std::find(make_reverse_iterator(fn_end), k_fn_rev_end, c); };
    static auto revert = [](decltype(k_fn_rev_end) ritr) { return (ritr.base()) - 1; };
    auto fslash = find_last('/' );
    auto bslash = find_last('\\');
    if (fslash != k_fn_rev_end && bslash != k_fn_rev_end) {
        filename = revert(std::max(fslash, bslash));
    } else if (fslash != k_fn_rev_end) {
        filename = revert(fslash);
    } else if (bslash != k_fn_rev_end) {
        filename = revert(bslash);
    }

    return std::string(filename) + " line " + std::to_string(line);
}

} // end of <anonymous> namespace
