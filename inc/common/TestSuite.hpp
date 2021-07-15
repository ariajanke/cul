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

#pragma once

#include <iosfwd>

namespace cul {

namespace ts {

class TestAssertion;
class TestSuite;

TestAssertion test(bool);

/** Performs a unit test, marking the source's current position and filename.
 *
 *  @note If it were possible to avoid macros here... I would happily do so.
 *        Maybe in 2023 I guess...
 *        It's also not even supported in Clang yet!
 */
#define MACRO_CUL_TEST(ts,x) do{\
    (ts).mark_source_position(__FILE__,__LINE__); /* < why I need a macro */ \
    (ts).test((x));\
}while(false)

using TestFunc = TestAssertion(*)(bool);

// semantic object
class TestAssertion {
    friend TestAssertion test(bool);
    friend class TestSuite;
    bool value;
};

/** A "home grown" unit test class.
 *
 *  Tests are done a "series" at time. Each series is named and when they're
 *  finished a small summary is printed.
 *
 *  By default std::cout is used as the output stream. Which can now be
 *  changed.
 *
 *  @note limitation: cannot test for segments that are supposed to fail to
 *        compile.
 */
class TestSuite {
public:
    /** Constructs with "no series" started, but with zerod out counters. */
    TestSuite();

    /** Constructs with a series name (calling start_series) */
    explicit TestSuite(const char * series_name);
    
    TestSuite(const TestSuite &) = delete;
    TestSuite(TestSuite &&) = delete;
    
    /** calls finish_up() on destruction */
    ~TestSuite();
    
    TestSuite & operator = (const TestSuite &) = delete;
    TestSuite & operator = (TestSuite &&) = delete;
    
    /** Prints the given string and resets the success/failure counters. */
    void start_series(const char *);

    /** Performs a test, which returns an assertion.
     *
     *  This maybe called using a lambda expression that is implicitly
     *  convertible to a function pointer.
     *  @param test_func function to call for testing
     *  @see MACRO_CUL_TEST
     */
    void test(TestAssertion (*test_func)());

    [[deprecated]] void test(TestAssertion (*test_func)(TestFunc));

    /** Assigns a stream to write to.
     *  @warning like all "assign_*" functions, this sets an unmanaged
     *           pointer, so the stream must survive in this case *longer*
     *           than this object.
     */
    void assign_output_stream(std::ostream &);

    /** Made to mark the current position in the current source file where
     *  the test is being issued the "MACRO_CUL_TEST" is meant to automate
     *  the call to this function.
     *  @param filename name of source file
     *  @param line line number in the source file
     *  @throws if filename is nullptr or line is a negative integer
     */
    void mark_source_position(const char * filename, int line);

    /** Returns the recorded source position to "none" like this object was
     *  freshly constructed.
     */
    void unmark_source_position();

    /** "Ends" the "series" by printing a one line summary.
     *  @note this function is called automatically by the destructor
     */
    void finish_up() noexcept;

    /** @returns true if all test cases in the current are up to this point
     *           all been successful, false otherwise
     */
    bool has_successes_only() const;

    /** Causes successful test cases to not print out anything. */
    void hide_successes() { m_silence_success = true; }

    /** Causes successful test cases to print out that it passed. */
    void show_successes() { m_silence_success = false; }

private:
    template <typename Func>
    void do_test_back(Func &&);

    void print_failure(const char * exception_text = nullptr);

    int m_test_count = 0;
    int m_test_successes = 0;
    bool m_silence_success = false;

    int m_source_position = 0;
    const char * m_source_file = nullptr;

    std::ostream * m_out;
};

} // end of ts namespace -> into ::cul

} // end of cul namespace
