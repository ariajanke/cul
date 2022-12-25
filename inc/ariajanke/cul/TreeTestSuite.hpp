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
#include <functional>
#include <vector>

#include <ariajanke/cul/Util.hpp>

namespace cul {

namespace tree_ts {

class TestAssertion;
class TestAssertionAttn;

TestAssertion test_that(bool);

class TestAssertion final {
    friend class TestAssertionAttn;
    bool success = false;
};

// based on ideas factorio team presented
// since C++ mocks are... not really possible
class TreeTestSuite final {
public:
    class Describer {
    public:
        virtual ~Describer() {}

        template <typename Callback>
        Describer & it(const char * it_string, Callback && callback) {
            if (at_current_it()) {
                print_it_string(it_string);
                set_current_it(callback());
            }
            increment_it_count();
            return *this;
        }

        virtual Describer & mark_source_position(int line, const char * file) = 0;

    protected:
        virtual void increment_it_count() = 0;

        virtual bool at_current_it() const = 0;

        virtual void set_current_it(TestAssertion) = 0;

        virtual void print_it_string(const char *) = 0;
    };

    class DescriberN final : public Describer {
    public:
        explicit DescriberN(std::ostream * out):
            m_out(out) {}


        bool at_end_it() const {
            return m_last_executed_test + 1 == m_max_it_count;
        }

        bool all_succeed() const {
            if (m_all_secceed == Succeed::indeterminate) {
                throw std::runtime_error{""};
            }
            return m_all_secceed == Succeed::yes;
        }

        void reset_counters() {
            m_max_it_counter = 0;
            m_has_run = false;
        }


        Describer & mark_source_position(int line, const char * file) final {
            m_source_position = SourcePosition{line, file};
            return *this;
        }

    private:
        class SourcePosition final {
        public:
            SourcePosition() {}
            SourcePosition(int line_, const char * source_file_):
                m_current_source_file(source_file_),
                m_current_source_line(line_) {}

            std::ostream & print_position(std::ostream & out) const {
                out << m_current_source_file << " line: " << m_current_source_line;
                return out;
            }

            explicit operator bool () const noexcept
                { return m_current_source_file; }

        private:
            const char * m_current_source_file = nullptr;
            int m_current_source_line = 0;
        };
        enum class Succeed {
            yes, no, indeterminate
        };

        void increment_it_count() final {
            m_max_it_count = std::max(m_max_it_count, ++m_max_it_counter);
        }

        bool at_current_it() const final {
            return    m_last_executed_test + 1 == m_max_it_counter
                   && !m_has_run;
        }

        void set_current_it(TestAssertion) final;

        void print_it_string(const char * it_string) final {
            *m_out << it_string << std::endl;
        }

        int m_max_it_count = 0;
        int m_max_it_counter = 0;
        int m_last_executed_test = -1;
        bool m_has_run = false;
        Succeed m_all_secceed = Succeed::indeterminate;
        std::ostream * m_out = nullptr;
        SourcePosition m_source_position;
    };

    using DescribeCallback = std::function<void(Describer &)>;
private:
    static constexpr const std::size_t k_no_dependancies = 0;
    struct DescribeBlock final {
        std::size_t depended_type = k_no_dependancies;
        std::vector<DescribeCallback> callbacks;
        std::string description;
    };
public:
    using DependancyMap = std::map<std::size_t, std::size_t>;

    class Described final {
    public:
        Described(DescribeBlock & block,
                  Described *& current_desc):
            m_block(block),
            m_current_desc(current_desc)
        {
            if (m_current_desc) {
                throw std::runtime_error{""};
            }
            m_current_desc = this;
        }

        Described(const Described &) = delete;

        Described(Described && lhs) = default;

        ~Described() {
            m_current_desc = nullptr;
        }

        Described & operator = (const Described &) = delete;

        template <typename DependedType>
        Described & depends_on() {
            m_block.depended_type = TreeTestSuite::type_to_id<DependedType>();
            return *this;
        }

        void operator() (void (*callback)()) {
            m_block.callbacks.emplace_back
                ([callback](Describer &) { callback(); });
        }

        void operator() (void (*callback)(Describer &))
            { m_block.callbacks.emplace_back(callback); }

    private:
        DescribeBlock & m_block;
        Described *& m_current_desc;
    };

    static TreeTestSuite & instance() {
        static TreeTestSuite suite;
        return suite;
    }

