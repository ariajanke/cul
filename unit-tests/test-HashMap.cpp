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
#include <ariajanke/cul/HashMap.hpp>

#include <memory>
#include <set>

namespace {

using namespace cul;
using namespace tree_ts;

#ifdef MACRO_HASHMAP_TEMPLATES
#   error "must not define MACRO_HASHMAP_TEMPLATES"
#endif
#ifdef MACRO_HASHMAP_CLASSNAME
#   error "must not define MACRO_HASHMAP_CLASSNAME"
#endif

#ifdef MACRO_ITERATOR_TEMPLATES
#   error "must not define MACRO_ITERATOR_TEMPLATES"
#endif

#ifdef MACRO_ITERATOR_CLASSNAME
#   error "must not define MACRO_ITERATOR_CLASSNAME"
#endif

#define mark_it mark_source_position(__LINE__, __FILE__).it

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

    A(const A & a_): m_id(a_.m_id) {
        ++s_copy_count;
        ++s_instance_count;
    }

    A(A && a_): m_id(a_.m_id) { ++s_instance_count; }

    A & operator = (const A & a_) {
        ++s_copy_count;
        m_id = a_.m_id;
        return *this;
    }

    A & operator = (A && a_) {
        m_id = a_.m_id;
        a_.m_id = -1;
        return *this;
    }

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

static constexpr const std::size_t empty_key = 0;
static constexpr const std::size_t a_key = 1;
static constexpr const std::size_t b_key = 2;
static constexpr const std::size_t c_key = 3;
static constexpr const std::size_t d_key = b_key + 8;

struct Emplace final {};
struct Reserve final {};
struct Find final {};
struct Insert final {};
struct Extract final {};
struct Rehash final {};
struct Iterators final {};
struct Clear final {};
struct Move final {};

describe<Reserve>("HashMap#reserve")([] {
    A::reset_counts();
    constexpr const static std::size_t k_empty_key = -1;
    HashMap<std::size_t, A> hmap{k_empty_key};
    mark_it("initial capacity is zero", [&] {
        return test_that(hmap.capacity() == 0);
    }).
    next([&] {
        hmap.reserve(4);
    }).
    mark_it("has no nothing to iterate", [&] {
        int count = 0;
        for (auto itr = hmap.begin(); itr != hmap.end(); ++itr) {
            ++count;
        }
        return test_that(count == 0);
    }).
    mark_it("reserve less space than current capacity, does not change "
            "capacity", [&]
    {
        auto old_bucket_count = hmap.bucket_count();
        hmap.reserve(1);
        return test_that(old_bucket_count == hmap.bucket_count());
    });
});

describe<Emplace>("HashMap#emplace").depends_on<Reserve>()([] {
    A::reset_counts();
    HashMap<SharedPtr<B>, A> hmap{nullptr};
    hmap.reserve(3);
    auto somekey = make_shared<B>();
    hmap.emplace(SharedPtr<B>{somekey}, A{});
    hmap.emplace(make_shared<B>(), A{});
    hmap.emplace(make_shared<B>(), A{});
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
        auto insrt = hmap.insert(somekey, A{});
        return test_that(!insrt.success);
    }).
    mark_it("throws an invalid_argument when inserting empty key", [&] {
        return expect_exception<std::invalid_argument>([&] {
            hmap.emplace(nullptr, A{});
        });
    });
});

describe<Clear>("HashMap#clear").depends_on<Iterators>()([] {
    A::reset_counts();
    HashMap<SharedPtr<B>, A> hmap{nullptr};
    hmap.reserve(3);
    hmap.emplace(make_shared<B>(), A{});
    hmap.emplace(make_shared<B>(), A{});
    hmap.clear();
    mark_it("sets size to zero", [&] {
        return test_that(hmap.is_empty());
    }).
    mark_it("iterates no elements", [&] {
        int i = 0;
        for (auto itr = hmap.begin(); itr != hmap.end(); ++itr) {
            ++i;
        }
        return test_that(i == 0);
    }).
    mark_it("destroys all instances of elements", [] {
        return test_that(A::instance_count() == 0);
    });
});

