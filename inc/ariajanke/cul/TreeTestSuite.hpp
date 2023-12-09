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

#pragma once

#include <ariajanke/cul/detail/tree-test-helpers.hpp>

#include <iostream>

#include <cstring>

namespace cul {

namespace tree_ts {

class TestAssertion final {
public:
    static TestAssertion test_that(bool s) { return TestAssertion{s}; }

    static TestAssertion test_failed() { return TestAssertion{false}; }

    // change this to enum?
    bool success(const detail::TestAssertionCheck &) const noexcept
        { return m_success; }

private:
    explicit TestAssertion(bool s): m_success(s) {}

    friend class TestAssertionAttn;
    bool m_success = false;
};

// ----------------------------------------------------------------------------

class Describer final : public detail::DescribedBase {
public:
    explicit Describer(std::ostream * out):
        m_out(out) {}

    template <typename Callback>
    Describer & it(const char * it_string, Callback && callback);

    Describer & mark_source_position(int line, const char * filename);

    template <typename Callback>
    Describer & next(Callback && callback);

    bool run(DescribeType::Block &);

private:
    enum class Succeed {
        yes, no, indeterminate
    };

    bool at_end_it() const;

    void increment_it_count();

    bool at_current_it() const;

    bool has_run() const noexcept { return m_has_run; }

    void set_current_it(TestAssertion);

    void print_it_string(const char * it_string);

    void handle_exception
        (const char * it_string, const std::exception * exp) noexcept;

    int m_max_it_count = 0;
    int m_max_it_counter = 0;
    int m_last_executed_test = -1;
    bool m_has_run = false;
    Succeed m_all_secceed = Succeed::indeterminate;
    std::ostream * m_out = nullptr;
    detail::SourcePosition m_source_position;
};

// ----------------------------------------------------------------------------

class Described final : public detail::DescribedBase {
public:
    Described
        (TreeTestSuite & parent_suite,
         DescribeType::Block & block,
         std::size_t & depended_type,
         detail::SuiteForDescribed && suite_desc);

    Described(const Described &) = delete;

    Described(Described && lhs);

    ~Described();

    Described & operator = (const Described &) = delete;

    template <typename DependedType>
    Described depends_on();

    TreeTestSuite & operator() (void (*callback)());

    TreeTestSuite & operator() (void (*callback)(Describer &));

private:
    TreeTestSuite & m_parent_suite;
    DescribeType::Block & m_block;
    std::size_t & m_depended_type;
    detail::SuiteForDescribed m_suite_desc;
};

// ----------------------------------------------------------------------------

class TreeTestSuite final : public detail::DescribedBase {
public:
    using DependancyMap = std::map<std::size_t, std::size_t>;

    static TreeTestSuite & instance();

    template <typename TestedType>
    Described describe(const char * description_text);

    Described describe(const char * description_text);

    int run_tests();

    Describer & current_describer() const;

