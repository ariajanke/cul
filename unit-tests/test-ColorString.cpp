#ifndef MACRO_ARIAJANKE_CUL_COLORSTRING_STATIC_ASSERT_TESTS
#   define MACRO_ARIAJANKE_CUL_COLORSTRING_STATIC_ASSERT_TESTS
#endif
#ifndef MACRO_ARIAJANKE_CUL_ENABLE_TYPELIST_TESTS
#   define MACRO_ARIAJANKE_CUL_ENABLE_TYPELIST_TESTS
#endif
#ifndef MACRO_ARIAJANKE_CUL_ENABLE_TYPESET_TESTS
#   define MACRO_ARIAJANKE_CUL_ENABLE_TYPESET_TESTS
#endif

#include <ariajanke/cul/ColorString.hpp>
#include <ariajanke/cul/TypeList.hpp>
#include <ariajanke/cul/TypeSet.hpp>

#include <iostream>

namespace cul {

class ColorStringHelpersTests {
    ColorStringHelpersTests() {}
#   ifdef MACRO_ARIAJANKE_CUL_COLORSTRING_STATIC_ASSERT_TESTS
    static_assert(ColorStringHelpers::len_of("hello") == 5, "");
    static_assert(ColorStringHelpers::component_string_offset<ColorString::k_red>(4) == 1, "");
    static_assert(ColorStringHelpers::make_to_u8<0, 0>()("1") == 0x11, "");
    static_assert(ColorStringHelpers::portion_val(1, 2, 8) == 4, "");
    static_assert(ColorStringHelpers::portion_val(1, 2, 0x88) == 0x44, "");
    static_assert(ColorStringHelpers::portion_val( 1, 2, ColorString{"#888"}.green().as_u8() ) == 0x44, "");
    static_assert([] {
        auto s = "0000010";
        return ColorStringHelpers::any_of(s, s + 6, [](char c) { return c == '1'; });
    }(), "");
#   endif
};

} // end of cul namespace

namespace {

using namespace cul;


#ifdef MACRO_ARIAJANKE_CUL_COLORSTRING_STATIC_ASSERT_TESTS
// constructors, methods tend to be "tied" together
static_assert(ColorString{} == "#FFF", "");
static_assert(ColorString{}.length() == 4, "");
static_assert(ColorString{"#777"}.length() == 4, "");
static_assert(ColorString{"4", "5", "6"} == "#456", "");
static_assert(ColorString{"99", "55", "33"} == "#995533", "");
static_assert(ColorString{"4", "65", "6"} == "#446566", "");
static_assert(ColorString{"4", "65", "6", "F"} == "#446566FF", "");
static_assert(ColorString{0x11, 0x22, 0x33}       == "#123", "");
static_assert(ColorString{0x11, 0x22, 0x33, 0xFF} == "#123", "");
static_assert(ColorString{0x11, 0x22, 0x33, 0x88} == "#1238", "");
static_assert(ColorString{0x66, 0x23, 0x33}       == "#662333", "");
static_assert(ColorString{0x66, 0x23, 0x33, 0xFF} == "#662333", "");
static_assert(ColorString{0x66, 0x23, 0x33, 0x01} == "#66233301", "");

static_assert((ColorString{} = "#777").length() == 4, "");
static_assert((ColorString{} = ColorString{"#777"}).length() == 4, "");

static_assert(ColorString{"#102058"}.to_rgba_u32() == 0x102058FF, "");

static_assert(ColorString{"#55FF22"}.new_length(4) == "#5F2", "");
static_assert(ColorString{"#55FF22"}.new_length(5, 0x18) == "#5F22", "");
static_assert(ColorString{"#F92"}.new_length(7, 0x55) == "#FF9922", "");
static_assert(ColorString{"#F92"}.new_length(9, 0x55) == "#FF992255", "");

static_assert(ColorString{"#777"}.red().length() == 1, "");
static_assert(   ColorString{"#678"}.red().as_string()[0] == '6'
              && ColorString{"#678"}.red().as_string()[1] == '\0', "");

static_assert(ColorString{"#777"}.red().as_u8() == 0x77, "");
static_assert(ColorString{"#123456"}.red().length() == 2, "");
static_assert(ColorString{"#123456"}.red  ().as_u8() == 0x12, "");
static_assert(ColorString{"#123456"}.green().as_u8() == 0x34, "");
static_assert(ColorString{"#123456"}.blue ().as_u8() == 0x56, "");
static_assert(ColorString{"#123456"}.alpha().as_u8() == 0xFF, "");
static_assert(ColorString{"#888"}.green().replace_with("4").green().as_u8() == 0x44, "");
static_assert(ColorString{"#888"}.green().as_u8() == 0x88, "");
static_assert(ColorString{"#888"}.green().replace_with("4").green().as_u8() == 0x44, "");
static_assert(ColorString{"#888"}.green().portion(1, 2).green().as_u8() == 0x44, "");
static_assert(ColorString{"#AEC"}.green().as_u8() == 0xEE, "");
static_assert(ColorString{"#cbe"}.green().as_u8() == 0xBB, "");

static_assert(ColorString{"#888"} > ColorString{"#777"}, "");
static_assert(ColorString{"#EF9"} == "#EF9", "");
#endif

}

int main() {
    std::cout
        << "[ColorString] [TypeList] [TypeSet]\n"
           "This program compiling successfully, is indication that all unit "
           "tests have passed." << std::endl;
    return 0;
}
