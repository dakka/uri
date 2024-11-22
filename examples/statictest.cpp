//-----------------------------------------------------------------------------------------
// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (C) 2024 Fix8 Market Technologies Pty Ltd
// SPDX-FileType: SOURCE
//
// uri (header only)
// Copyright (C) 2024 Fix8 Market Technologies Pty Ltd
//   by David L. Dight
// see https://github.com/fix8mt/uri
//
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is furnished
// to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice (including the next paragraph)
// shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
// INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
// PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
// HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//-----------------------------------------------------------------------------------------
#include <iostream>
#include <stdexcept>
#include <memory>
#include <string_view>
#include <fix8/uri.hpp>

//-----------------------------------------------------------------------------------------
using namespace FIX8;
using namespace std::literals::string_view_literals;

//-----------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
	constexpr std::string_view s1 { "https://dakka@www.blah.com:3000/" };
	constexpr uri_view u1{s1};
	std::cout << u1.get_uri() << '\t' << '(' << sizeof(u1) << ')' << '\n';
	uri u2{s1};
	std::cout << u2.get_uri() << '\t' << '(' << sizeof(u2) << ')' << '\n';

	constexpr uri_fixed u3{"https://user:password@example.com/path?search=1"};
	std::cout << u3.get_uri() << '\t' << '(' << sizeof(u3) << ')' << '\n';
	uri u3a{"https://user:password@example.com/path?search=1"};
	std::cout << u3a.get_uri() << '\t' << '(' << sizeof(u3a) << ')' << '\n';

	uri u4{"https://example.org/./a/../b/./c"};
	std::cout << u4.get_uri() << '\t' << '(' << sizeof(u4) << ')' << '\n';

	return 0;
}

