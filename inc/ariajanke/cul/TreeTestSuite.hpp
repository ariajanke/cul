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

#include <ariajanke/cul/Util.hpp>

#include <map>
#include <set>
#include <vector>
#include <iostream>
#include <string>
#include <functional>
#include <vector>

#include <cstring>

namespace cul {

namespace tree_ts {

class TestAssertion;
class TestAssertionAttn;

TestAssertion test_that(bool);

class TestAssertion final {
    friend class TestAssertionAttn;
    bool success = false;
};

class TreeTestSuite;
class TestAssertionAttn;

class Describer {
public:
    virtual ~Describer() {}

    template <typename Callback>
    Describer & it(const char * it_string, Callback && callback);

    virtual Describer & mark_source_position(int line, const char * file) = 0;

protected:
    virtual void increment_it_count() = 0;

    virtual bool at_current_it() const = 0;

    virtual void set_current_it(TestAssertion) = 0;

    virtual void print_it_string(const char *) = 0;

    virtual void handle_exception
        (const char * it_string, const std::exception *) noexcept = 0;
};

class TreeTestPriv {
    friend class TreeTestSuite;
    friend class Described;
    friend class TestAssertionAttn;

    // class TreeTestPriv

    using DescribeCallback = std::function<void(Describer &)>;

    class SourcePosition final {
    public:
        SourcePosition() {}
        SourcePosition(int line_, const char * source_file_);

        std::ostream & print_position(std::ostream & out) const;

        explicit operator bool () const noexcept
            { return m_current_source_file; }

        static const char * truncate_filename(const char * filename);

    private:
        const char * m_current_source_file = nullptr;
        int m_current_source_line = 0;
    };

    // class TreeTestPriv

    class DescriberN final : public Describer {
    public:
        explicit DescriberN(std::ostream * out):
            m_out(out) {}

        bool at_end_it() const;

        bool all_succeed() const;

        void reset_counters();

        Describer & mark_source_position(int line, const char * file) final;

    private:
        enum class Succeed {
            yes, no, indeterminate
        };

        void increment_it_count() final;

        bool at_current_it() const final;

        void set_current_it(TestAssertion) final;

        void print_it_string(const char * it_string) final;

        void handle_exception
            (const char * it_string, const std::exception * exp) noexcept final;

        int m_max_it_count = 0;
        int m_max_it_counter = 0;
        int m_last_executed_test = -1;
        bool m_has_run = false;
        Succeed m_all_secceed = Succeed::indeterminate;
        std::ostream * m_out = nullptr;
        SourcePosition m_source_position;
    };

    // class TreeTestPriv

    struct DescribeType final {
        struct Block final {
            DescribeCallback callback;
            std::string description;
        };

        static constexpr const std::size_t k_no_dependancies = 0;

        bool all_passes = false;
        std::size_t depended_type = k_no_dependancies;
        std::vector<Block> blocks;
    };

    // class TreeTestPriv

    class SuiteForDescribed final {
    public:
        SuiteForDescribed(std::map<std::size_t, DescribeType> & describes,
                          bool & has_current_desc);

        SuiteForDescribed(SuiteForDescribed && lhs);

        SuiteForDescribed(const SuiteForDescribed &) = delete;

        SuiteForDescribed & operator = (SuiteForDescribed &&) = delete;

        SuiteForDescribed & operator = (const SuiteForDescribed &) = delete;

        ~SuiteForDescribed() {}

        void clear_all_describes();

        void clear_current_describe();

        void forget_current_describe();

        bool has_current_describe() const noexcept;

    private:
        void set_as_having_current_describe();

        std::map<std::size_t, DescribeType> & m_describes;
        bool * m_has_current_desc = nullptr;
    };

    template <typename TestedType>
    static std::size_t type_to_id();
}; // end of class TreeTestPriv



class Described final {
    using DescribeType = TreeTestPriv::DescribeType;
    using SuiteForDescribed = TreeTestPriv::SuiteForDescribed;
public:
    Described(DescribeType::Block & block,
              std::size_t & depended_type,
              SuiteForDescribed && suite_desc);

    Described(const Described &) = delete;

    Described(Described && lhs);

    ~Described();

    Described & operator = (const Described &) = delete;

    template <typename DependedType>
    Described depends_on();

    void operator() (void (*callback)())
        { m_block.callback = [callback](Describer &) { callback(); }; }