describe<Move>("HashMap move").depends_on<Iterators>()([] {
    A a, b, c;
    HashMap<std::size_t, A> hmap{empty_key};
    hmap.insert(a_key, a);
    hmap.insert(b_key, b);
    hmap.insert(c_key, c);
    std::set<std::size_t> keys = { a_key, b_key, c_key };
    mark_it("moving preserves elements", [&] {
        auto map = std::move(hmap);
        for (auto [key, el] : map) {
            keys.erase(key);
        }
        return test_that(keys.empty());
    }).
    mark_it("moving iterators preseves okay", [&] {
        auto beg = hmap.begin();
        auto key = beg->first;
        auto beg2 = std::move(beg);
        return test_that(beg2->first == key);
    });
});

describe<Insert>("HashMap#insert").depends_on<Emplace>()([] {
    mark_it("does not consume the key passed", [] {
        HashMap<SharedPtr<B>, A> hmap{nullptr};
        hmap.reserve(3);
        auto somekey = make_shared<B>();
        hmap.insert(somekey, A{});
        return test_that(static_cast<bool>(somekey));
    });
});

describe<Find>("HashMap#find").depends_on<Emplace>()([] {
    A::reset_counts();
    HashMap<std::size_t, A> hmap{empty_key};
    hmap.reserve(4);
    A a, b, c, d;
    hmap.insert(a_key, a);
    hmap.insert(b_key, b);
    hmap.insert(c_key, c);
    hmap.insert(d_key, d);
    assert(hmap.find(b_key)->second.id() != hmap.find(d_key)->second.id());
    mark_it("finding by empty key, returns end iterator", [&] {
        return test_that(hmap.find(empty_key) == hmap.end());
    }).
    mark_it("finds element by key", [&] {
        return test_that(hmap.find(b_key)->second.id() == b.id());
    }).
    mark_it("correctly finds element with conflicting index", [&] {
        return test_that(hmap.find(d_key)->second.id() == d.id());
    });
});

describe<Extract>("HashMap#extract").depends_on<Find>()([] {
    A::reset_counts();
    HashMap<std::size_t, A> hmap{empty_key};
    hmap.reserve(4);
    A a, b, c, d;
    hmap.insert(a_key, a);
    hmap.insert(b_key, b);
    hmap.insert(c_key, c);
    assert(hmap.bucket_count() == 8);
    mark_it("can succesfully extract an element", [&] {
        auto ex = hmap.extract(hmap.find(b_key));
        return test_that(ex.key != empty_key &&
                         ex.element.id() == b.id());
    }).
    mark_it("extracts an element by moving it's contents out", [&] {
        auto old_copy_count = A::copy_count();
        auto ex = hmap.extract(hmap.find(b_key));
        return test_that(ex.element.id() == b.id() &&
                         old_copy_count == A::copy_count());
    }).
    mark_it("returns an extraction whose iterator points to the next element", [&] {
        std::set<std::size_t> keys = { a_key, b_key, c_key };
        for (auto itr = hmap.begin(); itr != hmap.end(); ) {
            keys.erase(itr->first);
            if (itr->first == b_key) {
                itr = hmap.erase(itr);
            } else {
                ++itr;
            }
        }
        return test_that(keys.empty());
    }).
    next([&] {
        hmap.insert(d_key, d);
    }).
    mark_it("following an extraction/suffled around elements, the shuffled "
            "element is still covered within the iterator range", [&]
    {
        auto ex = hmap.extract(hmap.find(b_key));
        bool found_d = false;
        for (auto itr = ex.next; itr != hmap.end(); ++itr) {
            if (itr->second.id() == d.id())
                found_d = true;
        }
        return test_that(found_d);
    }).
    mark_it("trying to extract from the end will throw an invalid argument "
            "error", [&]
    {
        return expect_exception<std::invalid_argument>([&] {
            hmap.erase(hmap.end());
        });
    });
});

