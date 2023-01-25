#include <stdexcept>
#include <utility>
#include <random>

#include <cmath>

#include <ariajanke/cul/Util.hpp>
#include <ariajanke/cul/Vector2.hpp>
#include <ariajanke/cul/Vector3.hpp>
#include <ariajanke/cul/VectorUtils.hpp>
#include <ariajanke/cul/sf/VectorTraits.hpp>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace cul {

template <int kt_dimensionality, typename T, glm::qualifier Q>
struct VectorTraits<glm::vec<kt_dimensionality, T, Q>> {
    static constexpr const bool k_is_vector_type          = true;
    static constexpr const bool k_should_define_operators = false;

    using GlmVec = glm::vec<kt_dimensionality, T, Q>;
    using ScalarType = T;

    template <int kt_idx, typename = int>
    struct Get final {};

    template <typename U>
    struct Get<0, U> final {
        GLM_CONSTEXPR ScalarType operator () (const GlmVec & r) const
            { return r.x; }
    };

    template <typename U>
    struct Get<1, U> final {
        GLM_CONSTEXPR ScalarType operator () (const GlmVec & r) const
            { return r.y; }
    };

    template <typename U>
    struct Get<2, U> final {
        GLM_CONSTEXPR ScalarType operator () (const GlmVec & r) const
            { return r.z; }
    };

    template <typename U>
    struct Get<3, U> final {
        GLM_CONSTEXPR ScalarType operator () (const GlmVec & r) const
            { return r.w; }
    };

    struct Make final {
        template <typename ... Types>
        GLM_CONSTEXPR GlmVec operator () (Types && ... comps) const
            { return GlmVec{ std::forward<Types>(comps)... }; }
    };

    template <typename U>
    using ChangeScalarType = glm::vec<kt_dimensionality, U, Q>;

    static constexpr const int k_dimension_count = kt_dimensionality;
};

} // end of cul(n) namespace

struct UserVector final {
    double x = 0, y = 0;
};


struct UserVector3 final {
    double x = 0, y = 0, z = 0;
};

namespace cul {

template <>
struct VectorTraits<UserVector> {
    static constexpr const bool k_is_vector_type          = true;
    static constexpr const bool k_should_define_operators = false;

    using ScalarType = double;

    template <int kt_idx, typename = int>
    struct Get final {};

    template <typename U>
    struct Get<0, U> final {
        ScalarType operator () (const UserVector & r) const
            { return r.x; }
    };

    template <typename U>
    struct Get<1, U> final {
        ScalarType operator () (const UserVector & r) const
            { return r.y; }
    };

    struct Make final {
        UserVector operator () (const double & x, const double & y) const
            { return UserVector{x, y}; }
    };
#   if 0
    template <typename U>
    using ChangeScalarType = ; // not defined...
#   endif
    static constexpr const int k_dimension_count = 2;
};

template <>
struct VectorTraits<UserVector3> {
    static constexpr const bool k_is_vector_type          = true;
    static constexpr const bool k_should_define_operators = false;

    using ScalarType = double;

    template <int kt_idx, typename = int>
    struct Get final {};

    template <typename U>
    struct Get<0, U> final {
        ScalarType operator () (const UserVector3 & r) const
            { return r.x; }
    };

    template <typename U>
    struct Get<1, U> final {
        ScalarType operator () (const UserVector3 & r) const
            { return r.y; }
    };

    template <typename U>
    struct Get<2, U> final {
        ScalarType operator () (const UserVector3 & r) const
            { return r.z; }
    };

    struct Make final {
        UserVector3 operator () (const double & x, const double & y, const double & z) const
            { return UserVector3{x, y, z}; }
    };
#   if 0
    template <typename U>
    using ChangeScalarType = ; // not defined...
#   endif
    static constexpr const int k_dimension_count = 3;
};

} // end of cul(n) namespace


// naughty, naughty...
#include <iostream>
#include <SFML/System/Vector2.hpp>

#include <cassert>

constexpr const auto k_pi_f = cul::k_pi_for_type<float>;
constexpr const auto k_inf = std::numeric_limits<double>::infinity();

