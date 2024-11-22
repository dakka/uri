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
#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <unordered_set>
#include <sstream>
#if __has_include(<format>)
# include <format>
#endif
#include <vector>
#include <algorithm>
#include <string_view>
#include <fix8/uri.hpp>

//-----------------------------------------------------------------------------------------
using namespace FIX8;
using namespace std::literals::string_view_literals;

//-----------------------------------------------------------------------------------------
#include <uriexamples.hpp>

//-----------------------------------------------------------------------------------------
// run as: ctest --output-on-failure
//-----------------------------------------------------------------------------------------
TEST_CASE("get component")
{
	const uri u1{tests[0].first};
	REQUIRE_NOTHROW(u1.get_component(host));
	REQUIRE(u1.get_component(host) == "www.blah.com");
	REQUIRE(u1.get_component<host>() == "www.blah.com");
	REQUIRE(u1.get_component<fragment>() == "");
	REQUIRE(u1.get_component(countof) == "");
}

//-----------------------------------------------------------------------------------------
TEST_CASE("subscript operator")
{
	uri u1{tests[0].first};
	REQUIRE(u1.has_any());
	const auto [tag,value] { u1[host] };
	REQUIRE(tag == 8);
	REQUIRE(value == 12);
}

//-----------------------------------------------------------------------------------------
/*
TEST_CASE("bitset")
{
	uri u1{tests[0].first};
	REQUIRE(u1.get_present() == 0b0010100011);
	u1.clear<countof>();
	REQUIRE(u1.get_present() == 0);
	u1.set<uri::countof>();
	REQUIRE(u1.get_present() == 0b1111111111);
	basic_uri b1{0b1111111111};
	REQUIRE(b1.get_component(scheme) == "");
	REQUIRE(b1.get_component(host) == "");
	b1.clear<scheme>();
	REQUIRE(b1.get_present() == 0b1111111110);
}
*/

//-----------------------------------------------------------------------------------------
TEST_CASE("get name")
{
	REQUIRE_NOTHROW(uri::get_name(host));
	REQUIRE(uri::get_name<host>() == "host");
	REQUIRE(uri::get_name(scheme) == "scheme");
	REQUIRE(uri::get_name(countof) == "");
}

//-----------------------------------------------------------------------------------------
TEST_CASE("in range")
{
	const uri u1{"https://user:password@example.com:8080/path?search=1#frag"};
	//            0         1         2         3         4         5
	REQUIRE(u1.in_range(1) == uri::bitsum<scheme>());
	REQUIRE(u1.in_range(9) == uri::bitsum<authority, user, userinfo>());
	REQUIRE(u1.in_range(13) == uri::bitsum<authority, password, userinfo>());
	REQUIRE(u1.in_range(22) == uri::bitsum<authority, host>());
	REQUIRE(u1.in_range(34) == uri::bitsum<authority, port>());
	REQUIRE(u1.in_range(39) == uri::bitsum<path>());
	REQUIRE(u1.in_range(44) == uri::bitsum<query>());
	REQUIRE(u1.in_range(53) == uri::bitsum<fragment>());
}

//-----------------------------------------------------------------------------------------
TEST_CASE("test any/all range")
{
	const uri u1{"https://example.com/path?search=1"};
	REQUIRE_FALSE(u1.test_any<user, password, port>());
	REQUIRE(u1.test_all<scheme, host, path>());
	REQUIRE(u1.test_all<scheme, host, path, query, authority>());
	REQUIRE_FALSE(u1.test_all<scheme, user, path>());
	REQUIRE_FALSE(u1.test_all<scheme, user, path, userinfo>());
}

//-----------------------------------------------------------------------------------------
TEST_CASE("clear/set all range")
{
	uri u1{"https://example.com/path?search=1"};
	u1.clear_all<scheme, host, path>();
	REQUIRE(u1.test_all<query, authority>());
	REQUIRE_FALSE(u1.test_all<scheme, host, path>());
	u1.set_all<fragment, scheme, host, port>();
	REQUIRE(u1.test_all<fragment, scheme, host, port>());
}

