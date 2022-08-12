#ifndef MACRO_ARIAJANKE_CUL_COLORSTRING_STATIC_ASSERT_TESTS
#   define MACRO_ARIAJANKE_CUL_COLORSTRING_STATIC_ASSERT_TESTS
#endif
#ifndef MACRO_ARIAJANKE_CUL_ENABLE_TYPELIST_TESTS
#   define MACRO_ARIAJANKE_CUL_ENABLE_TYPELIST_TESTS
#endif
#ifndef MACRO_ARIAJANKE_CUL_ENABLE_TYPESET_TESTS
#   define MACRO_ARIAJANKE_CUL_ENABLE_TYPESET_TESTS
#endif

#include <common/ColorString.hpp>
#include <common/TypeList.hpp>
#include <common/TypeSet.hpp>

#include <iostream>

int main() {
    std::cout
        << "[ColorString] [TypeList] [TypeSet]\n"
           "This program compiling successfully, is indication that all unit "
           "tests have passed." << std::endl;
    return 0;
}
