#include <common/Grid.hpp>
#include <common/SubGrid.hpp>
#include <common/TestSuite.hpp>

#include <iostream>

#include <cassert>

using ts::TestSuite;
using VectorI = sf::Vector2i;

int main() {
    // I need "call everything" tests for any template class!
    
    TestSuite tsuite;
    tsuite.start_series("Grid tests");
    tsuite.test([]() {
        Grid<int> g;
        g.set_size(1, 1);
        g.set_size(2, 3, 10);
        return ts::test(g(1, 1) == 10);
    });
    tsuite.test([]() {
        Grid<int> g;
        g.set_size(2, 3, 10);
        int count = 0;
        for (auto i : g) {
            (void)i;
            ++count;
        }
        return ts::test(int(g.size()) == count);
    });
    tsuite.test([]() {
        Grid<int> g;
        g.set_size(8, 8);
        Grid<int>::ConstReferenceType e = g(3, 4);
        auto gv = g.position_of(e);
        std::cout << g.width() << "x" << g.height() << std::endl;
        std::cout << gv.x << ", " << gv.y << std::endl;
        return ts::test(g.position_of(e) == sf::Vector2i(3, 4));
    });
    tsuite.test([]() {
        Grid<int> g;
        g.set_size(4, 3);
        assert(!g.is_empty());
        auto e = g.begin() + 5;
        *e = 20;
        std::cerr << ((e < g.begin()) ? "t" : "f") << " " << ((e > g.end()) ? "t" : "f") << std::endl;
        auto r = g.position_of(e);
        std::cout << r.x << ", " << r.y << std::endl;
        return ts::test(g(r) == 20);
    });
    // nothing to test outside of, it should compile
    {
    Grid<int> g;
    static constexpr const int k_res_size = 3;
    g.set_size(3, 3, k_res_size);
    g.set_size(4, 5, []() { return 2; }());
    
    g.set_width(3, k_res_size);
    g.set_width(4, []() { return 2; }());
    
    g.set_height(3, k_res_size);
    g.set_height(4, []() { return 2; }());
    
    }
    
    tsuite.test([]() {
        Grid<int> p;
        auto subg = make_sub_grid(p);
        const auto & cp = p;
        auto csg  = make_sub_grid(cp);
        
        return ts::test(&p == &subg.parent() && &p == &csg.parent());
    });
    
    {
    Grid<int> p;
    p.set_size(3, 3);
        {
        // Grid -> SubGrid
        auto a = make_sub_grid(p);
        auto b = make_sub_grid(p, VectorI(1, 2));
        auto c = make_sub_grid(p, 2);
        auto d = make_sub_grid(p, 2, 2);
        auto e = make_sub_grid(p, VectorI(1, 1), 1);
        auto f = make_sub_grid(p, VectorI(1, 1), 1, 1);
        static_assert(std::is_same_v<SubGrid<int>, decltype(a)>, "");
        static_assert(std::is_same_v<SubGrid<int>, decltype(b)>, "");
        static_assert(std::is_same_v<SubGrid<int>, decltype(c)>, "");
        static_assert(std::is_same_v<SubGrid<int>, decltype(d)>, "");
        static_assert(std::is_same_v<SubGrid<int>, decltype(e)>, "");
        static_assert(std::is_same_v<SubGrid<int>, decltype(f)>, "");
        }
        {
        // const Grid & -> ConstSubGrid
        const auto & cref_p = p;
        auto a = make_sub_grid(cref_p);
        auto b = make_sub_grid(cref_p, VectorI(1, 2));
        auto c = make_sub_grid(cref_p, 2);
        auto d = make_sub_grid(cref_p, 2, 2);
        auto e = make_sub_grid(cref_p, VectorI(1, 1), 1);
        auto f = make_sub_grid(cref_p, VectorI(1, 1), 1, 1);
        static_assert(std::is_same_v<ConstSubGrid<int>, decltype(a)>, "");
        static_assert(std::is_same_v<ConstSubGrid<int>, decltype(b)>, "");
        static_assert(std::is_same_v<ConstSubGrid<int>, decltype(c)>, "");
        static_assert(std::is_same_v<ConstSubGrid<int>, decltype(d)>, "");
        static_assert(std::is_same_v<ConstSubGrid<int>, decltype(e)>, "");
        static_assert(std::is_same_v<ConstSubGrid<int>, decltype(f)>, "");
        }
        {
        // Grid & -> ConstSubGrid
        auto a = make_const_sub_grid(p);
        auto b = make_const_sub_grid(p, VectorI(1, 2));
        auto c = make_const_sub_grid(p, 2);
        auto d = make_const_sub_grid(p, 2, 2);
        auto e = make_const_sub_grid(p, VectorI(1, 1), 1);
        auto f = make_const_sub_grid(p, VectorI(1, 1), 1, 1);
        static_assert(std::is_same_v<ConstSubGrid<int>, decltype(a)>, "");
        static_assert(std::is_same_v<ConstSubGrid<int>, decltype(b)>, "");
        static_assert(std::is_same_v<ConstSubGrid<int>, decltype(c)>, "");
        static_assert(std::is_same_v<ConstSubGrid<int>, decltype(d)>, "");
        static_assert(std::is_same_v<ConstSubGrid<int>, decltype(e)>, "");
        static_assert(std::is_same_v<ConstSubGrid<int>, decltype(f)>, "");
        }
        {
        const auto & cref_p = p;
        auto a = make_const_sub_grid(cref_p);
        auto b = make_const_sub_grid(cref_p, VectorI(1, 2));
        auto c = make_const_sub_grid(cref_p, 2);
        auto d = make_const_sub_grid(cref_p, 2, 2);
        auto e = make_const_sub_grid(cref_p, VectorI(1, 1), 1);
        auto f = make_const_sub_grid(cref_p, VectorI(1, 1), 1, 1);
        static_assert(std::is_same_v<ConstSubGrid<int>, decltype(a)>, "");
        static_assert(std::is_same_v<ConstSubGrid<int>, decltype(b)>, "");
        static_assert(std::is_same_v<ConstSubGrid<int>, decltype(c)>, "");
        static_assert(std::is_same_v<ConstSubGrid<int>, decltype(d)>, "");
        static_assert(std::is_same_v<ConstSubGrid<int>, decltype(e)>, "");
        static_assert(std::is_same_v<ConstSubGrid<int>, decltype(f)>, "");
        }
        // ------------------------------ SubGrid ------------------------------
        // make_sub_grid      : SubGrid      -> SubGrid
        //                      ConstSubGrid -> ConstSubGrid
        // make_const_sub_grid: SubGrid      -> ConstSubGrid
        //                      ConstSubGrid -> ConstSubGrid
        {
        auto subg = make_sub_grid(p);
        auto a = make_sub_grid(subg);
        auto b = make_sub_grid(subg, VectorI(1, 2));
        auto c = make_sub_grid(subg, 2);
        auto d = make_sub_grid(subg, 2, 2);
        auto e = make_sub_grid(subg, VectorI(1, 1), 1);
        auto f = make_sub_grid(subg, VectorI(1, 1), 1, 1);
        static_assert(std::is_same_v<SubGrid<int>, decltype(a)>, "");
        static_assert(std::is_same_v<SubGrid<int>, decltype(b)>, "");
        static_assert(std::is_same_v<SubGrid<int>, decltype(c)>, "");
        static_assert(std::is_same_v<SubGrid<int>, decltype(d)>, "");
        static_assert(std::is_same_v<SubGrid<int>, decltype(e)>, "");
        static_assert(std::is_same_v<SubGrid<int>, decltype(f)>, "");
        }
        {
        auto subg = make_const_sub_grid(p);
        auto a = make_sub_grid(subg);
        auto b = make_sub_grid(subg, VectorI(1, 2));
        auto c = make_sub_grid(subg, 2);
        auto d = make_sub_grid(subg, 2, 2);
        auto e = make_sub_grid(subg, VectorI(1, 1), 1);
        auto f = make_sub_grid(subg, VectorI(1, 1), 1, 1);
        static_assert(std::is_same_v<ConstSubGrid<int>, decltype(a)>, "");
        static_assert(std::is_same_v<ConstSubGrid<int>, decltype(b)>, "");
        static_assert(std::is_same_v<ConstSubGrid<int>, decltype(c)>, "");
        static_assert(std::is_same_v<ConstSubGrid<int>, decltype(d)>, "");
        static_assert(std::is_same_v<ConstSubGrid<int>, decltype(e)>, "");
        static_assert(std::is_same_v<ConstSubGrid<int>, decltype(f)>, "");
        }
        {
        auto subg = make_sub_grid(p);
        auto a = make_const_sub_grid(subg);
        auto b = make_const_sub_grid(subg, VectorI(1, 2));
        auto c = make_const_sub_grid(subg, 2);
        auto d = make_const_sub_grid(subg, 2, 2);
        auto e = make_const_sub_grid(subg, VectorI(1, 1), 1);
        auto f = make_const_sub_grid(subg, VectorI(1, 1), 1, 1);
        static_assert(std::is_same_v<ConstSubGrid<int>, decltype(a)>, "");
        static_assert(std::is_same_v<ConstSubGrid<int>, decltype(b)>, "");
        static_assert(std::is_same_v<ConstSubGrid<int>, decltype(c)>, "");
        static_assert(std::is_same_v<ConstSubGrid<int>, decltype(d)>, "");
        static_assert(std::is_same_v<ConstSubGrid<int>, decltype(e)>, "");
        static_assert(std::is_same_v<ConstSubGrid<int>, decltype(f)>, "");
        }
        {
        auto subg = make_const_sub_grid(p);
        auto a = make_const_sub_grid(subg);
        auto b = make_const_sub_grid(subg, VectorI(1, 2));
        auto c = make_const_sub_grid(subg, 2);
        auto d = make_const_sub_grid(subg, 2, 2);
        auto e = make_const_sub_grid(subg, VectorI(1, 1), 1);
        auto f = make_const_sub_grid(subg, VectorI(1, 1), 1, 1);
        static_assert(std::is_same_v<ConstSubGrid<int>, decltype(a)>, "");
        static_assert(std::is_same_v<ConstSubGrid<int>, decltype(b)>, "");
        static_assert(std::is_same_v<ConstSubGrid<int>, decltype(c)>, "");
        static_assert(std::is_same_v<ConstSubGrid<int>, decltype(d)>, "");
        static_assert(std::is_same_v<ConstSubGrid<int>, decltype(e)>, "");
        static_assert(std::is_same_v<ConstSubGrid<int>, decltype(f)>, "");
        }
        // need to test const ref to sub grid
    }
    
    return 0;
}