//-----------------------------------------------------------------------------------------
void run_test_comp(int id, const auto& ui)
{
	const auto& vec { tests[id].second };
	INFO("uri: " << id); // << ' ' << uri{u1});
	INFO(ui);
	REQUIRE(ui.count() == vec.size());
	for (auto [comp,str] : vec)
	{
		INFO("component: " << comp);
		REQUIRE(ui.get_component(comp) == str);
	}
}

TEST_CASE("uri component validations")
{
	static const std::unordered_set<int> decode1st {12, 19, 26, 29, 30, 31, 35};
	for (int ii{}; ii < tests.size(); ++ii)
	{
		auto str{decode1st.contains(ii) ? uri::decode_hex(tests[ii].first, false) : tests[ii].first};
		INFO("test: uri");
		run_test_comp(ii, uri{str});
		REQUIRE(std::string_view(tests[ii].first).size() < uri_static<>::max_size());
		INFO("test: uri_static");
		run_test_comp(ii, uri_static<>{str});
	}
}

//-----------------------------------------------------------------------------------------
#define tf(var,x) (var.has_##x() == var.test(x) \
	&& var.get_##x() == var.get_component<x>() && var.get_##x() == var.get_component(x))

TEST_CASE("uri has/get")
{
	for (const auto& [src,vec] : tests)
	{
		const uri_view u1{src};
		REQUIRE(tf(u1, scheme));
		REQUIRE(tf(u1, authority));
		REQUIRE(tf(u1, userinfo));
		REQUIRE(tf(u1, user));
		REQUIRE(tf(u1, password));
		REQUIRE(tf(u1, host));
		REQUIRE(tf(u1, port));
		REQUIRE(tf(u1, path));
		REQUIRE(tf(u1, query));
		REQUIRE(tf(u1, fragment));
	}
}

//-----------------------------------------------------------------------------------------
TEST_CASE("has_(special cases)")
{
	const uri u1{tests[0].first};
	REQUIRE(u1.has_any());
	REQUIRE(u1.has_any_authority());
	REQUIRE_FALSE(u1.has_any_userinfo());
	const uri u2{tests[3].first};
	REQUIRE(u2.has_any());
	REQUIRE(u2.has_any_authority());
	REQUIRE(u2.has_any_userinfo());
	const uri u3{tests[33].first};
	REQUIRE_FALSE(u3.has_any());
	REQUIRE_FALSE(u3);
	REQUIRE_FALSE(u3.has_any_authority());
	REQUIRE_FALSE(u3.has_any_userinfo());
}

//-----------------------------------------------------------------------------------------
TEST_CASE("replace")
{
	const auto& [src,vec] { tests[0] };
	const auto& [src1,vec1] { tests[4] };
	uri u1{src};
	REQUIRE(u1.get_component(host) == "www.blah.com");
	uri u2{u1.replace(src1)};
	REQUIRE(u1.get_component(host) == "example.com");
	REQUIRE(u2.get_component(host) == "www.blah.com");

	uri_static<> u3{src};
	REQUIRE(u3.get_component(host) == "www.blah.com");
	uri_static<> u4{u3.replace(src1)};
	REQUIRE(u3.get_component(host) == "example.com");
	REQUIRE(u4.get_component(host) == "www.blah.com");
}

//-----------------------------------------------------------------------------------------
TEST_CASE("invalid uri")
{
	static constexpr auto baduris
	{
		std::to_array<uri_view>
		({
			"https://www.example.com\n"sv,
			"https://www.example.com\r"sv,
			"https://www. example.com"sv,
			"https://www.example.\tcom"sv,
			"https://www.example.\vcom"sv,
			"https://www.example.\fcom"sv,
		})
	};
	for (auto pp : baduris)
	{
		REQUIRE_FALSE(pp);
		REQUIRE(pp.get_error() == uri::error_t::illegal_chars);
	}
}

