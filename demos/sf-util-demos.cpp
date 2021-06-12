#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/System/Sleep.hpp>

#include <common/sf/DrawText.hpp>
#include <common/sf/DrawLine.hpp>
#include <common/sf/DrawRectangle.hpp>
#include <common/sf/DrawTriangle.hpp>
#include <common/TypeList.hpp>
#include <common/Util.hpp>
#include <common/StringUtil.hpp>

#include <SFML/Graphics/Texture.hpp>

#include <random>
#include <iostream>

#include <cassert>

namespace {

using cul::DrawText;
using cul::DrawRectangle;
using cul::DrawTriangle;
using cul::BitmapFont;
using cul::DrawLine;
using cul::round_to;
using cul::SfBitmapFont;

constexpr const int k_window_width  = 800;
constexpr const int k_window_height = 600;
constexpr const int k_fps           =  60;

class AppState {
public:
    virtual ~AppState() {}
    virtual void setup() = 0;
    virtual void update(double) = 0;
    virtual void process_event(const sf::Event &) = 0;
    virtual void draw_to(sf::RenderTarget &) = 0;
    virtual const std::string & name() const = 0;

    std::unique_ptr<AppState> give_next_state() { return std::move(m_next_state); }

    static std::unique_ptr<AppState> default_instance();

protected:
    template <typename T>
    std::enable_if_t<std::is_base_of_v<AppState, T>, T &> set_next_state() {
        auto ptr = std::make_unique<T>();
        T * rv = ptr.get();
        m_next_state = std::move(ptr);
        return *rv;
    }

private:
    std::unique_ptr<AppState> m_next_state;
};

} // end of <anonymous> namespace

int main() {
    SfBitmapFont::load_builtin_font(BitmapFont::k_8x8_font).texture().copyToImage().saveToFile("/media/ramdisk/cultextout.png");
    SfBitmapFont::load_builtin_font(BitmapFont::k_8x8_highlighted_font).texture().copyToImage().saveToFile("/media/ramdisk/cultextouthighlight.png");

    sf::RenderWindow win;
    win.setFramerateLimit(k_fps);
    win.create(sf::VideoMode(k_window_width, k_window_height), "Common Utilities Demo for SFML");
    auto app_state = AppState::default_instance();
    app_state->setup();

    DrawText title_text;
    title_text.load_builtin_font(BitmapFont::k_8x8_highlighted_font);
    title_text.set_text_top_left(sf::Vector2f(), app_state->name());

    while (win.isOpen()) {
        {
        sf::Event event;
        while (win.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                win.close();
            } else if (event.type == sf::Event::KeyReleased) {
                if (event.key.code == sf::Keyboard::Escape)
                    win.close();
            }
            app_state->process_event(event);
        }
        }
        if (auto new_state = app_state->give_next_state()) {
            app_state = std::move(new_state);
            app_state->setup();

            title_text.set_text_top_left(sf::Vector2f(), app_state->name());
        }
        app_state->update(1. / double(k_fps));

        win.clear(sf::Color(0, 80, 0));
        app_state->draw_to(win);
        win.draw(title_text);
        win.display();
        sf::sleep(sf::microseconds(1'000'000 / k_fps));
    }
    return 0;
}