    template <typename TestedType>
    Described describe(const char * desc) {
        using namespace cul::exceptions_abbr;
        auto test_type_id = type_to_id<TestedType>();
        m_describes[test_type_id].description = desc;
        return Described
            {m_describes[test_type_id], m_current_describe};
    }

    int run_tests() {
        using namespace cul::exceptions_abbr;
        std::set<std::size_t> met_depends;
        std::size_t old_size = m_describes.size();
        bool rv = true;
        while (!m_describes.empty()) {
            bool gv = pop_non_depending_tests
                (m_describes, m_current_describer, met_depends, m_out);
            if (m_describes.size() == old_size) {
                throw RtError{""};
            }
            old_size = m_describes.size();
            rv = gv || rv;
        }
        return int(!rv);
    }

    Describer & current_describer() const {
        if (!m_current_describer) {
            throw std::runtime_error{""};
        }
        return *m_current_describer;
    }
private:
    template <typename TestedType>
    static std::size_t type_to_id() {
        static uint8_t a;
        return reinterpret_cast<std::size_t>(&a);
    }

    static bool pop_non_depending_tests
        (std::map<std::size_t, DescribeBlock> & describes,
         Describer *& describer,
         std::set<std::size_t> & met_depends,
         std::ostream * out)
    {
        bool rv = true;
        for (auto itr = describes.begin(); itr != describes.end(); ) {
            auto jtr = met_depends.find(itr->second.depended_type);
            if (   itr->second.depended_type == k_no_dependancies
                || jtr != met_depends.end())
            {
                bool gv = run_describe(itr->second, describer, out);
                rv = rv && gv;
                met_depends.insert(itr->first);
                itr = describes.erase(itr);
            } else {
                ++itr;
            }
        }
        for (auto & pair : describes) {
            const auto & depended_type = pair.second.depended_type;
            if (met_depends.find(depended_type) != met_depends.end()) {
                pair.second.depended_type = k_no_dependancies;
            }
        }
        return rv;
    }

    static bool run_describe
        (DescribeBlock & block, Describer *& describer_ptr, std::ostream * out) noexcept
    {
        *out << block.description << std::endl;
        bool all_succeed = true;
        for (auto & callback : block.callbacks) {
            DescriberN describer{out};
            describer_ptr = &describer;
            do {
                describer.reset_counters();
                callback(describer);
            } while (!describer.at_end_it());
            if (!describer.all_succeed()) {
                all_succeed = false;
            }
        }
        block.callbacks.clear();
        describer_ptr = nullptr;
        return all_succeed;
    }

    std::map<std::size_t, DescribeBlock> m_describes;
    Described * m_current_describe = nullptr;
    Describer * m_current_describer = nullptr;

    std::ostream * m_out = &std::cout;
};

class TestAssertionAttn final {
    friend class TreeTestSuite::DescriberN;
    friend TestAssertion test_that(bool);

    static TestAssertion make(bool b) {
        TestAssertion ta;
        ta.success = b;
        return ta;
    }

    static bool get(const TestAssertion & ta)
        { return ta.success; }
};

inline void TreeTestSuite::DescriberN::set_current_it(TestAssertion ta) {
    ++m_last_executed_test;
    m_has_run = true;
    auto to_succeed = [](bool b)
        { return b ? Succeed::yes : Succeed::no; };
    auto ta_val = TestAssertionAttn::get(ta);
    if (!ta_val) {
        *m_out << "Test failed: ";
        if (m_source_position) {
            m_source_position.print_position(*m_out);
        }
        *m_out << std::endl;
    }
    switch (m_all_secceed) {
    case Succeed::indeterminate:
    case Succeed::yes:
        m_all_secceed = to_succeed(ta_val);
        break;
    case Succeed::no: break;
    }
    m_source_position = SourcePosition{};
}

template <typename TestedType>
TreeTestSuite::Described describe(const char * description) {
    return TreeTestSuite::instance().describe<TestedType>(description);
}

template <typename Callback>
TreeTestSuite::Describer & it(const char * it_string, Callback && callback) {
    return TreeTestSuite::instance().current_describer().
        it(it_string, std::move(callback));
}

inline TestAssertion test_that(bool b)
    { return TestAssertionAttn::make(b); }

inline int run_tests()
    { return TreeTestSuite::instance().run_tests(); }

} // end of tree_ts namespace -> into ::cul

} // end of cul namespace
