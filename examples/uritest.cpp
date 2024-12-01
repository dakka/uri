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
#include <fstream>
#include <vector>
#include <stdexcept>
#include <memory>
#include <string_view>
#include <bitset>
#include <any>
#include <getopt.h>
#include <fix8/uri.hpp>

//-----------------------------------------------------------------------------------------
using namespace FIX8;
using namespace std::literals::string_view_literals;

//-----------------------------------------------------------------------------------------
#include <uriexamples.hpp>

//-----------------------------------------------------------------------------------------
void do_interactive();

//-----------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
   static constexpr const char *optstr{"t:T:d:hlasxf:iF:D:qV"};
	static constexpr auto long_options
	{
		std::to_array<option>
		({
			{ "help",			no_argument,			nullptr, 'h' },
			{ "verbose",		no_argument,			nullptr, 'V' },
			{ "list",			no_argument,			nullptr, 'l' },
			{ "sizes",			no_argument,			nullptr, '2' },
			{ "all",				no_argument,			nullptr, 'a' },
			{ "interactive",	no_argument,			nullptr, 'i' },
			{ "file",			required_argument,	nullptr, 'f' },
			{ "dump",			required_argument,	nullptr, 'd' },
			{ "test",			required_argument,	nullptr, 't' },
			{ "stat",			required_argument,	nullptr, 'T' },
			{}
		})
	};

	int val;
	bool decode{}, quiet{}, verbose{}, interactive{};

	auto runall([&verbose]
	{
		for (int ii{}; const auto& [src,vec] : tests)
			std::cout << ii++ << '\n' << (verbose ? uri::print_mode::detailed : uri::print_mode::default_mode) << uri{src} << '\n';
		std::cout << tests.size() << " test cases\n";
	});

	try
	{
		if (argc <= 1)
		{
			runall();
			return 0;
		}
		for (; (val = getopt_long (argc, argv, optstr, long_options.data(), 0)) != -1; )
		{
			switch (val)
			{
			case ':': case '?':
				std::cout << '\n';
				[[fallthrough]];
			case 'h':
				std::cout << "Usage: " << argv[0] << " [uri...] [-" << optstr << "]" << R"(
 -a parse all examples (default)
 -d [uri] parse uri from CLI, show debug output
 -D [uri] parse uri from CLI, show debug output - with normalize
 -h help
 -i interactive mode
 -V verbose uri output
 -l list tests
 -s show sizes
 -f [file] read and process from file
 -F [file] read and process from file - with normalize
 -T [num] static test to run
 -t [num] test to run
 -x special tests)" << '\n';
				return 1;
			case 'q': quiet ^= true; break;
			case 'V': verbose ^= true; break;
			case 'i': interactive ^= true; break;
			case 'x':
				break;
			case 'l':
				for (int ii{}; const auto& [src,vec] : tests)
					std::cout << ii++ << '\t' << src << " (" << std::string_view(src).size() << ")\n";
				break;
			case 'T':
				if (const auto tnum {std::stoul(optarg)}; tnum >= tests.size())
					throw std::range_error("invalid test case");
				else
					std::cout << (verbose ? uri::print_mode::detailed : uri::print_mode::default_mode) << uri_static<1024>{tests[tnum].first};
				break;
			case 't':
				if (const auto tnum {std::stoul(optarg)}; tnum >= tests.size())
					throw std::range_error("invalid test case");
				else
					std::cout << (verbose ? uri::print_mode::detailed : uri::print_mode::default_mode) << uri{tests[tnum].first};
				break;
			case 'D':
				{
					uri_decoded u1{std::string(optarg)};
					u1.normalize();
					if (!u1)
						std::cout << "error_t " << static_cast<int>(u1.get_error()) << '\n';
					std::cout << (verbose ? uri::print_mode::detailed : uri::print_mode::default_mode) << u1 << "bitset " << std::bitset<uri::countof>(u1.get_present()) << " ("
						<< std::hex << std::showbase << u1.get_present() << std::dec << std::noshowbase << ")\n";
					for (uri::component ii{}; ii != uri::countof; ii = uri::component(ii + 1))
					{
						if (u1.test(ii))
						{
							const auto [pos,len] { u1[ii] };
							std::cout << uri::get_name(ii) << ' ' << pos << " (" << len << ")\n";
						}
					}
				}
				break;
			case 'd':
				{
					uri u1{std::string(optarg)};
					if (decode)
						u1.normalize();
					if (!u1)
						std::cout << "error_t " << static_cast<int>(u1.get_error()) << '\n';
					std::cout << (verbose ? uri::print_mode::detailed : uri::print_mode::default_mode) << u1 << "bitset " << std::bitset<uri::countof>(u1.get_present()) << " ("
						<< std::hex << std::showbase << u1.get_present() << std::dec << std::noshowbase << ")\n";
					for (uri::component ii{}; ii != uri::countof; ii = uri::component(ii + 1))
					{
						if (u1.test(ii))
						{
							const auto [pos,len] { u1[ii] };
							std::cout << uri::get_name(ii) << ' ' << pos << " (" << len << ")\n";
						}
					}
				}
				break;
			case 'F':
				decode = true;
				[[fallthrough]];
			case 'f':
				if (std::ifstream ifs(optarg); ifs)
				{
					int cnt{}, longest{};
					std::vector<std::pair<uri::error_t, std::string>> err_uris{};
					char buff[4096];
					while (ifs.good())
					{
						if (ifs.getline(buff, sizeof(buff)); !ifs.fail())
						{
							std::string_view sv{buff};
							if (sv.size() > longest)
								longest = sv.size();
							/*
							if (sv.starts_with("//"))
								continue;
							if (sv.front() == '"')
								sv.remove_prefix(1);
							if (sv.ends_with(R"(",)"))
								sv.remove_suffix(2);
							*/
							//uri u1{decode ? uri::encode_hex_spaces(sv) : sv};
							uri u1{sv};
							if (decode)
								u1.normalize();
							if (!u1)
								err_uris.push_back(std::make_pair(u1.get_error(), std::string{sv}));
							else if (!quiet)
							{
								std::cout << (verbose ? uri::print_mode::detailed : uri::print_mode::default_mode) << u1 << "bitset " << std::bitset<uri::countof>(u1.get_present()) << " ("
									<< std::hex << std::showbase << u1.get_present() << std::dec << std::noshowbase << ")\n";
								for (uri::component ii{}; ii != uri::countof; ii = uri::component(ii + 1))
								{
									if (u1.test(ii))
									{
										const auto [pos,len] { u1[ii] };
										std::cout << uri::get_name(ii) << ' ' << pos << " (" << len << ")\n";
									}
								}
								std::cout << '\n';
							}
							++cnt;
						}
					}
					for(const auto& [err,ur] : err_uris)
						std::cout << static_cast<int>(err) << ": " << ur << '\n';
					std::cout << cnt << " uri(s) read from '" << optarg << "', " << err_uris.size() << " errors, longest uri was " << longest << '\n';
				}
				break;
			case 's':
				std::cout << "uri_base: " << sizeof(uri_base) << '\n';
				std::cout << "uri: " << sizeof(uri) << "\nuri_view: " << sizeof(uri_view) << '\n';
				std::cout << "uri_static<1024>: " << sizeof(uri_static<1024>) << '\n';
				break;
			case 'a':
				runall();
				break;
			default:
				break;
			}
		}
		if (interactive)
			do_interactive();
		while(optind < argc)
			std::cout << uri{argv[optind++]} << '\n';
	}
	catch (const std::exception& e)
	{
		std::cerr << "exception: " << e.what();
		if (optarg)
			std::cerr << " (-" << static_cast<char>(val) << ' ' << optarg << ')';
		std::cerr << std::endl;
		return 1;
	}
	return 0;
}

//-----------------------------------------------------------------------------------------
void do_interactive()
{
	for (std::string name;;)
	{
		std::cout << "Enter URI: ";
		std::getline(std::cin, name);
		if (name[0] == 'q')
			break;
		std::cout << '\n' << uri::print_mode::detailed << uri_view(uri::decode_hex(name)) << '\n';
	}
}