namespace {

using Rng = std::default_random_engine;

using DoubleDistri = std::uniform_real_distribution<double>;
using FloatDistri  = std::uniform_real_distribution<float>;
using IntDistri    = std::uniform_int_distribution<int>;

class RandomRectangleState;
class RandomTriangleState;
class RandomTextState;
class LineTestState;
class BezierLineState;
class IntroductionState;

enum class FadeStage {
    k_fade_in, k_still, k_fade_out
};

constexpr const double k_fade_in_time  = 1.5;
constexpr const double k_still_time    = 3.5;
constexpr const double k_fade_out_time = 1. ;
constexpr const uint8_t k_u8_max = std::numeric_limits<uint8_t>::max();

inline uint8_t fade_in_alpha(double time_rem)
    { return round_to<uint8_t>((1. - (time_rem / k_fade_in_time))*k_u8_max); }

inline uint8_t fade_out_alpha(double time_rem)
    { return round_to<uint8_t>((time_rem / k_fade_out_time)*k_u8_max); }

inline bool check_and_progress(FadeStage & stage, double & time_rem, double et) {
    if ((time_rem -= et) > 0.) return false;
    switch (stage) {
    case FadeStage::k_fade_in:
        time_rem = k_still_time;
        stage    = FadeStage::k_still;
        break;
    case FadeStage::k_fade_out:
        time_rem = k_fade_in_time;
        stage    = FadeStage::k_fade_in;
        return true;
    case FadeStage::k_still:
        time_rem = k_fade_out_time;
        stage    = FadeStage::k_fade_out;
        break;
    }
    return false;
}

sf::Color random_color(Rng &);

class RotatingState : public AppState {
public:
    void process_event(const sf::Event &) override;

    using StateTypeList = cul::TypeList<
        RandomRectangleState, RandomTriangleState, RandomTextState,
        LineTestState, /*BezierLineState,*/ IntroductionState>;

    template <typename T>
    static constexpr const int k_state_list_position_of
        = RotatingState::StateTypeList::GetTypeId<T>::k_value;

    virtual int position_in_state_list() const = 0;

private:
    template <typename ... Types>
    void switch_to_state_by_type_id(int, cul::TypeList<Types...>);

    template <typename Head, typename ... Types>
    void switch_to_state_by_type_id(int, cul::TypeList<Head, Types...>);
};

template <typename T>
class RotatingStateOf : public RotatingState {
    static_assert(StateTypeList::HasType<T>::k_value, "");

    int position_in_state_list() const final
        { return k_state_list_position_of<T>; }
};

class RandomRectangleState final : public RotatingStateOf<RandomRectangleState> {
public:
    void setup() final {
        m_drect          = random_rectangle(m_rng);
        m_stage          = FadeStage::k_fade_in;
        m_time_remaining = k_fade_in_time;
    }

    void update(double et) final {
        if (check_and_progress(m_stage, m_time_remaining, et)) {
            m_drect = random_rectangle(m_rng);
        }
        auto color = m_drect.color();
        switch (m_stage) {
        case FadeStage::k_fade_in:
            assert(color.a <= fade_in_alpha(m_time_remaining));
            color.a = fade_in_alpha(m_time_remaining);
            break;
        case FadeStage::k_fade_out:
            assert(color.a >= fade_out_alpha(m_time_remaining));
            color.a = fade_out_alpha(m_time_remaining);
            break;
        case FadeStage::k_still:
            color.a = 255;
            break;
        }
        m_drect.set_color(color);
    }

    void draw_to(sf::RenderTarget & target) final { target.draw(m_drect); }

    const std::string & name() const final {
        static const std::string k_name = "Random Rectangles";
        return k_name;
    }

private:
    // always alpha of 0
    static DrawRectangle random_rectangle(Rng & rng) {
        auto x = FloatDistri(0.f, k_window_width)(rng);
        auto y = FloatDistri(0.f, k_window_height)(rng);
        auto w = FloatDistri(0.f, k_window_width - x)(rng);
        auto h = FloatDistri(0.f, k_window_width - y)(rng);
        return DrawRectangle(x, y, w, h, random_color(rng));
    }

    DrawRectangle m_drect;

    double m_time_remaining = 0.;
    FadeStage m_stage;
    Rng m_rng { std::random_device()() };
};

class RandomTriangleState final : public RotatingStateOf<RandomTriangleState> {
public:
    void setup() final {
        m_dtri           = random_triangle(m_rng);
        m_stage          = FadeStage::k_fade_in;
        m_time_remaining = k_fade_in_time;
    }

    void update(double et) final {
        if (check_and_progress(m_stage, m_time_remaining, et)) {
            m_dtri = random_triangle(m_rng);
        }
        auto color = m_dtri.color();
        switch (m_stage) {
        case FadeStage::k_fade_in:
            color.a = fade_in_alpha(m_time_remaining);
            break;
        case FadeStage::k_fade_out:
            color.a = fade_out_alpha(m_time_remaining);
            break;
        case FadeStage::k_still:
            color.a = 255;
            break;
        }
        m_dtri.set_color(color);
    }

