/****************************************************************************

    File: DrawRectangle.hpp
    Author: Aria Janke
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

#pragma once

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Vertex.hpp>

#include <common/FixedLengthArray.hpp>

#include <type_traits>

/** A drawing rectangle by default is zero-sized white "box" located at the
 *  origin. @n
 *  Units for this class are in pixels. @n
 *  I hope it's self-explanitory what the class members do.
 */
class DrawRectangle final : public sf::Drawable {
public:

    static constexpr const unsigned k_vertex_count = 4;

    DrawRectangle();

    DrawRectangle
        (float x_, float y_, float width_ = 0.f, float height_ = 0.f,
         sf::Color clr_ = sf::Color::White);

    ~DrawRectangle() override;

    void set_x(float) noexcept;

    void set_y(float) noexcept;

    void set_width(float) noexcept;

    void set_height(float) noexcept;

    void set_position(float x_, float y_) noexcept;

    void set_position(const sf::Vector2f & r) noexcept;

    void move(const sf::Vector2f & r) noexcept;

    void move(float x_, float y_) noexcept;

    void set_size(float width, float height) noexcept;

    void set_color(sf::Color) noexcept;

    float width() const noexcept;

    float height() const noexcept;

    float x() const noexcept;

    float y() const noexcept;

    sf::Vector2f position() const noexcept;

    sf::Color color() const noexcept;

protected:

    void draw(sf::RenderTarget &, sf::RenderStates) const override;

private:

    static constexpr const unsigned k_top_left     = 0;
    static constexpr const unsigned k_top_right    = 1;
    static constexpr const unsigned k_bottom_right = 2;
    static constexpr const unsigned k_bottom_left  = 3;

    // use that dank af copy semantics
    FixedLengthArray<sf::Vertex, k_vertex_count> m_vertices;
};