    void operator() (void (*callback)(Describer &))
        { m_block.callback = callback; }

private:
    DescribeType::Block & m_block;
    std::size_t & m_depended_type;
    SuiteForDescribed m_suite_desc;
};

// based on ideas factorio team presented
// since C++ mocks are... not really possible
class TreeTestSuite final {
public:
    using DependancyMap = std::map<std::size_t, std::size_t>;

    static TreeTestSuite & instance();

    template <typename TestedType>
    Described describe(const char * description_text);

    Described describe(const char * description_text);

    int run_tests();

    Describer & current_describer() const;

private:
    using DescribeType = TreeTestPriv::DescribeType;

    static bool pop_non_depending_tests
        (std::map<std::size_t, DescribeType> & unrun_describes,
         std::map<std::size_t, DescribeType> & already_run_describes,
         Describer *&, std::ostream * out);

    static bool run_describe
        (DescribeType &, Describer *&, std::ostream * out) noexcept;

    static void list_unrun_blocks
        (const DescribeType &, std::ostream * out) noexcept;

    void clear();

    std::map<std::size_t, DescribeType> m_describes;
    bool m_has_active_desribe = false;
    Describer * m_current_describer = nullptr;

    std::ostream * m_out = &std::cout;
};

inline Described describe(const char * description)
    { return TreeTestSuite::instance().describe(description); }

template <typename TestedType>
Described describe(const char * description)
    { return TreeTestSuite::instance().describe<TestedType>(description); }

template <typename Callback>
Describer & it(const char * it_string, Callback && callback) {
    return TreeTestSuite::instance().current_describer().
        it(it_string, std::move(callback));
}

Describer & mark_source_position(int line, const char * file) {
    return TreeTestSuite::instance().current_describer().
        mark_source_position(line, file);
}

TestAssertion test_that(bool);

inline int run_tests()
    { return TreeTestSuite::instance().run_tests(); }

// ----------------------------------------------------------------------------

class TestAssertionAttn final {
    friend class TreeTestPriv::DescriberN;
    friend TestAssertion test_that(bool);

    static TestAssertion make(bool b) {
        TestAssertion ta;
        ta.success = b;
        return ta;
    }

