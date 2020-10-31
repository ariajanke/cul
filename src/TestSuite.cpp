#include <common/TestSuite.hpp>

#include <iostream>

#include <cmath>

namespace {

std::string to_padded_string(int);

} // end of <anonymous> namespace

namespace ts {

TestAssertion test(bool v) {
   TestAssertion ta;
   ta.value = v;
   return ta;
}

TestSuite::TestSuite():
    m_test_count(0), m_test_successes(0), m_out(&std::cout)
{}

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

void TestSuite::test(TestAssertion (*test_func)()) {
    do_test_back([this, test_func]() {
        return test_func().value;
    });
}

void TestSuite::test(TestAssertion (*test_func)(TestFunc)) {
    do_test_back([this, test_func]() {
        return test_func(ts::test).value;
    });
}

template <typename Func>
void TestSuite::do_test_back(Func && f) {
    auto test_num = to_padded_string(m_test_count);
    try {
        if (f()) {
            *m_out << "[ Passed TEST " << test_num << " ]" << std::endl;
            ++m_test_successes;
        } else {
            *m_out << "[ FAILED TEST " << test_num << " ]" << std::endl;
        }
    } catch (std::exception & exp) {
        *m_out << "[ FAILED TEST " << test_num << " ]\n"
               << "Test threw an exception with the following description:\n"
               << exp.what() << std::endl;
    }
    ++m_test_count;
}

void TestSuite::finish_up() {
    if (m_test_count == 0) return;
    *m_out << "[ Passed " << m_test_successes << " / " << m_test_count
           << " test cases (" << std::to_string(int(double(m_test_successes)/double(m_test_count)*100.))
           << "%) ]" << std::endl;
    m_test_count = m_test_successes = 0;
}

bool TestSuite::has_successes_only() const
    { return m_test_successes == m_test_count; }

} // end of ts namespace

namespace {

std::string to_padded_string(int x) {
    if (x == 0) return "000";
    std::string rv = std::to_string(x);
    return std::string(std::size_t(2 - std::floor(std::log10(double(x)))), '0') + rv;
}

} // end of <anonymous> namespace
