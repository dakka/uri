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
#include <fix8/uri.hpp>
#include <criterion/criterion.hpp>
//-----------------------------------------------------------------------------------------
using namespace FIX8;

//-----------------------------------------------------------------------------------------
constinit const std::array uris { std::to_array<std::string_view>
({
#include <basiclist.hpp>
})};

//-----------------------------------------------------------------------------------------
BENCHMARK(uri_view_1000)
{
	// SETUP_BENCHMARK()

	for (const auto& pp : uris)
		[[maybe_unused]] uri_view a1{pp};

	//	TEARDOWN_BENCHMARK()
}

BENCHMARK(uri_1000)
{
	//SETUP_BENCHMARK()

	for (const auto& pp : uris)
		[[maybe_unused]] uri a1{pp};

	//TEARDOWN_BENCHMARK()
}

BENCHMARK(uri_static_1000)
{
	//SETUP_BENCHMARK()

	for (const auto& pp : uris)
		[[maybe_unused]] uri_static a1{pp};

	//TEARDOWN_BENCHMARK()
}

CRITERION_BENCHMARK_MAIN()