describe<Rehash>("HashMap#rehash").depends_on<Emplace>()([] {
    A::reset_counts();
    HashMap<std::size_t, A> hmap{empty_key};
    A a, b, c, d;
    auto add_elements = [&] {
        hmap.insert(a_key, a);
        hmap.insert(b_key, b);
        hmap.insert(c_key, c);
        hmap.insert(d_key, d);
    };
    mark_it("does not copy anything while rehashing", [&] {
        auto old_copy_count = A::copy_count();
        add_elements();
        // Note: insertions force at least four copies
        //       we're interested to see that the rehash itself does not copy
        return test_that(old_copy_count + 4 == A::copy_count());
    }).
    next([&] { add_elements(); }).
    mark_it("insertions causing a rehash, all elements remain accessible", [&] {
        std::set<std::size_t> keys = { a_key, b_key, c_key, d_key };
        for (const auto & [key, element] : hmap) {
            keys.erase(key);
        }
        return test_that(keys.empty());
    });
});

describe<Iterators>("HashMap iterators").depends_on<Emplace>()([] {
    A::reset_counts();
    HashMap<SharedPtr<B>, A> hmap{nullptr};
    const auto & cref_hmap = hmap;
    hmap.reserve(3);
    A a, b, c;
    std::set<int> ids;
    for (auto * a_ : { &a, &b, &c }) {
        ids.insert(a_->id());
        hmap.emplace(make_shared<B>(), std::move(*a_));
    }
    using PairWrapper = decltype(*hmap.begin());

    mark_it("iterators count exactly three elements", [&] {
        int i = 0;
        auto itr = hmap.begin();
        while (itr != hmap.end()) {
            ++i;
            ++itr;
        }
        return test_that(i == 3);
    }).
    mark_it("never iterate a pair with an empty key", [&] {
        auto is_not_empty_key = [&](const PairWrapper & pair)
            { return pair.first != nullptr; };
        return test_that(std::all_of
            (hmap.begin(), hmap.end(), std::move(is_not_empty_key)));
    }).
    mark_it("finds every instance saved to the map", [&] {
        for (auto itr = hmap.begin(); itr != hmap.end(); ++itr) {
            auto & el = itr->second;
            ids.erase(el.id());
        }
        return test_that(ids.empty());
    }).
    mark_it("finds every instance saved to the map, with conflicting ideal "
            "index", []
    {
        static constexpr const std::size_t empty_key = -1;
        HashMap<std::size_t, A> hmap{empty_key};
        hmap.reserve(4);
        A a, b, c, d;
        static constexpr const std::size_t a_key = 0;
        static constexpr const std::size_t b_key = 1;
        static constexpr const std::size_t c_key = 2;
        static constexpr const std::size_t d_key = b_key + 8;
        hmap.insert(a_key, a);
        hmap.insert(b_key, b);
        hmap.insert(c_key, c);
        hmap.insert(d_key, d);
        std::set<int> ids = { a_key, b_key, c_key, d_key };
        for (auto itr = hmap.begin(); itr != hmap.end(); ++itr) {
            auto & key = itr->first;
            ids.erase(key);
        }
        return test_that(ids.empty());
    }).
    mark_it("iterating pairs using for-range, are reference pairs", [&] {
        using RefPair = HashMap<SharedPtr<B>, A>::Iterator::Reference;
        A new_c;
        for (auto pair : hmap) {
            if (pair.second.id() == c.id()) {
                pair.second = new_c;
            }
        }
        auto itr = std::find_if
            (hmap.begin(), hmap.end(),
             [&new_c](const RefPair & pair)
             { return pair.second.id() == new_c.id(); });
        return test_that(itr != hmap.end());
    }).
    mark_it("begin returns a valid key (advances past empty)", [&] {
        HashMap<std::size_t, A> hmap{empty_key};
        hmap.insert(b_key, b);
        return test_that(hmap.begin()->first == b_key);
    });
    static_assert(
        std::is_same_v<decltype(hmap.cbegin()), decltype(cref_hmap.begin())>,
        "cbegin will return the same type for begin on constant map"
    );
    static_assert(
        std::is_same_v<decltype(hmap.cend()), decltype(cref_hmap.end())>,
        "cend will return the same type for begin on constant map"
    );
});

return [] {};

} ();

int main() { return cul::tree_ts::run_tests(); }