//-----------------------------------------------------------------------------------------
TEST_CASE("limits")
{
	char buff[uri::uri_max_len+1]{};
	std::fill(buff, buff + sizeof(buff), 'x');
	uri u1{buff};
	REQUIRE_FALSE(u1);
	REQUIRE(u1.get_error() == uri::error_t::too_long);
	uri_static<> u2{buff}; // too long
	REQUIRE(u2.get_uri() == "");
	uri_static<64> u3{tests[35].first};
	REQUIRE_FALSE(u3);
}

//-----------------------------------------------------------------------------------------
TEST_CASE("empty")
{
	uri u1{""};
	REQUIRE_FALSE(u1);
	REQUIRE(u1.get_error() == uri::error_t::empty_src);
}
//-----------------------------------------------------------------------------------------
TEST_CASE("ports")
{
	REQUIRE(uri::find_port("ftp") == "21");
	REQUIRE(uri::find_port("http") == "80");
	REQUIRE(uri::find_port("https") == "443");
	REQUIRE(uri::find_port("telnet") == "23");
}

//-----------------------------------------------------------------------------------------
TEST_CASE("normalization")
{
	static constexpr std::array uris
	{
		std::to_array<std::pair<std::string_view,std::string_view>>
		({
			 { "HTTPS://WWW.HELLO.COM/path/%62%6c%6f%67/%75%72%6c%73"sv, "https://www.hello.com/path/blog/urls"sv },
			 { "HTTPS://WWW.HELLO.COM/path/../this/./blah/blather/../end"sv, "https://www.hello.com/this/blah/end"sv },
			 { "https://www.buyexample.com/./begin/one-removed/../two-removed/../three-removed/../end?name=ferret&time=any#afrag"sv,
					"https://www.buyexample.com/begin/end?name=ferret&time=any#afrag"sv },
			 { "https://www.buyexample.com/.././.././"sv, "https://www.buyexample.com/"sv },
			 { "https://www.test.com"sv, "https://www.test.com/"sv },
			 { "https://www.nochange.com/"sv, "https://www.nochange.com/"sv },
			 { "https://www.hello.com/doc/../index.html"sv, "https://www.hello.com/index.html"sv },
			 { "http://www.hello.com:80/doc/../index.html"sv, "http://www.hello.com/index.html"sv },
			 { "https://www.hello.com:443/doc/../index.html"sv, "https://www.hello.com/index.html"sv },
			 { "https://www.hello.com:8080/doc/../index.html"sv, "https://www.hello.com:8080/index.html"sv },
			 { "https://www.hello.com/doc/../%69%6e%64%65%78%20file.html"sv, "https://www.hello.com/index%20file.html"sv },
		})
	};
	for (const auto [before, after] : uris)
	{
		if (before != after)
			REQUIRE(uri_view(before) != uri_view(after));
		REQUIRE(uri(uri::normalize_http_str(before)) == uri(after));
		uri u1{before};
		REQUIRE(u1.normalize_http() == before);
		REQUIRE(u1.get_uri() == after);
	}
}

//-----------------------------------------------------------------------------------------
TEST_CASE("normalization_http")
{
	static constexpr std::array uris
	{
		"https://www.test.com/"sv, // all should normalize to this one
		"https://www.test.com"sv,
		"https://www.test.com:/"sv,
		"https://www.test.com:443/"sv,
	};
	for (const auto control{uris[0]}; const auto pp : uris)
	{
		uri u1{pp};
		u1.normalize_http();
		REQUIRE(u1.get_uri() == control); // basic_uri equivalence operator
		uri u2{pp}, u3{control};
		REQUIRE(u2 % u3); // uri normalize_http equivalence operator
	}
}

