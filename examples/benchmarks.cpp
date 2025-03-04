//-----------------------------------------------------------------------------------------
// uri (header only)
// Copyright (C) 2024 Fix8 Market Technologies Pty Ltd
//   by David L. Dight
// see https://github.com/fix8mt/uri
//
// Lightweight header-only C++20 URI parser
//
// Distributed under the Boost Software License, Version 1.0 August 17th, 2003
//
// Permission is hereby granted, free of charge, to any person or organization
// obtaining a copy of the software and accompanying documentation covered by
// this license (the "Software") to use, reproduce, display, distribute,
// execute, and transmit the Software, and to prepare derivative works of the
// Software, and to permit third-parties to whom the Software is furnished to
// do so, all subject to the following:
//
// The copyright notices in the Software and this entire statement, including
// the above license grant, this restriction and the following disclaimer,
// must be included in all copies of the Software, in whole or in part, and
// all derivative works of the Software, unless such copies or derivative
// works are solely in the form of machine-executable object code generated by
// a source language processor.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//-----------------------------------------------------------------------------------------
#include <array>
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
BENCHMARK(basic_uri_1000)
{
	// SETUP_BENCHMARK()

	for (const auto& pp : uris)
	{
		basic_uri a1{pp};
		auto hs { a1.get_component<uri::host>() };
	}

	//	TEARDOWN_BENCHMARK()
}

BENCHMARK(uri_1000)
{
	//SETUP_BENCHMARK()

	for (const auto& pp : uris)
	{
		uri a1{pp};
		auto hs { a1.get_component<uri::host>() };
	}

	//TEARDOWN_BENCHMARK()
}

BENCHMARK(uri_static_1000)
{
	//SETUP_BENCHMARK()

	for (const auto& pp : uris)
	{
		uri_static<> a1{pp};
		auto hs { a1.get_component<uri::host>() };
	}

	//TEARDOWN_BENCHMARK()
}

CRITERION_BENCHMARK_MAIN()