    void draw_to(sf::RenderTarget & target) final { target.draw(m_dtri); }

    const std::string & name() const final {
        static const std::string k_name = "Random Triangles";
        return k_name;
    }

private:
    static DrawTriangle random_triangle(Rng & rng) {
        std::array<sf::Vector2f, 3> pts;
        for (auto * fptr : { &pts[0].x, &pts[1].x, &pts[2].x }) {
            *fptr = FloatDistri(0.f, k_window_width)(rng);
        }
        for (auto * fptr : { &pts[0].y, &pts[1].y, &pts[2].y }) {
            *fptr = FloatDistri(0.f, k_window_height)(rng);
        }
        DrawTriangle dtri;
        dtri.set_point_a(pts[0]);
        dtri.set_point_b(pts[1]);
        dtri.set_point_c(pts[2]);
        dtri.set_color(random_color(rng));
        return dtri;
    }
    DrawTriangle m_dtri;
    double m_time_remaining = 0.;
    FadeStage m_stage;
    Rng m_rng { std::random_device()() };
};

class RandomTextState final : public RotatingStateOf<RandomTextState> {
public:
    void setup() final {
        static constexpr const int k_iters = 200;
        static constexpr const double k_total_setup_time = 5.;
        for (int i = 0; i != k_iters; ++i) {
            update(k_total_setup_time / k_iters);
        }
    }

    void update(double et) final {
        if (DoubleDistri(0., 1.)(m_rng) < probability_of_spawn()*et) {
            spawn_string<random_string>(random_screen_position(m_rng));
        }
        for (auto & text : m_texts) {
            text.location_y -= k_rise_speed*et;
            auto displacement = sf::Vector2f(0, -1.f)*k_rise_speed*float(et);
            for (auto & vtx : text.verticies) {
                vtx.position += displacement;
            }
        }
        auto tend = m_texts.end();
        m_texts.erase(std::remove_if(m_texts.begin(), tend, should_delete_float_text), tend);
    }

    void draw_to(sf::RenderTarget & target) final {
        auto states = sf::RenderStates::Default;
        for (auto & text : m_texts) {
            states.texture = text.texture;
            target.draw(text.verticies.data(), text.verticies.size(),
                        DrawText::k_primitive_type, states);
        }
    }

    void process_event(const sf::Event & event) final {
        RotatingState::process_event(event);
        if (event.type == sf::Event::MouseButtonReleased) {
            spawn_string<random_click_string>(
                sf::Vector2f(event.mouseButton.x, event.mouseButton.y));
        }
    }

    const std::string & name() const final {
        static const std::string k_name = "Random Text";
        return k_name;
    }

private:
    static constexpr float k_rise_speed = 125.f;

    double probability_of_spawn() const {
        // probability of spawn per second
        static constexpr const double k_min_probability = 0.05;
        static constexpr const double k_max_probability = 1.75;
        static constexpr const int k_quantity_to_min = 30;
        if (m_texts.size() > k_quantity_to_min) return k_min_probability;
        auto intpl = 1. - (double(m_texts.size()) / double(k_quantity_to_min));
        return k_min_probability + (k_max_probability - k_min_probability)*intpl;
    }

    static sf::Vector2f random_screen_position(Rng & rng) {
        return sf::Vector2f(FloatDistri(0.f, k_window_width )(rng),
                            FloatDistri(0.f, k_window_height)(rng));
    }

    template <const std::string & (*choose_string_f)(Rng &)>
    void spawn_string(sf::Vector2f position) {
        DrawText dtext;
        static const std::array k_font_choices {
            BitmapFont::k_8x8_font, BitmapFont::k_8x8_highlighted_font
        };
        dtext.load_builtin_font(k_font_choices[ IntDistri(0, k_font_choices.size() - 1)(m_rng) ]);
        dtext.set_text_center(position, choose_string_f(m_rng));

        m_texts.emplace_back();
        m_texts.back().location_y  = k_window_height + dtext.font()->character_size().height / 2;
        m_texts.back().char_height = dtext.font()->character_size().height;
        m_texts.back().texture     = &dtext.font()->texture();
        m_texts.back().verticies   = dtext.give_verticies();
    }

