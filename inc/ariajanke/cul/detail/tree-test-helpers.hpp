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

#include <functional>
#include <string>
#include <vector>
#include <map>

namespace cul {

namespace tree_ts {

class Describer;
class TreeTestSuite;

namespace detail {

class DescribedBase {
protected:
    using DescribeCallback = std::function<void(Describer &)>;

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

    struct PlainDescribe final {};

    static constexpr const auto k_indent = "  ";

    template <typename TestedType>
    static std::size_t type_to_id();
};

class SuiteForDescribed final : public DescribedBase {
public:
    SuiteForDescribed
        (std::map<std::size_t, DescribeType> & describes,
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

class TestAssertionCheck final {};

} // end of ::cul::tree_ts::detail namespace -> ::cul::tree_ts

} // end of ::cul::tree_ts namespace -> ::cul

} // end of ::cul namespace
