/****************************************************************************

    MIT License

    Copyright (c) 2020 Aria Janke

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

#include <ariajanke/cul/StringUtil.hpp>
#include <ariajanke/cul/TestSuite.hpp>
#include <ariajanke/cul/TreeTestSuite.hpp>

#include <cstring>
#include <cassert>

#define mark_it mark_source_position(__LINE__, __FILE__).it

namespace {

// using namespace cul;
void add_for_split_tests();
void add_string_to_number_tests();
void add_trim_tests();
void add_wrap_tests();

} // end of <anonymous> namespace

int main() {
    using namespace cul::tree_ts;
    // here's a problem: testing template classes...
    describe<cul::View<std::string::const_iterator>>("View class")([] {
        mark_it("can loop between two given iterators", [] {
            std::string a = "hello";
            cul::View b{ a.cbegin(), a.cend() };
            std::string c;
            for (char c_ : b) {
                c += (c_ - 'a') + 'A';
            }
            return test_that(c == "HELLO");
        });
    });

    add_for_split_tests();
    add_string_to_number_tests();
    add_trim_tests();
    add_wrap_tests();

    return run_tests();
}

namespace {

inline bool is_whitespace(char c) { return c == ' ' || c == '\t' || c == '\r' || c == '\n'; }
inline bool is_comma(char c) { return c == ','; }
inline bool is_whitespace_u(std::u32string::value_type c) { return is_whitespace(char(c)); }

void add_for_split_tests() {
    using namespace cul::tree_ts;
    using namespace cul;
    using Iter = std::string::iterator;
    using ConstIter = std::string::const_iterator;
    describe("for_split string helper")([] {
        mark_it("splits single character seperated values", [] {
            int count = 0;
            std::string samp = "a b c";

            for_split<is_whitespace>(samp.begin(), samp.end(),
                [&count](Iter, Iter)
            { ++count; });
            return test_that(count == 3);
        });
        mark_it("split strings with the correct lengths", [] {
            int count = 0;
            std::string samp = "a b c";
            auto beg = &samp[0];
            auto end = beg + samp.length();
            for_split<is_whitespace>(beg, end,
                [&count](const char * beg, const char * end)
            { count += (end - beg); });
            return test_that(count == 3);
        });
        mark_it("responds to 'flow control signals'", [] {
            int count = 0;
            std::string samp = "a b c e f";

            for_split<is_whitespace>(samp.begin(), samp.end(),
                [&count](Iter, Iter)
            {
                ++count;
                return (count == 3) ? fc_signal::k_break : fc_signal::k_continue;
            });
            return test_that(count == 3);
        });
        mark_it("treats multiple split characters as one", [] {
            int count = 0;
            std::string samp = " a b c  e    f           ";
            for_split<is_whitespace>(samp, [&count](ConstIter, ConstIter)
                { ++count; });
            return test_that(count == 5);
        });
        mark_it("does not call predicate once, with strings that are just "
                "split characters", []
        {
            int count = 0;
            std::string samp = ",,,";
            for_split<is_comma>(samp.begin(), samp.end(), [&](ConstIter, ConstIter)
                { ++count; });
            return test_that(count == 0);
        });
    });
}

void add_string_to_number_tests() {
    using namespace cul::tree_ts;
    using cul::string_to_number_assume_negative, cul::string_to_number,
          cul::magnitude, cul::string_to_number_multibase;
    describe("string_to_number helper method")([] {
        mark_it("converts three digits to a negative number", [] {
            const char * str = "856";
            int out = 0;
            bool res = string_to_number_assume_negative(str, str + strlen(str), out);
            return test_that(res && out == -856);
        });
        mark_it("converts to positive floating point, accurate to two places", [] {
            const char * str = "123.34";
            float out = 0.f;
            bool res = string_to_number_assume_negative(str, str + strlen(str), out);
            return test_that(res && magnitude(out + 123.34) < 0.005f);
        });
        mark_it("converts unsigned types, while assuming negative, still "
                "produces a valid integer", []
        {
            std::string samp = "5786";
            std::size_t out = 0;
            bool res = string_to_number_assume_negative(samp.begin(), samp.end(), out);
            // ??? what?
            return test_that(res && out == 5786);
        });
        mark_it("correctly converts '0' to zero", [] {
            std::string samp = "0";
            int out = -1;
            bool res = string_to_number(samp.begin(), samp.end(), out);
            return test_that(res && out == 0);
        });
        mark_it("converts three character string to correct positive number", [] {
            std::string samp = "123";
            int out = 0;
            bool res = string_to_number(samp, out);
            return test_that(res && out == 123);
        });
        mark_it("converts the string representation of the min integer correctly", [] {
            std::string samp = "-2147483648";
            int32_t out = 0;
            bool res = string_to_number(samp, out);
            return test_that(res && out == -2147483648);
        });
        mark_it("converts to binary numbers correctly", [] {
            std::string samp = "-101001";
            int out = 0;
            bool res = string_to_number(samp, out, 2);
            return test_that(res && out == -0b101001);
        });
        mark_it("converts u32 numeric strings correctly", [] {
            std::u32string wide = U"-9087";
            int out = 0;
            bool res = string_to_number(wide, out);
            return test_that(res && out == -9087);
        });
        mark_it("converts to integer rounding up from n.5", [] {
            std::string samp = "10.5";
            int out = 0;
            bool res = string_to_number(samp, out);
            return test_that(res && out == 11);
        });
        mark_it("converts to integer rounding down from numbers less than n.5", [] {
            std::string samp = "10.4";
            int out = 0;
            bool res = string_to_number(samp, out);
            return test_that(res && out == 10);
        });
    });
    describe("string_to_number_multibase utility method")([] {
        mark_it("converts to octal numbers correctly", [] {
            std::string samp = "0o675";
            int out = 0;
            bool res = string_to_number_multibase(samp.begin(), samp.end(), out);
            return test_that(res && out == 0675);
        });
        mark_it("converts non-prefixed strings as decimal numbers", [] {
            std::string samp = "7995";
            int out = 0;
            bool res = string_to_number_multibase(samp, out);
            return test_that(res && out == 7995);
        });
        mark_it("converts without regard to a leading zero", [] {
            std::string samp = "089";
            int out = 0;
            bool res = string_to_number_multibase(samp, out);
            return test_that(res && out == 89);
        });
        mark_it("converts based on hex prefix, with sign", [] {
            std::string samp = "-0x567.8";
            int out = 0;
            bool res = string_to_number_multibase(samp, out);
            // should round up!
            return test_that(res && out == -0x568);
        });
        mark_it("detects and converts to binary number", [] {
            std::string samp = "0b11011";
            int out = 0;
            bool res = string_to_number_multibase(samp, out);
            return test_that(res && out == 0b11011);
        });
        mark_it("fails to convert non-prefixed valid hexidecimal string", [] {
            std::string samp = "a0";
            int out = 0;
            bool res = string_to_number_multibase(samp, out);
            return test_that(!res); // no prefix... strictly decimal
        });
    });
}

void add_trim_tests() {
    using namespace cul::tree_ts;
    using cul::trim;
    describe("trim helper method")([] {
        mark_it("trims the ends of a string", [] {
            std::string samp = " a ";
            auto beg = samp.begin();
            auto end = samp.end  ();
            trim<is_whitespace>(beg, end);
            return test_that(end - beg == 1 && *beg == 'a');
        });
        mark_it("trims the ends of a c-string", [] {
            const char * str = " a ";
            auto beg = str;
            auto end = str + strlen(str);
            trim<is_whitespace>(beg, end);
            return test_that(end - beg == 1 && *beg == 'a');
        });
        mark_it("trims one end of a u32 string", [] {
            std::u32string str = U" true";
            auto beg = str.begin();
            auto end = str.end  ();
            trim<is_whitespace_u>(beg, end);
            return test_that(end - beg == 4 && char(*beg) == 't');
        });
        mark_it("trims multiple spaces", [] {
            std::string samp = "a   ";
            auto beg = samp.begin();
            auto end = samp.end();
            trim<is_whitespace>(beg, end);
            return test_that(end - beg == 1 && *beg == 'a');
        });
        mark_it("trims a string to empty if all characters match", [] {
            std::string samp = "               ";
            auto beg = samp.begin();
            auto end = samp.end();
            trim<is_whitespace>(beg, end);
            return test_that(end == beg);
        });
    });
}

void add_wrap_tests() {
    static auto do_wrap_tests = [](const char * in, int max_width, std::initializer_list<const char *> list) {
        std::vector<std::string> correct { list.begin(), list.end() };
        for (const auto & str : correct) {
            if (int(str.size()) > max_width) return false;
        }

        std::vector<std::string> res;
        cul::wrap_string_as_monowidth(in, in + strlen(in), max_width,
            [&res](const char * beg, const char * end)
        {
            cul::trim<is_whitespace>(beg, end);
            res.emplace_back(beg, end);
        }, is_whitespace);
        return std::equal(res.begin(), res.end(), correct.begin(), correct.end());
    };
    using namespace cul::tree_ts;
    describe("wrap_string_as_monowidth helper")([] {
        mark_it("breaks along a single whitespace", [] {
            return test_that(do_wrap_tests("Hello world.", 9, { "Hello", "world." }));
        });
        mark_it("breaks whitespace in the right place", [] {
            return test_that(do_wrap_tests(
            //  0123456789012345678901234
                "This is a short sentence.", 20,
                { "This is a short", "sentence." }));
        });
        mark_it("breaks into mutliple lines when appropriate and only in whitespace", [] {
            return test_that(do_wrap_tests(
            //   0123456789012345678901234
                "This is a short sentence.", 10,
                { "This is a", "short", "sentence." }));
        });
        mark_it("breaks mid-sequence if there are no breaking characters", [] {
            return test_that(do_wrap_tests(
                "-------------------------", 15,
                { "---------------", "----------" }));
        });
        mark_it("breaks into multiple lines at limit with no breaking characters", [] {
            return test_that(do_wrap_tests(
                "-------------------------", 10,
                { "----------", "----------", "-----" }));
        });
        mark_it("handles multiline complex case", [] {
            return test_that(do_wrap_tests(
            //   0123456789012345678901234
                "0 1 2 3333 4 55 6 777 8", 8,
                { "0 1 2", "3333 4", "55 6 777", "8" }));
        });
        mark_it("", [] {
            //                      0123456789ABCDEFG
            std::u32string samp = U"  -  --   ---  -";
            auto output =       { U"  -  --", U"   ---  ", U"-" };
            using ConstUIter = std::u32string::const_iterator;
            auto itr = output.begin();
            bool rv = true;
            cul::wrap_string_as_monowidth(samp.begin(), samp.end(), 8,
            [&itr, &rv, &output](ConstUIter beg, ConstUIter end) {
                using namespace cul;
                if (itr == output.end()) {
                    rv = false;
                } else if (!std::equal(beg, end, *itr++)) {
                    rv = false;
                }
                return rv ? fc_signal::k_continue : fc_signal::k_break;
            }, [](u_char c) { return c == u_char('-'); });
            return test_that(rv);
        });
    });
}

} // end of <anonymous> namespace
