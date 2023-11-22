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

#include <ariajanke/cul/TreeTestSuite.hpp>
#include <ariajanke/cul/detail/either-helpers.hpp>
#include <ariajanke/cul/OptionalEither.hpp>
#include <ariajanke/cul/HashMap.hpp>

#include <memory>
#include <set>

namespace {

using namespace cul;
using namespace tree_ts;

#define mark_it mark_source_position(__LINE__, __FILE__).it

template <typename T>
using UniquePtr = std::unique_ptr<T>;

template <typename T>
using SharedPtr = std::shared_ptr<T>;

using std::make_shared, std::make_unique;

struct A final {
public:
    static const int & instance_count() { return s_instance_count; }

    static const int & copy_count() { return s_copy_count; }

    static void reset_counts()
        { s_instance_count = s_copy_count = 0; }

    A() { ++s_instance_count; }

    A(const A &) {
        ++s_copy_count;
        ++s_instance_count;
    }

    A(A &&) { ++s_instance_count; }

    A & operator = (const A &) {
        ++s_copy_count;
        return *this;
    }

    A & operator = (A &&) { return *this; }

    ~A() { --s_instance_count; }

    int id() const { return m_id; }

private:
    static int s_id_counter;
    static int s_instance_count;
    static int s_copy_count;
    int m_id = s_id_counter++;
};

class B final {};

/* private static */ int A::s_id_counter = 0;
/* private static */ int A::s_instance_count = 0;
/* private static */ int A::s_copy_count = 0;

} // end of <anonymous> namespace

auto x = [] {

describe("HashMap")([] {
    A::reset_counts();
    HashMap<SharedPtr<B>, A> hmap{nullptr};
    mark_it("initial capacity is zero", [&] {
        return test_that(hmap.capacity() == 0);
    });
});

describe("HashMap#emplace")([] {
    // sanity tests
    A::reset_counts();
    HashMap<SharedPtr<B>, A> hmap{nullptr};
    hmap.reserve(3);
    auto somekey = make_shared<B>();
    hmap.emplace(somekey, A{});
    hmap.emplace(make_shared<B>(), A{});
    auto insrt = hmap.emplace(make_shared<B>(), A{});
    ;
    mark_it("emplacing three elements, report a size of three", [&] {
        return test_that(hmap.size() == 3);
    }).
    mark_it("emplacing three elements, creates three instances of A", [&] {
        return test_that(A::instance_count() == 3);
    }).
    mark_it("emplacing three elements, never need to copy an instance", [&] {
        return test_that(A::copy_count() == 0);
    }).
    mark_it("insertion object success false if element already exist", [&] {
        return test_that(false);
    }).
    mark_it("throws an invalid_argument when inserting empty key", [&] {
        return expect_exception<std::invalid_argument>([&] {
            hmap.emplace(nullptr, A{});
        });
    });
});

describe("HashMap#extract")([] {
    // sanity test
});

describe("HashMap#find")([] {
    // sanity test
});

describe("HashMap#rehash")([] {
    // no copying elements OR keys >:3!!
});


describe("HashMap iterators")([] {
    // sanity test
    // (no bubbles!)
    A::reset_counts();
    HashMap<SharedPtr<B>, A> hmap{nullptr};
    hmap.reserve(3);
    A a, b, c;
    std::set<int> ids;
    for (auto * a_ : { &a, &b, &c }) {
        ids.insert(a_->id());
        hmap.emplace(make_shared<B>(), std::move(*a_));
    }
    using PairWrapper = decltype(*hmap.begin());

    mark_it("never iterate a pair with an empty key", [&] {
        return test_that(std::all_of
            (hmap.begin(), hmap.end(),
             [](const PairWrapper & pair) { return pair->first != nullptr; }));
    }).
    mark_it("finds every instance saved to the map", [&] {
        for (auto itr = hmap.begin(); itr != hmap.end(); ++itr) {
            ids.find(itr->second.id());
        }
        return test_that(ids.empty());
    });
});

return [] {};

} ();

int main() { return cul::tree_ts::run_tests(); }
