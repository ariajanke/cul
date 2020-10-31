#pragma once

#include <iosfwd>

namespace ts {

class TestAssertion;
class TestSuite;

TestAssertion test(bool);

using TestFunc = TestAssertion(*)(bool);

// semantic object
class TestAssertion {
    friend TestAssertion test(bool);
    friend class TestSuite;
    bool value;
};

class TestSuite {
public:
    TestSuite();
    explicit TestSuite(const char * series_name);
    
    TestSuite(const TestSuite &) = delete;
    TestSuite(TestSuite &&) = delete;
    
    ~TestSuite();
    
    TestSuite & operator = (const TestSuite &) = delete;
    TestSuite & operator = (TestSuite &&) = delete;
    
    void start_series(const char *);
    void test(TestAssertion (*test_func)());
    void test(TestAssertion (*test_func)(TestFunc));
    void finish_up();
    bool has_successes_only() const;
private:
    template <typename Func>
    void do_test_back(Func &&);
    int m_test_count;
    int m_test_successes;
    std::ostream * m_out;
};

} // end of ts namespace