    static bool get(const TestAssertion & ta)
        { return ta.success; }
};

// ----------------------------------------------------------------------------

template <typename TestedType>
/* private static */ std::size_t TreeTestPriv::type_to_id() {
    static uint8_t a;
    return reinterpret_cast<std::size_t>(&a);
}

// ----------------------------------------------------------------------------

template <typename Callback>
Describer & Describer::it(const char * it_string, Callback && callback) {
    if (at_current_it()) {
        try {
            set_current_it(callback());
            print_it_string(it_string);
        } catch (std::exception & exp) {
            handle_exception(it_string, &exp);
        } catch (...) {
            handle_exception(it_string, nullptr);
        }
    }
    increment_it_count();
    return *this;
}

// ----------------------------------------------------------------------------

inline Described::Described
    (DescribeType::Block & block, std::size_t & depended_type,
     SuiteForDescribed && suite_desc):
    m_block(block),
    m_depended_type(depended_type),
    m_suite_desc(std::move(suite_desc)) {}

inline Described::Described(Described && lhs):
    m_block(lhs.m_block),
    m_depended_type(lhs.m_depended_type),
    m_suite_desc(std::move(lhs.m_suite_desc))
{ lhs.m_suite_desc.forget_current_describe(); }

inline Described::~Described()
    { m_suite_desc.clear_current_describe(); }

template <typename DependedType>
Described Described::depends_on() {
    using namespace cul::exceptions_abbr;
    const auto depend_type_id = TreeTestPriv::type_to_id<DependedType>();
    if (   m_depended_type != DescribeType::k_no_dependancies
        && m_depended_type != depend_type_id)
    {
        m_suite_desc.clear_all_describes();
        throw RtError{"Depended type already set, 'depends_on' may "
                      "only be called once per type"};
    }
    m_depended_type = depend_type_id;
    if (!m_suite_desc.has_current_describe()) {
        m_suite_desc.clear_all_describes();
        throw RtError{""};
    }
    return Described{m_block, m_depended_type, std::move(m_suite_desc)};
}

// ----------------------------------------------------------------------------

/* static */ TreeTestSuite & TreeTestSuite::instance() {
    static TreeTestSuite suite;
    return suite;
}

template <typename TestedType>
Described TreeTestSuite::describe(const char * desc) {
    using namespace cul::exceptions_abbr;
    auto test_type_id = TreeTestPriv::type_to_id<TestedType>();
    auto & blocks = m_describes[test_type_id].blocks;
    blocks.push_back(DescribeType::Block{});
    DescribeType::Block & block = blocks.back();
    block.description = desc;
    return Described
        {block, m_describes[test_type_id].depended_type,
         TreeTestPriv::SuiteForDescribed{m_describes, m_has_active_desribe}};
}

inline Described TreeTestSuite::describe(const char * desc) {
    class Impl final {};
    class DepImpl final {};

    // force it as having passed
    m_describes[TreeTestPriv::type_to_id<DepImpl>()].all_passes = true;
    return describe<Impl>(desc).depends_on<DepImpl>();
}

inline int TreeTestSuite::run_tests() {
    using namespace cul::exceptions_abbr;
    std::size_t old_size = m_describes.size();
    bool rv = true;
    auto unrun_describes = std::move(m_describes);
    std::map<std::size_t, DescribeType> already_ran;
    m_describes.clear();

    while (!unrun_describes.empty()) {
        bool gv = pop_non_depending_tests
            (unrun_describes, already_ran, m_current_describer, m_out);

        rv = gv || rv;
        if (unrun_describes.size() == old_size)
            { break; }
        old_size = unrun_describes.size();
    }
    for (auto & pair : unrun_describes) {
        list_unrun_blocks(pair.second, m_out);
    }
    clear();
    return int(!rv);
}

inline Describer & TreeTestSuite::current_describer() const {
    if (!m_current_describer) {
        using namespace cul::exceptions_abbr;
        throw RtError{"TreeTestSuite::current_describer: no current describer"};
    }
    return *m_current_describer;
}

/* private static */ inline bool TreeTestSuite::pop_non_depending_tests
    (std::map<std::size_t, DescribeType> & unrun_describes,
     std::map<std::size_t, DescribeType> & already_run_describes,
     Describer *& describer, std::ostream * out)
{
    bool rv = true;
    for (auto itr = unrun_describes.begin(); itr != unrun_describes.end(); ) {
        auto run_describe_ = [itr, &describer, out]
            { return run_describe(itr->second, describer, out); };
        // weird things with iterator invalidation
        auto mark_as_ran =
            [&already_run_describes, &unrun_describes]
            (decltype(unrun_describes.begin()) itr)
        {
            auto next_itr = itr;
            ++next_itr;
            already_run_describes.insert(unrun_describes.extract(itr));
            return next_itr;
        };

        if (itr->second.depended_type == DescribeType::k_no_dependancies) {
            // no dependants
            rv = rv && run_describe_();
            itr = mark_as_ran(itr);
        } else {
            auto dependee = already_run_describes.find(itr->second.depended_type);
            if (dependee != already_run_describes.end()) {
                // has ran dependees
                rv = rv && run_describe_();
                itr = mark_as_ran(itr);
            } else {
                // yet to run dependees
                ++itr;
            }
        }
    }
    return rv;
}

/* private static */ inline bool TreeTestSuite::run_describe
    (DescribeType & type, Describer *& describer_ptr, std::ostream * out) noexcept
{
    bool all_succeed = true;
    for (auto & block : type.blocks) {
        TreeTestPriv::DescriberN describer{out};
        describer_ptr = &describer;
        *out << block.description << std::endl;
        do {
            describer.reset_counters();
            block.callback(describer);
        } while (!describer.at_end_it());
        if (!describer.all_succeed()) {
            all_succeed = false;
        }
    }
    type.blocks.clear();
    type.all_passes = all_succeed;
    describer_ptr = nullptr;
    return all_succeed;
}

/* private static */ inline void TreeTestSuite::list_unrun_blocks
    (const DescribeType & type, std::ostream * out) noexcept
{
    *out << "Not running following describe blocks, due to previously failed tests:\n";
    for (auto & block : type.blocks) {
        *out << "\t" << block.description << "\n";
    }
    *out << std::flush;
}

/* private */ void TreeTestSuite::clear() {
    m_describes.clear();
    m_has_active_desribe = false;
    m_current_describer = nullptr;
}

// ----------------------------------------------------------------------------

inline TestAssertion test_that(bool b)
    { return TestAssertionAttn::make(b); }

// ----------------------------------------------------------------------------

inline TreeTestPriv::SourcePosition::SourcePosition
    (int line_, const char * source_file_):
    m_current_source_file(source_file_),
    m_current_source_line(line_) {}

inline std::ostream & TreeTestPriv::SourcePosition::print_position
    (std::ostream & out) const
{
    out << truncate_filename(m_current_source_file) << " line: "
        << m_current_source_line;
    return out;
}

/* static */ inline const char * TreeTestPriv::SourcePosition::truncate_filename
    (const char * filename)
{
    static constexpr const auto k_dir_slash = '/';
    auto fileend = filename + ::strlen(filename);
    auto itr = std::make_reverse_iterator(fileend);
    auto end = std::make_reverse_iterator(filename);
    for (auto jtr = itr; jtr != end; ++jtr) {
        if (*jtr == k_dir_slash) {
            break;
        }
    }
    return std::find(itr, end, k_dir_slash).base();
}

// ----------------------------------------------------------------------------

inline bool TreeTestPriv::DescriberN::at_end_it() const
    { return m_last_executed_test + 1 == m_max_it_count; }

inline bool TreeTestPriv::DescriberN::all_succeed() const {
    if (m_all_secceed == Succeed::indeterminate) {
        throw std::runtime_error{""};
    }
    return m_all_secceed == Succeed::yes;
}

inline void TreeTestPriv::DescriberN::reset_counters() {
    m_max_it_counter = 0;
    m_has_run = false;
}

inline Describer & TreeTestPriv::DescriberN::mark_source_position
    (int line, const char * file)
{
    m_source_position = SourcePosition{line, file};
    return *this;
}

/* private */ inline void TreeTestPriv::DescriberN::increment_it_count()
    { m_max_it_count = std::max(m_max_it_count, ++m_max_it_counter); }

/* private */ inline bool TreeTestPriv::DescriberN::at_current_it() const {
    return    m_last_executed_test + 1 == m_max_it_counter
           && !m_has_run;
}

/* private */ inline void TreeTestPriv::DescriberN::set_current_it
    (TestAssertion ta)
{
    ++m_last_executed_test;
    m_has_run = true;
    auto to_succeed = [](bool b)
        { return b ? Succeed::yes : Succeed::no; };
    auto ta_val = TestAssertionAttn::get(ta);
    if (!ta_val) {
        *m_out << "\t[ Test Failed ] ";
        if (m_source_position) {
            m_source_position.print_position(*m_out);
        } else {
            *m_out << "(source position not set)";
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

/* private */ inline void TreeTestPriv::DescriberN::print_it_string
    (const char * it_string)
{ *m_out << "\t" << it_string << std::endl; }

/* private */ inline void TreeTestPriv::DescriberN::handle_exception
    (const char * it_string, const std::exception * exp) noexcept
{
    set_current_it(test_that(false));
    print_it_string(it_string);
    if (exp) {
        *m_out << "An exception was thrown:\n\t"
               << exp->what() << std::endl;
    } else {
        *m_out << "An unknown exception was thrown." << std::endl;
    }
}

// ----------------------------------------------------------------------------

inline TreeTestPriv::SuiteForDescribed::SuiteForDescribed
    (std::map<std::size_t, DescribeType> & describes, bool & has_current_desc):
    m_describes(describes),
    m_has_current_desc(&has_current_desc)
{ set_as_having_current_describe(); }

inline TreeTestPriv::SuiteForDescribed::SuiteForDescribed
    (SuiteForDescribed && lhs):
    m_describes(lhs.m_describes),
    m_has_current_desc(lhs.m_has_current_desc)
{
    using namespace cul::exceptions_abbr;
    if (!m_has_current_desc) {
        throw RtError{""};
    }
    lhs.m_has_current_desc = nullptr;
}

inline void TreeTestPriv::SuiteForDescribed::clear_all_describes() {
    clear_current_describe();
    m_describes.clear();
}

inline void TreeTestPriv::SuiteForDescribed::clear_current_describe() {
    if (!m_has_current_desc) return;
    *m_has_current_desc = false;
}

inline void TreeTestPriv::SuiteForDescribed::forget_current_describe()
    { m_has_current_desc = nullptr; }

inline bool TreeTestPriv::SuiteForDescribed::has_current_describe() const noexcept
    { return m_has_current_desc ? *m_has_current_desc : false; }

/* private */ inline void TreeTestPriv::SuiteForDescribed::set_as_having_current_describe() {
    using namespace cul::exceptions_abbr;
    if (*m_has_current_desc) {
        clear_all_describes();
        throw std::runtime_error{""};
    }
    *m_has_current_desc = true;
}

} // end of tree_ts namespace -> into ::cul

} // end of cul namespace