//-----------------------------------------------------------------------------------------
TEST_CASE("print")
{
	static constexpr auto str
	{
R"(uri         http://nodejs.org:89/docs/latest/api/foo/bar/qua/13949281/0f28b/5d49/b3020/url.html?payload1=true&payload2=false&test=1&benchmark=3&foo=38.38.011.293&bar=1234834910480&test=19299&3992&key=f5c65e1e98fe07e648249ad41e1cfdb0#test (225)
scheme      http
authority   nodejs.org:89
host        nodejs.org
port        89
path        /docs/latest/api/foo/bar/qua/13949281/0f28b/5d49/b3020/url.html
   docs
   latest
   api
   foo
   bar
   qua
   13949281
   0f28b
   5d49
   b3020
   url.html
query       payload1=true&payload2=false&test=1&benchmark=3&foo=38.38.011.293&bar=1234834910480&test=19299&3992&key=f5c65e1e98fe07e648249ad41e1cfdb0
   payload1    true
   payload2    false
   test        1
   benchmark   3
   foo         38.38.011.293
   bar         1234834910480
   test        19299
   3992        (empty)
   key         f5c65e1e98fe07e648249ad41e1cfdb0
fragment    test
)"
	};

	std::ostringstream ostr;
	uri_view u1{tests[9].first};
	u1.print(ostr);
	REQUIRE(ostr.str() == str);
	ostr.str("");
	ostr << u1;
	REQUIRE(ostr.str() == tests[9].first);
}

//-----------------------------------------------------------------------------------------
TEST_CASE("decode hex")
{
	static constexpr std::array uris
	{
		"https://www.netmeister.org/%62%6C%6F%67/%75%72%6C%73.%68%74%6D%6C?!@#$%25=+_)(*&^#top%3C"sv,
		"https://www.netmeister.org/blog/urls.html?!@#$%=+_)(*&^#top<"sv,
		"https://www.netmeister.org/path#top%3"sv,
		"https://www.netmeister.org/%%62"sv,
		"https://www.netmeister.org/blog/urls.html?!@#$%=+_)(*&^#top<"sv,
		"https://www.netmeister.org/%62%6c%6f%67/%75%72%6c%73.%68%74%6d%6c?!@#$%25=+_)(*&^#top%3C"sv,
	};

	REQUIRE(uri::has_hex(uris[0]));
	REQUIRE_FALSE(uri::has_hex(uris[1]));
	REQUIRE_FALSE(uri::has_hex(uris[2]));
	auto result { uri::decode_hex(uris[0]) };
	REQUIRE_FALSE(uri::has_hex(result));
	uri u1{result};
	REQUIRE(u1.get_uri() == uris[1]);
	uri_view u2(uris[0]);
	REQUIRE(uri_view::has_hex(u2.get_uri()));
	REQUIRE(uri::has_hex(uris[3]));
	REQUIRE(uri::decode_hex(uris[0]) == uri::decode_hex(uris[5]));
}

//-----------------------------------------------------------------------------------------
TEST_CASE("encode hex")
{
	const std::string str {"/foo/" + uri_view::encode_hex("this path has embedded spaces") + "/test/node.js"};
	REQUIRE(str == "/foo/this%20path%20has%20embedded%20spaces/test/node.js"sv);
}

//-----------------------------------------------------------------------------------------
template<typename T>
void do_decode()
{
	static const typename T::query_result tbl
	{
		{ "payload1", "true" }, { "payload2", "false" }, { "test", "1" }, { "benchmark", "3" }, { "foo", "38.38.011.293" },
		{ "bar", "1234834910480" }, { "test", "19299" }, { "3992", "" }, { "key", "f5c65e1e98fe07e648249ad41e1cfdb0" },
	};
	const T u1{tests[9].first};
	auto result{u1.decode_query()};
	REQUIRE(tbl == result);
	const T u2{tests[8].first};
	auto result1{u2.decode_query()};
	REQUIRE(result1.empty());

	const T u3{ "http://host.com/?payload1:true;payload2:false;test:1;benchmark:3;foo:38.38.011.293"
		";bar:1234834910480;test:19299;3992;key:f5c65e1e98fe07e648249ad41e1cfdb0#test"};
	auto result2{u3.template decode_query<';',':'>()};
	REQUIRE(tbl == result2);
}