    static const std::string & random_string(Rng & rng) {
        using Str = std::string;
        static std::array strings {
            Str("Hello There"),
            Str("This is a test string")
        };
        return strings[IntDistri(0, strings.size())(rng)];
    }

    static const std::string & random_click_string(Rng & rng) {
        using Str = std::string;
        static std::array strings {
            Str("Hello There"),
            Str("Click!"),
        };
        return strings[IntDistri(0, strings.size())(rng)];
    }

    struct FloatText {
        const sf::Texture * texture = nullptr;
        float location_y = 0.f;
        float char_height = 0.f;
        std::vector<sf::Vertex> verticies;
    };

    static bool should_delete_float_text(const FloatText & text)
        { return text.location_y + text.char_height * 0.5f < 0.f; }

    std::vector<FloatText> m_texts;
    Rng m_rng { std::random_device()() };
};

class LineTestState final : public RotatingStateOf<LineTestState> {
public:
    void setup() final {
        update_lines();
    }

    void update(double et) final {
        m_elapsed_time += et;
        update_lines();
    }

    void draw_to(sf::RenderTarget & target) final {
        target.draw(m_verticies.data(), m_verticies.size(), sf::PrimitiveType::Triangles);
        for (const auto & line : m_lines) {
            target.draw(line);
        }
    }

    const std::string & name() const final {
        static const std::string k_name = "Line Rendering Tests";
        return k_name;
    }

private:
    void update_lines() {
        static constexpr const double k_pi = cul::k_pi_for_type<double>;
        sf::Vector2f center(k_window_width / 2, k_window_height / 2);
        m_verticies.clear();
        m_verticies.reserve(k_line_count*6);
        m_lines.clear();
        m_lines.reserve(k_line_count);
        auto t_offset = std::fmod(m_elapsed_time*k_rotation_speed, 2.*k_pi);
        auto thickness = k_thickness_minimum
            +  (std::sin( m_elapsed_time*k_thickness_change_speed ) + 1.f)
              * 0.5f * (k_thickness_maximum - k_thickness_minimum);
        for (int i = 0; i != k_line_count / 2; ++i) {
            auto t = (double(i) / double(k_line_count))*2.*k_pi + t_offset;
            auto pt = sf::Vector2f(std::cos(t), std::sin(t))*0.6f*float(std::min(k_window_width, k_window_height));
            DrawLine dline(center, center + pt, thickness, sf::Color::White);
            m_lines.emplace_back(dline);
        }
    }

    // radians per second
    static const constexpr double k_rotation_speed         = 0.25;
    // also radian per second (sine between [0 1])
    static const constexpr double k_thickness_change_speed = 0.3;

    static const constexpr double k_thickness_maximum      = 15.;
    static const constexpr double k_thickness_minimum      =  1.;

    static const constexpr int k_line_count = 28;

    std::vector<sf::Vertex> m_verticies;
    std::vector<DrawLine> m_lines;

    double m_elapsed_time = 0.;
};

class BezierLineState;

class IntroductionState final : public RotatingStateOf<IntroductionState> {
public:

    static constexpr const auto k_intro_font = BitmapFont::k_8x8_highlighted_font;

    void setup() final {
        using namespace cul;
        int line_count = 0;
        for_split<is_newline>(k_intro_text, k_intro_text_end,
            [&line_count](const char *, const char *)
        { ++line_count; });

        const auto & intro_font_inst = SfBitmapFont::load_builtin_font(k_intro_font);
        auto y_start = (k_window_height - intro_font_inst.character_size().height*line_count ) / 2;
        m_texts.reserve(line_count);
        for_split<is_newline>(k_intro_text, k_intro_text_end,
            [this, &y_start, &intro_font_inst](const char * beg, const char * end)
        {
            DrawText dtext;
            dtext.assign_font(intro_font_inst);
            dtext.set_text_center(sf::Vector2f(k_window_width / 2, y_start),
                                  beg, end);
            m_texts.emplace_back(std::move(dtext));
            y_start += intro_font_inst.character_size().height;
        });
    }

