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

#include <ariajanke/cul/Vector2.hpp>

using Vector2I = cul::Vector2<int>;

// equality/inequality
static_assert(Vector2I{2, 3} == Vector2I{2, 3});
static_assert(Vector2I{2, 2} != Vector2I{2, 3});

// vector operators in order of declaration
static_assert(-Vector2I{2, -5} == Vector2I{-2, 5});
static_assert(Vector2I{1, 2} + Vector2I{3, 4} == Vector2I{4, 6});
static_assert(Vector2I{5, 2} - Vector2I{3, 4} == Vector2I{2, -2});

// scalar operators in order of declaration
static_assert(Vector2I{2, 3}*2 == Vector2I{4, 6});
static_assert(Vector2I{9, 6} / 3 == Vector2I{3, 2});
static_assert(3*Vector2I{-1, 3} == Vector2I{-3, 9});