TEST_CASE("query decode")
{
	do_decode<uri>();
	do_decode<uri_static<>>();
}

//-----------------------------------------------------------------------------------------
template<typename T>
void do_segment()
{
	static const auto paths
	{
		std::to_array<std::pair<std::string_view, typename T::segments>>
		({
			{ "http://host.com/au/locator//area/file.txt", { { "au" }, { "locator" }, {}, { "area" }, { "file.txt" } } },
			{ "http://host.com/test//this", { { "test" }, {}, { "this" } } },
			{ "http://host.com/.//", { {}, {}, {} } },
			{ "http://host.com//./", { {}, {}, {} } },
		})
	};
	for (const auto& [src,tbl] : paths)
	{
		const T u1{src};
		auto result{u1.decode_segments()};
		REQUIRE(tbl == result);
	}
}

TEST_CASE("segment decode")
{
	do_segment<uri>();
	do_segment<uri_static<>>();
}

//-----------------------------------------------------------------------------------------
TEST_CASE("query search")
{
	static const uri::query_result tbl { { "first", "1st" }, { "second", "2nd" }, { "third", "3rd" } };
	const uri u1{tests[34].first};
	auto result{u1.decode_query(true)}; // auto sort - must be sorted to use find
	auto result1{u1.decode_query()}; // not sorted, sort later
	uri::sort_query(result1);
	REQUIRE(tbl == result);
	REQUIRE(uri::find_query("first", result) == "1st");
	REQUIRE(uri::find_query("second", result) == "2nd");
	REQUIRE(uri::find_query("third", result) == "3rd");
	REQUIRE(uri::find_query("fourth", result) == "");
	REQUIRE(result == result1);
}

//-----------------------------------------------------------------------------------------
template<typename T>
void do_factory()
{
	const auto u1 { T::factory({{scheme, "https"}, {user, "dakka"}, {host, "www.blah.com"}, {port, "3000"}, {path, "/"}}) };
	run_test_comp(3, u1);
	const auto u2 { T::factory({{scheme, "file"}, {authority, ""}, {path, "/foo/bar/test/node.js"}}) };
	run_test_comp(8, u2);
	const auto u3 { T::factory({{scheme, "mailto"}, {path, "John.Smith@example.com"}}) };
	run_test_comp(15, u3);
	const auto u4 { uri::factory({{scheme, "file"}, {authority, ""}, {path, "/foo/" + uri_view::encode_hex("this path has embedded spaces") + "/test/node.js"}}) };
	REQUIRE(u4.get_path() == "/foo/this%20path%20has%20embedded%20spaces/test/node.js"sv);
	const auto u5 { T::factory({{scheme, "https"}, {user, "user"}, {password, "password"}, {host, "example.com"}, {path, "/path"}, {query, "search=1"}})};
	run_test_comp(10, u5);
}

TEST_CASE("factory")
{
	do_factory<uri>();
	do_factory<uri_static<>>();
}

//-----------------------------------------------------------------------------------------
template<typename T>
void do_edit()
{
	T u1 { "https://dakka@www.blah.com:3000/" };
	u1.edit({{port, "80"}, {user, ""}, {path, "/newpath"}});
	REQUIRE(u1.get_uri() == "https://www.blah.com:80/newpath");

	T u2 { "file:///foo/bar/test/node.js" };
	u2.edit({{scheme, "mms"}, {fragment, "bookmark1"}});
	REQUIRE(u2.get_uri() == "mms:///foo/bar/test/node.js#bookmark1");

	T u3 { "https://user:password@example.com/?search=1" };
	u3.edit({{port, "80"}, {user, "dakka"}, {password, ""}, {path, "/newpath"}});
	REQUIRE(u3.get_uri() == "https://dakka@example.com:80/newpath?search=1");

	T u6 { "https://dakka:pass123@example.com/?search=1" };
	u6.edit({{user, ""}, {password, ""}});
	REQUIRE(u6.get_uri() == "https://example.com/?search=1");

	T u4 { "https://dakka:pass123@example.com/?search=1" };
	u4.edit({{userinfo, ""}});
	REQUIRE(u4.get_uri() == "https://example.com/?search=1");

	T u5 { "https://user@example.com/?search=1" };
	u5.edit({{port, "80"}, {userinfo, ""}});
	REQUIRE(u5.get_uri() == "https://example.com:80/?search=1");
}