    void update(double) final {}

    void draw_to(sf::RenderTarget & target) final {
        for (const auto & text : m_texts) {
            target.draw(text);
        }
    }

    const std::string & name() const final {
        static const std::string k_name = "Introduction Screen";
        return k_name;
    }

private:
    static bool is_newline(char c) { return c == '\n'; }

    static constexpr const char * k_intro_text =
        "Hello, this is my Common Utilities for SFML Demo App\n"
        "You will notice the current state's name in the top left corner.\n"
        "Use the left and right arrow keys to switch app states.\n"
        "Press Escape to quit the application.";

    static constexpr const char * k_intro_text_end =
        cul::find_str_end(k_intro_text);

    std::vector<DrawText> m_texts;
};

sf::Color random_color(Rng & rng) {
    using Uint8Distri = std::uniform_int_distribution<uint8_t>;
    std::array<uint8_t, 3> attrs;
    attrs[0] = Uint8Distri(180, 255)(rng);
    attrs[1] = (DoubleDistri(0., 1.)(rng) > 0.5 ? Uint8Distri(0, 100) : Uint8Distri(180, 255))(rng);
    attrs[2] = (DoubleDistri(0., 1.)(rng) > 0.25 ? Uint8Distri(0, 100) : Uint8Distri(180, 255))(rng);
    std::shuffle(attrs.begin(), attrs.end(), rng);
    return sf::Color(attrs[0], attrs[1], attrs[2], 0);
}

/* static */ std::unique_ptr<AppState> AppState::default_instance() {
    return std::make_unique<IntroductionState>();
}

void RotatingState::process_event(const sf::Event & event) {
    if (event.type != sf::Event::KeyReleased) return;
    static constexpr const auto k_state_count = StateTypeList::k_count;
    switch (event.key.code) {
    case sf::Keyboard::Left: {
        auto np = position_in_state_list() - 1;
        if (np == -1) np = k_state_count - 1;
        switch_to_state_by_type_id(np, StateTypeList());
        }
        break;
    case sf::Keyboard::Right: {
        auto np = position_in_state_list() + 1;
        switch_to_state_by_type_id(np % k_state_count, StateTypeList());
        }
        break;
    default: break;
    }
}

template <typename ... Types>
/* private */ void RotatingState::switch_to_state_by_type_id(int, cul::TypeList<Types...>) {
    throw std::invalid_argument("index not a valid type id");
}

template <typename Head, typename ... Types>
/* private */ void RotatingState::switch_to_state_by_type_id(int idx, cul::TypeList<Head, Types...>) {
    if (idx == k_state_list_position_of<Head>) {
        return (void)set_next_state<Head>();
    }
    switch_to_state_by_type_id(idx, cul::TypeList<Types...>());
}
#if 0
/* private */ void RotatingState::switch_to_state_by_type_id(int idx) {
    assert(idx >= 0 && idx < int(StateTypeList::k_count));
    switch (idx) {
    case k_state_list_position_of  <RandomRectangleState>:
        return (void)set_next_state<RandomRectangleState>();
    case k_state_list_position_of  <RandomTriangleState>:
        return (void)set_next_state<RandomTriangleState>();
    case k_state_list_position_of  <RandomTextState>:
        return (void)set_next_state<RandomTextState>();
    case k_state_list_position_of  <LineTestState>:
        return (void)set_next_state<LineTestState>();
    /*k_state_list_position_of<BezierLineState>*/
    case k_state_list_position_of  <IntroductionState>:
        return (void)set_next_state<IntroductionState>();
    }
    throw std::runtime_error("bad branch");
}
#endif

} // end of <anonymous> namespace
