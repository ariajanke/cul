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

#pragma once

#include <map>
#include <set>
#include <vector>
#include <iostream>
#include <string>

#include <ariajanke/cul/Util.hpp>

namespace cul {

namespace tree_ts {

class TestAssertion final {};

// based on ideas factorio team presented
// since C++ mocks are... not really possible
class TreeTestSuite final {
public:
    struct Describer final {
        // behavior: pushes a string onto a stack
        template <typename Callback>
        Describer & context(const char *, Callback &&);

        template <typename Callback>
        Describer & it(const char *, Callback &&);
    };

    static TreeTestSuite & instance();

    template <typename TestedType>
    TreeTestSuite & describe(const char * desc) {
        using namespace cul::exceptions_abbr;
        auto test_type_id = type_to_id<TestedType>();
        auto itr = m_ran_describes.find(test_type_id);
        if (itr != m_ran_describes.end()) {
            throw RtError{"Describe for a type can only be called once."};
        }
        m_ran_describes.insert(test_type_id);
        if (!m_describe_stack.empty())
            m_describe_stack.pop_back();
        m_describe_stack.push_back(desc);


    }

    template <typename DependedType>
    TreeTestSuite & depends_on();

    TreeTestSuite & operator() (void (*callback)());

    TreeTestSuite & operator() (void (*callback)(Describer &));

private:
    template <typename TestedType>
    static std::size_t type_to_id() {
        static uint8_t a;
        return reinterpret_cast<std::size_t>(&a);
    }

    std::map<std::size_t, std::size_t> m_dependancies;
    std::set<std::size_t> m_ran_describes;
    std::vector<std::string> m_describe_stack;
    std::ostream * m_out = &std::cout;
};

template <typename TestedType>
TreeTestSuite & describe(const char *) {}

template <typename Callback>
TreeTestSuite::Describer & context(const char *, Callback &&) {}

template <typename Callback>
TreeTestSuite::Describer & it(const char *, Callback &&) {}

TestAssertion test_that(bool);

void run_tests();

} // end of tree_ts namespace -> into ::cul

} // end of cul namespace