TEST_CASE("edit")
{
	do_edit<uri>();
	do_edit<uri_static<>>();
}

//-----------------------------------------------------------------------------------------
template<typename T>
void do_add()
{
	T u1 { "https://dakka@www.blah.com:3000/" };
	u1.add_path("/newpath");
	REQUIRE(u1.get_uri() == "https://dakka@www.blah.com:3000/newpath");

	T u4 { "https://example.com/?search=1" };
	u4.add_userinfo("dakka:pass123@");
	REQUIRE(u4.get_uri() == "https://dakka:pass123@example.com/?search=1");
}

TEST_CASE("add")
{
	do_add<uri>();
	do_add<uri_static<>>();
}

//-----------------------------------------------------------------------------------------
template<typename T>
void do_remove()
{
	T u1 { "https://dakka@www.blah.com:3000/newpath" };
	u1.remove_port();
	REQUIRE(u1.get_uri() == "https://dakka@www.blah.com/newpath");

	T u4 { "https://dakka:pass123@example.com/?search=1" };
	u4.remove_userinfo();
	REQUIRE(u4.get_uri() == "https://example.com/?search=1");

	T u6 { "https://dakka:pass123@example.com/?search=1" };
	u6.remove_scheme();
	REQUIRE(u6.get_uri() == "dakka:pass123@example.com/?search=1");

	T u5 { "https://dakka:pass123@example.com/?search=1" };
	u5.remove_authority();
	REQUIRE(u5.get_uri() == "https:///?search=1");
	u5.remove_scheme();
	REQUIRE(u5.get_uri() == "/?search=1");
};

TEST_CASE("remove")
{
	do_remove<uri>();
	do_remove<uri_static<>>();
}

//-----------------------------------------------------------------------------------------
#if __has_include(<format>)
template<typename T>
void do_format()
{
	const auto u1 { uri::format<T>("{}://{}@{}:{}{}", "https", "dakka", "www.blah.com", "3000", "/") };
	run_test_comp(3, u1);
	const auto u2 { uri::format<T>("{}://{}", "file", "/foo/bar/test/node.js") };
	run_test_comp(8, u2);
	const auto u3 { uri::format<T>("{}:{}", "mailto", "John.Smith@example.com") };
	run_test_comp(15, u3);
	const auto u4 { uri::format<T>("{}:{}", "file", "/foo/" + uri_view::encode_hex("this path has embedded spaces") + "/test/node.js") };
	REQUIRE(u4.get_path() == "/foo/this%20path%20has%20embedded%20spaces/test/node.js"sv);
}

TEST_CASE("format")
{
	do_format<uri>();
	do_format<uri_static<>>();
}
#endif

//-----------------------------------------------------------------------------------------
TEST_CASE("uri_fixed")
{
	constexpr uri_fixed u1{"https://dakka@www.blah.com:3000/"};
	REQUIRE(u1.get_host() == "www.blah.com");

	constexpr uri_fixed u2
	{
		"http://nodejs.org:89/docs/latest/api/foo/bar/qua/13949281/0f28b/5d49/b3020/url.html"
		"?payload1=true&payload2=false&test=1&benchmark=3&foo=38.38.011.293"
		"&bar=1234834910480&test=19299&3992&key=f5c65e1e98fe07e648249ad41e1cfdb0#test"
	};
	REQUIRE(u2.get_port() == "89");
}

