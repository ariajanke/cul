#include <SFML/System/Vector2.hpp>

#include <glm/vec2.hpp>

#include <common/Vector2.hpp>
#include <common/Vector2Util.hpp>
#include <common/SfmlVectorTraits.hpp>

#include <iostream>

namespace cul {

template <>
struct Vector2Scalar<glm::vec2> {
    using Type = glm::vec2::value_type;
};

template <>
struct Vector2Traits<glm::vec2::value_type, glm::vec2> {
    using Scal = glm::vec2::value_type;

    static constexpr const bool k_is_vector_type          = true ;
    static constexpr const bool k_should_define_operators = false;

    struct GetX {
        Scal   operator () (const glm::vec2 & r) const { return r.x; }
        Scal & operator () (      glm::vec2 & r) const { return r.x; }
    };
    struct GetY {
        Scal   operator () (const glm::vec2 & r) const { return r.y; }
        Scal & operator () (      glm::vec2 & r) const { return r.y; }
    };
};

} // end of cul namespace

struct A {};

int main() {
    {
    sf::Vector2i a { 50, 20 }, b;
    a + b;
    std::cout << std::boolalpha;
    std::cout << cul::k_is_convertible_vector2<A> << std::endl;
    std::cout << cul::k_is_convertible_vector2<sf::Vector2<int>> << std::endl;
    std::cout << cul::k_is_convertible_vector2<glm::vec2> << std::endl;
    std::cout << cul::k_is_convertible_vector2<glm::vec3> << std::endl;
    std::cout << cul::k_is_convertible_vector2<cul::Vector2<int>> << std::endl;
    std::cout << cul::magnitude(a) << std::endl;

    glm::vec2 c { 4.5, -1.2 };
    glm::vec2 d { 10, -1 };
    c = cul::normalize(c);
    c = cul::project_onto(c, d);
    std::cout << c.x << ", " << c.y << std::endl;
    std::cout << cul::magnitude(c) << std::endl;
    }
    {
    glm::vec2 a {  0,  0 };
    glm::vec2 b { 10, 10 };
    glm::vec2 c {  4,  0 };
    auto cpt = cul::find_closest_point_to_line(a, b, c);
    std::cout << cpt.x << ", " << cpt.y << std::endl;
    }
    {
    // check if operators are okay
    using VecD = cul::Vector2<double>;
    VecD a, b, c;
    (void)(a + b);
    (void)(a - c);
    a += b;
    a -= c;
    (void)-a;
    double s = 10.;
    a*s;
    a/s;
    a *= s;
    a /= s;
    (void)(a == b);
    (void)(a != c);
    }
    {
    // check if operators are okay
    using VecD = cul::Size2<double>;
    VecD a, b, c;
    (void)(a + b);
    (void)(a - c);
    a += b;
    a -= c;
    (void)-a;
    double s = 10.;
    a*s;
    a/s;
    a *= s;
    a /= s;
    (void)(a == b);
    (void)(a != c);
    }
    {
    using RectD = cul::Rectangle<double>;
    using VecD  = cul::Vector2<double>;
    using SizeD = cul::Size2<double>;
    RectD rect(VecD(10, 90), SizeD(78, 10));
    [[maybe_unused]] auto [r, sz] = cul::decompose(rect);
    [[maybe_unused]] auto b = rect;
    (void)r;
    (void)sz;
    }

    // run through each function, once with another library's vector type
    // and once with the internal vector type
    using VecD = cul::Vector2<double>;
    using VecF = cul::Vector2<float>;
    using VecI = cul::Vector2<int>;
    using cul::convert_to;
    {
        auto a = cul::magnitude(VecI(100, 190));
        auto b = cul::magnitude(sf::Vector2i(100, 190));
        assert(a == b);
    }
    {
        auto a = cul::normalize(VecF(-89.f, 10.f));
        auto b = cul::normalize(glm::vec2(-89.f, 10.f));
        // their values should actually be exactly equal
        assert(a == convert_to<VecF>(b));
        auto abm_a = cul::are_within(a, convert_to<VecF>(b), 0.01f);
        auto abm_b = cul::are_within(convert_to<glm::vec2>(a), b, 0.1f);
        assert(abm_a && abm_b);
    }
    {
        static constexpr const auto k_pi =  cul::k_pi_for_type<float>;
        VecF a(5, 4);
        sf::Vector2f b(6, 7);
        a = cul::rotate_vector(a,  k_pi*0.5f );
        b = cul::rotate_vector(b, -k_pi*0.25f);
        std::cout << "vector (5, 4) rotated " << a.x << ", " << a.y
                  << "\nvector (6, 7) rotated " << b.x << ", " << b.y << std::endl;
    }
    {
        VecI a(5, 4), b(-2, 3);
        glm::vec2 c(6.5, 7.25), d(8.9, -1.4);
        [[maybe_unused]] int a_dot = dot(a, b);
        [[maybe_unused]] double c_dot = cul::dot(c, d);
        [[maybe_unused]] int a_cross = cross(a, b);
        [[maybe_unused]] double c_cross = cul::cross(c, d);
        VecD e(5, 4), f(-2, 3);
        auto ef_ang = cul::angle_between(e, f);
        auto ef_dang = cul::directed_angle_between(e, f);
        assert(cul::magnitude(ef_ang - cul::magnitude(ef_dang)) < 0.005);

        [[maybe_unused]] auto cd_ang  = cul::angle_between(c, d);
        [[maybe_unused]] auto cd_dang = cul::directed_angle_between(c, d);
    }
    {
        VecD a(100, 45), b(-1, 0.1);
        glm::vec2 c(6.5, 7.25), d(8.9, -1.4);
        auto abp = cul::project_onto(a, b);
        auto cdp = cul::project_onto(c, d);
        std::cout << "vector projection abp: " << abp.x << ", " << abp.y
                  << "\n projection cdp: " << cdp.x << ", " << cdp.y << std::endl;
    }
    {
        VecD ai(0, 0), bi(1, 1), ci(0, 1), di(1, 0);
        auto intxi = cul::find_intersection(ai, bi, ci, di);
        sf::Vector2f ao(0, 0), bo(1, 1), co(0, 1), do_(1, 0);
        auto intxo = cul::find_intersection(ao, bo, co, do_);
        std::cout << "vector intersection i: " << intxi.x << ", " << intxi.y
                  << "\n intersection o: " << intxo.x << ", " << intxo.y << std::endl;
    }
    // will need tests for round_to
    cul::round_to<int>(VecF(10.5, 11.3));
    {
        VecD s(34, 12), t(100, 20), g(0, -10);
        double speed = 100;
        auto [s0, s1] = cul::find_velocities_to_target(s, t, g, speed);
        auto so = convert_to<glm::vec2>(s);
        auto to = convert_to<glm::vec2>(t);
        auto go = convert_to<glm::vec2>(g);
        auto [s0o, s1o] = cul::find_velocities_to_target(so, to, go, float(speed));
        assert(cul::magnitude(s0 - convert_to<VecD>(s0o)) < 0.0005);
        assert(cul::magnitude(s1 - convert_to<VecD>(s1o)) < 0.0005);
    }
    {
        VecD a(5, 4), b(10, 12), c(7, 5);
        auto d = cul::find_closest_point_to_line(a, b, c);
        sf::Vector2f ao(-2, 3), bo(4, -3), co(0, 0);
        auto do_ = cul::find_closest_point_to_line(ao, bo, co);
        std::cout << "vector closest point i: " << d.x << ", " << d.y
                  << "\n closest point o: " << do_.x << ", " << do_.y << std::endl;
    }
    {
        std::cout << std::boolalpha
                  << cul::is_real(sf::Vector2f(900, 5.12e26f)) << " "
                  << cul::is_real(sf::Vector2i(1, 1)) << " "
                  << cul::is_real(cul::get_no_solution_sentinel<VecD>()) << std::endl;
    }
    return 0;
}
