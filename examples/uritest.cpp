//-----------------------------------------------------------------------------------------
// fiber (header only)
// Copyright (C) 2022-23 Fix8 Market Technologies Pty Ltd
//   by David L. Dight
// see https://github.com/fix8mt/fiber
//
// Lightweight header-only stackful per-thread fiber
//		with built-in roundrobin scheduler x86_64 / linux only
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
#include <iostream>
#include <functional>
#include <deque>
#include <set>
#include <fix8/fiber.hpp>

//-----------------------------------------------------------------------------------------
using namespace FIX8;
using namespace std::literals;

//-----------------------------------------------------------------------------------------
void doit(int arg)
{
	std::cout << this_fiber::name(("sub"s + std::to_string(arg)).c_str());
	std::cout << "\tstarting " << arg << '\n';
	for (int ii{}; ii < arg; )
	{
		std::cout << '\t' << this_fiber::name() << ' ' << arg << ": " << ++ii << '\n';
		this_fiber::yield();
	}
	std::cout << "\tleaving " << arg << '\n';
	fibers::print();
}

struct foo
{
	void sub(int arg)
	{
		doit(arg);
	}
	void sub1(int arg, const char *str)
	{
		std::cout << str << '\n';
		doit(arg);
	}
	void sub3(int arg, const char *str)
	{
		auto st { "sub"s + std::to_string(arg) };
		this_fiber::name(st.c_str());
		std::cout << "\tsub2 starting " << arg << '\n';
		for (int ii{}; ii < arg; )
		{
			std::cout << '\t' << this_fiber::name() << ' ' << arg << ": " << ++ii << '\n';
			//this_fiber::sleep_until(std::chrono::steady_clock::now() + 500ms);
			this_fiber::sleep_for(100ms);
		}
		std::cout << "\tsub2 leaving " << arg << '\n';
	}
	void sub2()
	{
		doit(4);
	}
};

//-----------------------------------------------------------------------------------------
int main(void)
{
	foo bar;
	fiber sub_co({.stacksz=2048}, &doit, 3),
			sub_co1({.stacksz=16384}, &foo::sub2, &bar),
			sub_co2({.stacksz=32768}, &foo::sub, &bar, 5),
			sub_co3({.stacksz=8192}, &foo::sub1, &bar, 8., "hello"),
			sub_co4(std::bind(&foo::sub3, &bar, 12, "there"));
	fiber sub_lam({.name="sub lambda",.stack=std::make_unique<f8_fixedsize_mapped_stack>()}, [](int arg)
	//char stack[4096];
	//fiber sub_lam({.name="sub lambda",.stacksz=sizeof(stack),.stack=std::make_unique<f8_fixedsize_placement_stack>(stack)}, [](int arg)
	{
		std::cout << "\tlambda starting " << arg << '\n';
		for (int ii{}; ii < arg; )
		{
			std::cout << '\t' << this_fiber::name() << ' ' << arg << ": " << ++ii << '\n';
			this_fiber::yield();
		}
		std::cout << "\tlambda leaving " << arg << '\n';
	}, 15);
	for (int ii{}; fibers::has_fibers(); ++ii)
	{
		if (ii == 0)
		{
			fibers::print(std::cout);
			sub_co3.resume();
			fibers::print(std::cout);
		}
		this_fiber::yield();
		std::cout << "main: " << std::dec << ii << '\n';
		//fibers::print(std::cout);
	}
	std::cout	<< "Exiting from main\n"
					<< fibers::size_finished() << " fibers finished\n"
					<< "fiber: " << sizeof(fiber) << '\n'
					<< "fiber::cvars: " << sizeof(fiber::cvars) << '\n'
					<< "fiber::all_cvars: " << sizeof(fiber::all_cvars) << '\n'
					<< "fiber_id: " << sizeof(fiber_id) << '\n'
					<< "fiber_base: " << sizeof(fiber_base) << '\n'
					<< "fiber_params: " << sizeof(fiber_params) << std::endl;
	return 0;
}