    // a nice to have if we're running tests only a file at a time
    // where there maybe depends on not specified in that file
    TreeTestSuite & pass_all_undescribed_types();

private:
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

// ----------------------------------------------------------------------------

inline Described describe(const char * description)
    { return TreeTestSuite::instance().describe(description); }

template <typename TestedType>
Described describe(const char * description)
    { return TreeTestSuite::instance().describe<TestedType>(description); }

// test me
template <typename ExpType, typename Func>
TestAssertion expect_exception(Func &&);

inline int run_tests()
    { return TreeTestSuite::instance().run_tests(); }

TestAssertion test_that(bool);

TestAssertion test_failed();

// ------------------------- END OF PUBLIC INTERFACE --------------------------

namespace detail {

template <typename TestedType>
/* protected static */ std::size_t DescribedBase::type_to_id() {
    static uint8_t a;
    return reinterpret_cast<std::size_t>(&a);
}

// ----------------------------------------------------------------------------

inline SuiteForDescribed::SuiteForDescribed
    (std::map<std::size_t, DescribeType> & describes, bool & has_current_desc):
    m_describes(describes),
    m_has_current_desc(&has_current_desc)
{ set_as_having_current_describe(); }

inline SuiteForDescribed::SuiteForDescribed
    (SuiteForDescribed && lhs):
    m_describes(lhs.m_describes),
    m_has_current_desc(lhs.m_has_current_desc)
{
    if (!m_has_current_desc) {
        throw std::runtime_error{"Cannot mark current desribe (not set)"};
    }
    lhs.m_has_current_desc = nullptr;
}

inline void SuiteForDescribed::clear_all_describes() {
    clear_current_describe();
    m_describes.clear();
}

inline void SuiteForDescribed::clear_current_describe() {
    if (!m_has_current_desc) return;
    *m_has_current_desc = false;
}

inline void SuiteForDescribed::forget_current_describe()
    { m_has_current_desc = nullptr; }

inline bool SuiteForDescribed::has_current_describe() const noexcept
    { return m_has_current_desc ? *m_has_current_desc : false; }

/* private */ inline void SuiteForDescribed::set_as_having_current_describe() {
    if (*m_has_current_desc) {
        clear_all_describes();
        throw std::runtime_error{"Cannot mark current desribe (not set)"};
    }
    *m_has_current_desc = true;
}

// ----------------------------------------------------------------------------

inline SourcePosition::SourcePosition
    (int line_, const char * source_file_):
    m_current_source_file(source_file_),
    m_current_source_line(line_) {}

inline std::ostream & SourcePosition::print_position
    (std::ostream & out) const
{
    out << truncate_filename(m_current_source_file) << " line: "
        << m_current_source_line;
    return out;
}

/* static */ inline const char * SourcePosition::truncate_filename
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

} // end of ::cul::detail namespace -> ::cul

// ----------------------------------------------------------------------------

template <typename Callback>
Describer & it(const char * it_string, Callback && callback) {
    return TreeTestSuite::instance().   
        current_describer().
        it(it_string, std::move(callback));
}

inline Describer & mark_source_position(int line, const char * file) {
    return TreeTestSuite::instance().
        current_describer().
        mark_source_position(line, file);
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

inline Describer & Describer::mark_source_position
    (int line, const char * file)
{
    m_source_position = detail::SourcePosition{line, file};
    return *this;
}

template <typename Callback>
Describer & Describer::next(Callback && callback) {
    if (!has_run())
        { callback(); }
    return *this;
}

inline bool Describer::run(DescribeType::Block & block) {
    do {
        m_max_it_counter = 0;
        m_has_run = false;
        // GAWD
        block.callback(*this);
    } while (!at_end_it());

    switch (m_all_secceed) {
    case Succeed::indeterminate:
        *m_out << "[[ !! This describe has no it statements !! ]"
            << std::endl;
        [[fallthrough]];
    case Succeed::no: return false;
    default         : return true;
    }
}

/* private */ inline bool Describer::at_end_it() const
    { return m_last_executed_test + 1 == m_max_it_count; }

/* private */ inline void Describer::increment_it_count()
    { m_max_it_count = std::max(m_max_it_count, ++m_max_it_counter); }

/* private */ inline bool Describer::at_current_it() const {
    return    m_last_executed_test + 1 == m_max_it_counter
           && !m_has_run;
}

/* private */ inline void Describer::set_current_it
    (TestAssertion ta)
{
    ++m_last_executed_test;
    m_has_run = true;
    auto to_succeed = [](bool b)
        { return b ? Succeed::yes : Succeed::no; };

    auto ta_successful = ta.success(detail::TestAssertionCheck{});
    if (!ta_successful) {
        *m_out << k_indent << "[ Test Failed ] ";
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
        m_all_secceed = to_succeed(ta_successful);
        break;
    case Succeed::no: break;
    }
    m_source_position = detail::SourcePosition{};
}

/* private */ inline void Describer::print_it_string
    (const char * it_string)
{ *m_out << k_indent << it_string << std::endl; }

/* private */ inline void Describer::handle_exception
    (const char * it_string, const std::exception * exp) noexcept
{
    set_current_it(test_failed());
    print_it_string(it_string);
    if (exp) {
        *m_out << "An exception was thrown:\n" << k_indent << exp->what()
               << std::endl;
    } else {
        *m_out << "An unknown exception was thrown." << std::endl;
    }
}

// ----------------------------------------------------------------------------

inline Described::Described
    (TreeTestSuite & parent_suite,
     DescribeType::Block & block,
     std::size_t & depended_type,
     detail::SuiteForDescribed && suite_desc):
    m_parent_suite(parent_suite),
    m_block(block),
    m_depended_type(depended_type),
    m_suite_desc(std::move(suite_desc)) {}

inline Described::Described(Described && lhs):
    m_parent_suite(lhs.m_parent_suite),
    m_block(lhs.m_block),
    m_depended_type(lhs.m_depended_type),
    m_suite_desc(std::move(lhs.m_suite_desc))
{ lhs.m_suite_desc.forget_current_describe(); }

inline Described::~Described()
    { m_suite_desc.clear_current_describe(); }

template <typename DependedType>
Described Described::depends_on() {
    using RuntimeError = std::runtime_error;
    const auto depend_type_id = type_to_id<DependedType>();
    if (m_depended_type == type_to_id<PlainDescribe>() &&
        depend_type_id  != type_to_id<PlainDescribe>()   )
    {
        throw RuntimeError{"Untyped describes may not depend on anything"};
    }
    if (   m_depended_type != DescribeType::k_no_dependancies
        && m_depended_type != depend_type_id)
    {
        m_suite_desc.clear_all_describes();
        throw RuntimeError{"Depended type already set, 'depends_on' may "
                           "only be called once per type"};
    }
    m_depended_type = depend_type_id;
    if (!m_suite_desc.has_current_describe()) {
        m_suite_desc.clear_all_describes();
        throw RuntimeError{"Described does not control its suite"};
    }
    return Described{m_parent_suite, m_block, m_depended_type, std::move(m_suite_desc)};
}

inline TreeTestSuite & Described::operator() (void (*callback)()) {
    m_block.callback = [callback](Describer &) { callback(); };
    m_suite_desc.clear_current_describe();
    m_suite_desc.forget_current_describe();
    return m_parent_suite;
}

inline TreeTestSuite & Described::operator() (void (*callback)(Describer &)) {
    m_block.callback = callback;
    m_suite_desc.clear_current_describe();
    m_suite_desc.forget_current_describe();
    return m_parent_suite;
}

// ----------------------------------------------------------------------------

/* static */ inline TreeTestSuite & TreeTestSuite::instance() {
    static TreeTestSuite suite;
    return suite;
}

template <typename TestedType>
Described TreeTestSuite::describe(const char * desc) {
    auto test_type_id = type_to_id<TestedType>();
    auto & blocks = m_describes[test_type_id].blocks;
    auto & depended_type =
        m_describes[test_type_id].depended_type;
    blocks.push_back(DescribeType::Block{});
    DescribeType::Block & block = blocks.back();
    block.description = desc;
    return Described
        {*this,
         block,
         depended_type,
         detail::SuiteForDescribed{m_describes, m_has_active_desribe}};
}

inline Described TreeTestSuite::describe(const char * desc) {
    struct Impl final {};
    return describe<Impl>(desc).depends_on<PlainDescribe>();
}

inline int TreeTestSuite::run_tests() {
    std::size_t old_size = m_describes.size();
    bool rv = true;
    auto unrun_describes = std::move(m_describes);
    std::map<std::size_t, DescribeType> already_ran;
    m_describes.clear();

    while (!unrun_describes.empty()) {
        bool gv = pop_non_depending_tests
            (unrun_describes, already_ran, m_current_describer, m_out);

        rv = gv && rv;
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
    using RuntimeError = std::runtime_error;
    if (!m_current_describer) {
        throw RuntimeError
            {"TreeTestSuite::current_describer: no current describer"};
    }
    return *m_current_describer;
}

inline TreeTestSuite & TreeTestSuite::pass_all_undescribed_types() {
    for (auto & pair : m_describes) {
        auto & describe = pair.second;
        auto itr = m_describes.find(describe.depended_type);
        if (itr != m_describes.end())
            { continue; }
        describe.depended_type = type_to_id<PlainDescribe>();
    }
    return *this;
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

        if (itr->second.depended_type == DescribeType::k_no_dependancies ||
            itr->second.depended_type == type_to_id<PlainDescribe>())
        {
            // no dependants
            rv = run_describe_() && rv;
            itr = mark_as_ran(itr);
        } else {
            auto dependee = already_run_describes.find(itr->second.depended_type);
            if (dependee != already_run_describes.end()) {
                // has ran dependees
                if (dependee->second.all_passes) {
                    rv = run_describe_() && rv;
                }
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
        Describer describer{out};
        describer_ptr = &describer;
        *out << block.description << std::endl;
        bool res = describer.run(block);
        all_succeed = res && all_succeed;
    }
    type.blocks.clear();
    type.all_passes = all_succeed;
    describer_ptr = nullptr;
    return all_succeed;
}

/* private static */ inline void TreeTestSuite::list_unrun_blocks
    (const DescribeType & type, std::ostream * out) noexcept
{
    *out << "Not running following describes, due to previously failed tests:\n";
    for (auto & block : type.blocks) {
        *out << k_indent << block.description << "\n";
    }
    *out << std::flush;
}

/* private */ inline void TreeTestSuite::clear() {
    m_describes.clear();
    m_has_active_desribe = false;
    m_current_describer = nullptr;
}

// ----------------------------------------------------------------------------

template <typename ExpType, typename Func>
TestAssertion expect_exception(Func && f) {
    try {
        f();
    } catch (ExpType &) {
        return test_that(true);
    }
    return test_failed();
}

inline TestAssertion test_that(bool s)
    { return TestAssertion::test_that(s); }

inline TestAssertion test_failed() 
    { return TestAssertion::test_failed(); }

} // end of tree_ts namespace -> into ::cul

} // end of cul namespace
