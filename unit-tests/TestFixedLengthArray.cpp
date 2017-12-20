/****************************************************************************

    File: TestFixedLengthArray.cpp
    Author: Andrew "Kip" Janke
    License: GPLv3

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*****************************************************************************/

#include <common/FixedLengthArray.hpp>

#include <iostream>

namespace {

struct CountedType {
    CountedType(): number(++inst_created_so_far) { ++counter; }
    ~CountedType() { --counter; }
    static int counter;
    int number;
    static int inst_created_so_far;
};

/* static */ int CountedType::counter = 0;
/* static */ int CountedType::inst_created_so_far = -1;

struct Simple {
    Simple(): c(' ') {}
    Simple(const Simple &) = default;
    char c;
};

using Error = std::runtime_error;

const char * const TEST_FAILED_MSG =
    "A test has failed for fixed length arrays.\n"
    "This indicates that the application is not ready to run and that the "
    "object will need to be fixed and debugged.";

void run_fixed_length_array_tests(std::ostream & out);

} // end of <anonymous> namespace

int main() {
    run_fixed_length_array_tests(std::cout);
}

namespace {

void run_fixed_length_array_tests(std::ostream & out) {
    out << " --- Fixed Length Array Test Suite --- \n"
           "If you are seeing this in a \"release\" build, something went "
           "horribly wrong in the compliation process (preprocessors were not "
           "properly defined)." << std::endl;
    // test 01, construct and destructer
    {
        {
        FixedLengthArray<CountedType, 5> test_array;
        (void)test_array;
        if (5 != CountedType::counter) {
            out << "[ FAILED ] TEST 01" << std::endl;
            throw Error(TEST_FAILED_MSG);
        }
        }
    if (0 != CountedType::counter) {
        out << "[ FAILED ] TEST 01" << std::endl;
        throw Error(TEST_FAILED_MSG);
    }
    }
    out << "[ PASSED ] TEST 01" << std::endl;
    // test 02, copying
    {
    FixedLengthArray<Simple, 5> target;
    FixedLengthArray<Simple, 5> source;
    for (Simple & sim : source)
        sim.c = '!';
    target = source;
    const FixedLengthArray<Simple, 5> & const_target = target;
    for (const Simple & sim : const_target) {
        if (sim.c != '!') {
            out << "[ FAILED ] TEST 02";
            throw Error(TEST_FAILED_MSG);
        }
    }
    }
    out << "[ PASSED ] TEST 02" << std::endl;
}

} // end of <anonymous> namespace
