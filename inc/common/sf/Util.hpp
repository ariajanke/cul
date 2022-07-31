/****************************************************************************

    MIT License

    Copyright 2022 Aria Janke

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.

*****************************************************************************/

#pragma once

#include <common/Grid.hpp>
#ifdef MACRO_NEW_20220728_VECTORS
#   include <common/sf/VectorTraits.hpp>
#   include <common/VectorUtils.hpp>
#else
#   include <common/SfmlVectorTraits.hpp>
#endif
#include <SFML/Graphics/Image.hpp>

namespace cul {

sf::Image to_image(const Grid<sf::Color> &);

Grid<sf::Color> to_color_grid(const sf::Image &);

template <typename T>
sf::Vector2f to_sf_vec2f(const cul::Vector2<T> & r)
    { return convert_to<sf::Vector2<T>>(r); }

} // end of cul namespace