int main() {

    using Vec2I = cul::Vector2<int>;
    using Vec2D = cul::Vector2<double>;
    using Vec3I = cul::Vector3<int>;
    using SfVec2F = sf::Vector2f;
    using GlmVec3 = glm::vec3;
    using GlmVec4 = glm::vec4;
    using GlmVec3I = cul::VectorTraits<glm::vec3>::ChangeScalarType<int>;
    constexpr double k_error = 0.00005;

    // operators for builtin type

    static_assert(-Vec2I{1, 2} == Vec2I{-1, -2});
    static_assert(Vec2I{1, 2} + Vec2I{1, 3} == Vec2I{2, 5});
    static_assert(Vec3I{ 3, 4, 5 } - Vec3I{ 2, 2, -1 } == Vec3I{1, 2, 6});
    static_assert(-Vec3I{1, -2, 3} == Vec3I{-1, 2, -3});
    static_assert(Vec3I{ 1, 2, 3 }*2 == Vec3I{2, 4, 6});
    static_assert(Vec3I{ 4, 8, 16 }/2 == Vec3I{2, 4, 8});
    static_assert(2*Vec2I{ 4, -2 } == Vec2I{8, -4});

    using namespace cul;
    static_assert(project_onto( Vec2I{ 3, 4 }, Vec2I{ -3, 0 } ) == Vec2I{3, 0});
    auto res = magnitude( convert_to<sf::Vector2i>(Vec2I{ 3, 4 }) );
    std::cout << "mag of vector <3, 4> = " << res << std::endl;
    {
        auto v = cul::round_to<int>(sf::Vector2f{ 1.2, 5.5 });
        static_assert(std::is_same_v<decltype(v), sf::Vector2i>);
        assert(([v] { return v == sf::Vector2i{1, 6}; } ()));
    }
    cul::find_velocities_to_target(Vec2D{ 1, 1 }, Vec2D{ 11, 5}, Vec2D{ 0, -8 }, 12);
    cul::find_velocities_to_target(SfVec2F{ 1, 1 }, SfVec2F{ 11, 5}, SfVec2F{ 0, -8 }, 12);

    // ---

    // angle_between

    assert(magnitude( angle_between(GlmVec3{-1, 0, 3}, GlmVec3{3, 0, 1}) - k_pi_f*0.5) < k_error);
    std::cout << angle_between(UserVector{-1, -2}, UserVector{1, 2}) << std::endl;
    assert(magnitude( angle_between(UserVector{-1, -2}, UserVector{1, 2}) - k_pi_f) < k_error);
    static_assert(std::is_same_v<decltype(angle_between(UserVector{-1, -2}, UserVector{1, 2})), double>);

    // are_within
    static_assert(are_within( Vec2I{1, 2}, Vec2I{1, 3}, 2));
    assert(are_within( UserVector{1, 2}, UserVector{1, 3}, 2));

    // area_of_triangle
    std::cout << area_of_triangle(Vec2I{0, 0}, Vec2I{10, 10}, Vec2I{10, 0}) << std::endl;
    std::cout << area_of_triangle(Vec3I{0, 0, 0}, Vec3I{10, 10, 0}, Vec3I{10, 0, 0}) << std::endl;
    assert(area_of_triangle(Vec2I{0, 0}, Vec2I{10, 10}, Vec2I{10, 0}) == 50);
    assert(area_of_triangle(UserVector{0, 0}, UserVector{10, 10}, UserVector{10, 0}) == 50);

    // convert_to
    // conversions should work to and from other library types
    static_assert(convert_to<Vec3I>( GlmVec3{ 10.4, 8.7, 9.4 } ) == Vec3I{10, 8, 9});
    assert(magnitude(convert_to<SfVec2F>( Vec2I{9, 8} ) - SfVec2F{9, 8}) < k_error);

    // cross (2D)
    static_assert(cross(Vec2I{ -1, 0 }, Vec2I{ 0, 1 }) == -1);
    assert(cross(UserVector{-1, 0}, UserVector{0, 1}) == -1);

    // cross (3D)
    {
    constexpr auto cpr = cross(GlmVec3{ -1, 0, 0 }, GlmVec3{ 0, 1, 0 });
    static_assert(cpr.z < -.9 && cpr.z > -1.1);
    }
    assert(magnitude( cross(UserVector3{ -1, 0, 0 }, UserVector3{ 0, 1, 0 }).z + 1) < k_error);

    // directed_angle_between (2D context only)
    {
    auto da = directed_angle_between( Vec2D{1, 0}, Vec2D{0, 1} );
    // I'm not sure what "convention" I'm using here...
    assert(magnitude( da - k_pi_f*0.5 ) < k_error );
    // must work with "rotate_vector"
    assert(magnitude( rotate_vector(Vec2D{1, 0}, da) - Vec2D{0, 1} ) < k_error);
    assert(directed_angle_between(UserVector{1, 0}, UserVector{0, -1}) < 0);
    }

    // dot
    {
    constexpr auto dpr = dot( GlmVec4{0, 1, 2, 3}, GlmVec4{3, 2, 1, 0} );
    static_assert(dpr < 4.1 && dpr > 3.9);
    auto dpr2 = dot(UserVector{1, 2}, UserVector{2, 3});
    assert(magnitude(dpr2 - 8) < k_error);
    }

    // find_closest_point_to_line
    {
    constexpr auto r = find_closest_point_to_line(Vec2I{0, 0}, Vec2I{6, 3}, Vec2I{5, 4});
    std::cout << "Closest point from <5, 4> to <0, 0> - <6, 3> = " << r.x << ", " << r.y << std::endl;
    }
    {
    auto res = find_closest_point_to_line(UserVector3{0, 0, 0}, UserVector3{10, 10, 10}, UserVector3{5, 5, 6});
    std::cout << "(" << res.x << ", " << res.y << ", " << res.z << ")" << std::endl;
    }
    // find_intersection
    {
    constexpr auto res = find_intersection(Vec2I{0, 0}, Vec2I{10, 10}, Vec2I{0, 10}, Vec2I{10, 0});
    static_assert(res == Vec2I{5, 5});
    std::cout << "intx: " << res.x << ", " << res.y << std::endl;
    }
    find_intersection(UserVector{0, 0}, UserVector{10, 10}, UserVector{0, 10}, UserVector{10, 0});
#   if 1
    // find_velocities_to_target
    {
    auto [s0, s1] = find_velocities_to_target(UserVector{0, 0}, UserVector{10, 2}, UserVector{0, -10}, 12.);
    std::cout << "vel to target: (" << s0.x << ", " << s0.y
              << "), ("             << s1.x << ", " << s1.y
              << ")" << std::endl;
    }
#   endif
#   if 0
    {
    auto [s0, s1] = find_velocities_to_target(Vec2I{0, 0}, Vec2I{10, 2}, Vec2I{0, -10}, 12);
    std::cout << "vel to target: (" << s0.x << ", " << s0.y
              << "), ("             << s1.x << ", " << s1.y
              << ")" << std::endl;
    }
    // now in 3D
    {
    auto [s0, s1] = find_velocities_to_target(Vec2I{0, 0}, Vec2I{10, 2}, Vec2I{0, -10}, 12);
    std::cout << "vel to target: (" << s0.x << ", " << s0.y
              << "), ("             << s1.x << ", " << s1.y
              << ")" << std::endl;
    }
#   endif
    // is_inside_triangle
    static_assert( is_inside_triangle(Vec2I{0, 0}, Vec2I{6, 0}, Vec2I{3, 3}, Vec2I{3,  1}));
    static_assert(!is_inside_triangle(Vec2I{0, 0}, Vec2I{6, 0}, Vec2I{3, 3}, Vec2I{3, -1}));
    // is_real

    static_assert(is_real(Vec2I{1, 2}));
    static_assert(!is_real(Vec2D{1, k_inf}));
    assert(is_real(UserVector{1., 2.}));
    assert(!is_real(UserVector3{1., k_inf, 2.}));
#   if 1
    // is_solution
    assert(!is_solution(std::get<0>(
        find_velocities_to_target(Vec2D{0, 0}, Vec2D{100, 100}, Vec2D{0, -10}, 2.)
    )));
    assert(!is_solution(std::get<0>(
        find_velocities_to_target(UserVector{0, 0}, UserVector{100, 100}, UserVector{0, -10}, 2.)
    )));
#   endif
    // is_zero_vector

    assert(is_zero_vector(Vec2I{}));
    assert(!is_zero_vector(GlmVec3{0, 0, 1}));
    assert(is_zero_vector(UserVector{ 0, 0 }));
    // magnitude
    assert(magnitude(Vec2I{ 3, 4 }) == 5);
    magnitude(GlmVec3{ 5, 4, 6 });

    // make_zero_vector
    static_assert(make_zero_vector<GlmVec3>().z == 0);
    assert(make_zero_vector<UserVector3>().z == 0);

    // make_nonsolution_sentinel (both integer and floating point)
    static_assert(!is_solution(make_nonsolution_sentinel<Vec2I>()));
    static_assert(!is_solution(make_nonsolution_sentinel<GlmVec3>()));
    assert(!is_solution(make_nonsolution_sentinel<UserVector>()));

    // normalize
    assert(magnitude( magnitude( normalize(GlmVec4{ 5, 4, 2, 1}) ) - 1 ) < k_error);

    // project_onto
    {
    auto res = project_onto(UserVector{ 1, 3 }, UserVector{ -1, 0 });
    assert(magnitude(res.x - 1) < k_error && magnitude(res.y) < k_error);
    }
    // project_onto_plane
    {
    auto res = project_onto_plane(UserVector3{ 1, 1, 1 }, UserVector3{ 0, -1, 0 });
    assert(res.x > 0 && res.z > 0 && res.y < k_error);
    }
    // round_to
    assert((round_to<int>( Vec2D{ 2.3, 4.5 } ) == Vec2I{2, 5}));
    static_assert(std::is_same_v<decltype(round_to<int>(GlmVec3{})), GlmVec3I>);

    // rotate_vector
    // test library stated interface
    {
        using RealDistri = std::uniform_real_distribution<double>;
        using IntDistri = std::uniform_int_distribution<int>;
        auto rndg = std::default_random_engine{0xDEADBEEF};
        auto rndv = [&rndg] {
            auto x = RealDistri{-5, 5}(rndg);
            double y = magnitude(x) < k_error ?
                       RealDistri{1, 5}(rndg)*( IntDistri{0, 1}(rndg) ? 1 : -1 ) :
                       RealDistri{-5, 5}(rndg);
            return Vec2D{x, y};
        };
        for (int i = 0; i != 100; ++i) {
            auto from = rndv();
            auto to   = rndv();
            auto t      = directed_angle_between(from, to);
            auto new_to = rotate_vector(from, t);
            assert(angle_between(new_to, to) < k_error);
        }
    }

    // sum_of_squares
    static_assert(sum_of_squares(Vec2I{ 3, 4 }) == 25);
    static_assert(sum_of_squares(GlmVec3I{ 3, 4, 5 }) == 50);
    assert(magnitude(sum_of_squares(UserVector{3, 4}) - 25) < k_error);
}