//-----------------------------------------------------------------------------------------
/*
TEST_CASE("uri_fixed")
{
	uri_fixed<"https://gitter.im/t1gor/Robots.txt-Parser-Class/archives"> test;
	REQUIRE(test.get_host() == "gitter.im");
}
*/

//-----------------------------------------------------------------------------------------
TEST_CASE("for_each")
{
	constexpr uri_fixed u1{"https://dakka@www.blah.com:3000/"};
	int count{};
	u1.for_each([](uri::comp_pair comp, int& cnt)
	{
		//std::cout << static_cast<int>(comp.first) << ": " << comp.second << '\n';
		++cnt;
	}, std::ref(count));
	REQUIRE(count == 7);
	count = 0;
}

//-----------------------------------------------------------------------------------------
TEST_CASE("dispatch")
{
	int called{};
	auto test([&called](const uri_view& ul, uri::component comp, std::string_view what)
	{
		++called;
		REQUIRE(ul.get_component(comp) == what);
	});
	using testfunc = decltype(test);

	struct foo
	{
		void host(const uri_view& ul, uri::component comp, testfunc func) const { func(ul, comp, "www.blah.com"); }
		void scheme(const uri_view& ul, uri::component comp, testfunc func) const { func(ul, comp, "https"); }
		void port(const uri_view& ul, uri::component comp, testfunc func) const { func(ul, comp, "3000"); }
		void path(const uri_view& ul, uri::component comp, testfunc func) const { func(ul, comp, "/stuff"); }
		void fragment(const uri_view& ul, uri::component comp, testfunc func) const { func(ul, comp, "not_called"); }
	};
	constexpr auto tarr
	{
		std::to_array<std::tuple<uri::component, void (foo::*)(const uri_view&, uri::component, testfunc) const>>
		({
			{ uri::component::host, &foo::host },
			{ uri::component::scheme, &foo::scheme },
			{ uri::component::port, &foo::port },
			{ uri::component::path, &foo::path },
			{ uri::component::fragment, &foo::fragment },
		})
	};
	foo bar;
	uri_view u1 { "https://dakka@www.blah.com:3000/stuff" };
	u1.dispatch(tarr, &bar, test);
	REQUIRE(called == 4);
}

//-----------------------------------------------------------------------------------------
TEST_CASE("dispatch with default")
{
	struct foo
	{
		int called{};
		std::vector<uri::component> default_called;

		void host(const uri_view& ul, uri::component comp) { ++called; }
		void scheme(const uri_view& ul, uri::component comp) { ++called; }
		void port(const uri_view& ul, uri::component comp) { ++called; }
		void path(const uri_view& ul, uri::component comp) { ++called; }
		void default_handler(const uri_view& ul, uri::component comp)
		{
			++called;
			default_called.push_back(comp);
		}
	};
	constexpr auto tarr
	{
		std::to_array<std::tuple<uri::component, void (foo::*)(const uri_view&, uri::component)>>
		({
			{ uri::component::host, &foo::host },
			{ uri::component::scheme, &foo::scheme },
			{ uri::component::port, &foo::port },
			{ uri::component::path, &foo::path },
			{ uri::component::countof, &foo::default_handler },
		})
	};
	foo bar;
	uri_view u1 { "https://dakka@www.blah.com:3000/stuff?first=that#extra" };
	REQUIRE(u1.dispatch(tarr, &bar) == 9);
	REQUIRE(bar.called == 9);
	REQUIRE(bar.default_called ==
		std::vector<uri::component>{uri::component::authority, uri::component::userinfo, uri::component::user, uri::component::query, uri::component::fragment});
}

//-----------------------------------------------------------------------------------------
TEST_CASE("host_as_ipv4")
{
	const uri_view u1{tests[18].first};
	REQUIRE(u1.host_is_ipv4());
	REQUIRE(u1.host_as_ipv4() == 3221226000);
	const uri_view u2{tests[0].first};
	REQUIRE_FALSE(u2.host_is_ipv4());
	REQUIRE(u2.host_as_ipv4() == 0);
}
