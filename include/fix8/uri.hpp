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
//----------------------------------------------------------------------------------------
#ifndef FIX8_URI_HPP_
#define FIX8_URI_HPP_

//----------------------------------------------------------------------------------------
#include <iostream>
#include <iomanip>
#include <type_traits>
#include <array>
#include <span>
#include <string>
#include <string_view>
#include <stdexcept>
#include <utility>
#include <limits>
#include <cstdint>
#include <compare>
#include <functional>
#include <concepts>
#if __has_include(<format>)
# include <format>
#endif
#include <vector>
#include <list>
#include <algorithm>
#include <bit>

//-----------------------------------------------------------------------------------------
namespace FIX8 {

//-----------------------------------------------------------------------------------------
template<std::size_t sz>
requires(sz > 0)
class uri_storage
{
   std::array<char, sz> _buffer;
	std::size_t _sz{};

public:
   explicit constexpr uri_storage(std::string_view src) noexcept : _sz(src.size() > sz ? 0 : src.size())
      { std::copy_n(src.cbegin(), _sz, _buffer.begin()); }
   explicit constexpr uri_storage(std::string src) noexcept : uri_storage(std::string_view(src)) {}
   constexpr uri_storage(auto sv) noexcept : uri_storage(std::string_view(sv.begin(), sv.end())) {}
   constexpr uri_storage() = default;
   constexpr ~uri_storage() = default;

	constexpr std::string_view substr(std::string_view::size_type pos=0, std::string_view::size_type count=std::string_view::npos) const noexcept
		{ return { _buffer.data() + pos, count == std::string_view::npos || count > sz ? _sz : count }; }
   constexpr const char *data() const noexcept { return _buffer.data(); }
   constexpr std::size_t size() const noexcept { return _sz; }
	constexpr char& operator[](std::size_t pos) noexcept { return _buffer[pos]; }
	constexpr const char& operator[](std::size_t pos) const noexcept { return _buffer[pos]; }

   static constexpr auto max_size() noexcept { return sz; }
};

//-----------------------------------------------------------------------------------------
template<std::size_t sz>
requires(sz > 0)
class uri_storage_immutable
{
   const std::array<char, sz> _buffer;

   template<std::size_t... I>
   constexpr uri_storage_immutable(std::span<const char, sz> sv, std::index_sequence<I...>) noexcept : _buffer{sv[I]...} {}

public:
   constexpr uri_storage_immutable(auto sv) noexcept : uri_storage_immutable{sv, std::make_index_sequence<sz>{}} {}
   constexpr uri_storage_immutable() = delete;
   constexpr ~uri_storage_immutable() = default;

	constexpr std::string_view substr(std::string_view::size_type pos=0, std::string_view::size_type count=std::string_view::npos) const noexcept
		{ return { _buffer.data() + pos, count == std::string_view::npos || count > sz ? sz : count }; }
   constexpr const char *data() const noexcept { return _buffer.data(); }
   constexpr std::size_t size() const noexcept { return sz; }
	constexpr const char& operator[](std::size_t pos) const noexcept { return _buffer[pos]; }
   static constexpr auto max_size() noexcept { return sz; }
};

//-----------------------------------------------------------------------------------------
template <typename T>
concept is_mutable = requires(T a) { { a[0] = 'c' }; };

//-----------------------------------------------------------------------------------------
struct uri_common
{
	std::uint16_t _present{};

	using value_pair = std::pair<std::string_view, std::string_view>;
	using query_result = std::vector<value_pair>;
	using uri_len_t = std::uint16_t;
	using range_pair = std::pair<uri_len_t, uri_len_t>; // offset, len
	enum component { scheme, authority, userinfo, user, password, host, port, path, query, fragment, countof };
	enum class error_t : uri_len_t { no_error, too_long, illegal_chars, empty_src, countof };
	enum class scheme_t
	{
		cassandra, dns, elasticsearch, ftp, ftps, git, http, https, imap, imaps, ldap, ldaps, mongodb, mqtt, mqtt_tls,
		mysql, ntp, pop3, pop3s, postgresql, rdp, redis, sftp, sip, sip_tls, smtp, smtps, sqlserver, ssh, telnet,
		tftp, xmpp, countof
	};
	using comp_pair = std::pair<component, std::string_view>;
	using comp_list = std::vector<std::string_view>;
	using segments = comp_list;
	using port_pair = value_pair;

	static constexpr int all_components {(1 << countof) - 1};
	static constexpr auto uri_max_len {std::numeric_limits<uri_len_t>::max()};
	constexpr int count() const noexcept { return std::popcount(_present); } // upgrade to std::bitset when constexpr in c++23
	constexpr std::uint16_t get_present() const noexcept { return _present; }
	constexpr void set(component what) noexcept
		{ what == countof ? _present = all_components : _present |= (1 << what); }

	template<component what>
	constexpr void set() noexcept
	{
		if constexpr (what < countof)
			_present |= (1 << what);
		else
			_present = all_components;
	}
	constexpr void clear(component what) noexcept
		{ what == countof ? _present = 0 : _present &= ~(1 << what); }

	template<component what>
	constexpr void clear() noexcept
	{
		if constexpr (what < countof)
			_present &= ~(1 << what);
		else
			_present = 0;
	}
	constexpr bool test(component what) const noexcept { return _present & (1 << what); }

	template<component what>
	constexpr bool test() const noexcept { return _present & (1 << what); }

	template<component... comp>
	constexpr int test_any() const noexcept { return (... || test<comp>()); }

	template<component... comp>
	constexpr int test_all() const noexcept { return (... && test<comp>()); }

	template<component... comp>
	constexpr void set_all() noexcept { (set<comp>(),...); }

	template<component... comp>
	constexpr void clear_all() noexcept { (clear<comp>(),...); }

	constexpr bool has_any() const noexcept { return _present; }
	constexpr bool has_any_authority() const noexcept
		{ return test_any<host,password,port,user,userinfo>(); }
	constexpr bool has_any_userinfo() const noexcept
		{ return test_any<password,user>(); }

	// syntactic sugar has
#define df(x) \
	constexpr bool has_##x() const noexcept { return test<x>(); }
	df(scheme); df(authority); df(userinfo); df(user); df(password); df(host); df(port); df(path); df(query); df(fragment);
#undef df

	template<typename... Comp>
	static constexpr int bitsum(Comp... comp) noexcept { return (... | (1 << comp)); }

	template<component... comp>
	static constexpr int bitsum() noexcept { return (... | (1 << comp)); }

	template<component what>
	static constexpr bool has_bit(auto totest) noexcept
	{
		if constexpr (what < countof)
			return totest & (1 << what);
		else
			return false;
	}
};

//-----------------------------------------------------------------------------------------
template<typename T>
class basic_uri : public uri_common
{
	T _source;
	std::array<range_pair, component::countof> _ranges{};

	static constexpr std::array _component_names { "scheme", "authority", "userinfo", "user", "password", "host", "port", "path", "query", "fragment", };
	static constexpr std::string_view _hexds { "0123456789ABCDEF" }, _reserved { ":/?#[]@!$&'()*+,;=" };
	static constexpr std::array _default_ports { std::to_array<port_pair>
	({
		{ "cassandra", "9042" },{ "dns", "53" }, 				{ "elasticsearch", "9200" }, 	{ "ftp", "21" },		{ "ftps", "990" },	{ "git", "9418" },
		{ "http", "80" },			{ "https", "443" },			{ "imap", "143" }, 				{ "imaps", "993" }, 	{ "ldap", "389" },	{ "ldaps", "636" },
		{ "mongodb", "27017" },	{ "mqtt", "1883" },			{ "mqtt-tls", "8883" }, 		{ "mysql", "3306" }, { "ntp", "123" },		{ "pop3", "110" },
		{ "pop3s", "995" }, 		{ "postgresql", "5432" }, 	{ "rdp", "3389" }, 				{ "redis", "6379" }, { "sftp", "22" },		{ "sip", "5060" },
		{ "sip-tls", "5061" },	{ "smtp", "25" },				{ "sqlserver", "1433" },		{ "ssh", "22" }, 		{ "telnet", "23" },	{ "tftp", "69" },
		{ "xmpp ", "5222" },
	})};

	static constexpr bool is_reserved(char c) noexcept { return _reserved.find_first_of(c) != std::string_view::npos; }
	static constexpr bool is_unreserved(char c) noexcept { return std::isalnum(c) || c == '-' || c == '.' || c == '_' || c == '~'; }
	static constexpr bool is_unreserved_ascii(std::string_view src) noexcept // is %XX unreserved?
		{ return is_unreserved(cvt_hex_octet(src[1]) << 4 | cvt_hex_octet(src[2])); }
	static constexpr bool query_comp(const value_pair& pl, const value_pair& pr) noexcept { return pl.first < pr.first; }
	template<typename Fn>
	static constexpr bool tuple_comp(const std::tuple<component, Fn>& pl, const std::tuple<component, Fn>& pr) noexcept
		{ return std::get<component>(pl) < std::get<component>(pr); }
	static constexpr char cvt_hex_octet(char c) noexcept { return (c & 0xF) + (c >> 6) * 9; }
	static constexpr std::string& decode_to(std::string& result, bool unreserved) // inplace decode
	{
		for (std::string_view::size_type fnd{}; ((fnd = find_hex(result, fnd))) != std::string_view::npos;)
			if (unreserved ? is_unreserved_ascii(result.substr(fnd, 3)) : true)
				result.replace(fnd, 3, 1, cvt_hex_octet(result[fnd + 1]) << 4 | cvt_hex_octet(result[fnd + 2]));
			else
				fnd += 3;
		return result;
	}

	static constexpr uint32_t ipv4_to_uint32(std::string_view sv)
	{
		uint32_t result{};
		for(size_t start{}, segidx{}, end; start < sv.size() && segidx < 4; start = end + 1, ++segidx)
		{
			if ((end = sv.find('.', start)) == std::string_view::npos)
				end = sv.size();
			auto seg { sv.substr(start, end - start) };
			int val;
			if (std::from_chars(seg.data(), seg.data() + seg.size(), val).ec == std::errc{} && val >= 0 && val <= 255)
				result |= static_cast<uint32_t>(val) << (3 - segidx) * 8;
		}
		return result;
	}
	static constexpr bool is_valid_ipv4(std::string_view sv)
	{
		int segs{};
		for(size_t start{}, end; start < sv.size(); start = end + 1)
		{
			if (++segs > 4)
				return false;
			if ((end = sv.find('.', start)) == std::string_view::npos)
				end = sv.size();
			auto seg { sv.substr(start, end - start) };
			if (seg.empty() || seg.size() > 3 || !std::all_of(seg.begin(), seg.end(), ::isdigit))
				return false;
			int val;
			if ((std::from_chars(seg.data(), seg.data() + seg.size(), val).ec != std::errc{} || val < 0 || val > 255)
				|| (seg.size() > 1 && seg[0] == '0'))
					return false;
		}
		return segs == 4;
	}

public:
   constexpr basic_uri(std::string_view src) noexcept : _source{src} { parse(); }
   constexpr basic_uri(const char *src) noexcept : basic_uri{std::string_view{src}} {}
   constexpr basic_uri(std::string src) noexcept : _source{src} { parse(); }
	template<std::size_t sz>
   explicit constexpr basic_uri(std::span<const char, sz> sv) noexcept : _source{sv} { parse(); }
   constexpr basic_uri() = default;
   constexpr ~basic_uri() = default;

   constexpr std::string_view as_string_view() const noexcept
		requires(std::derived_from<std::string_view,T> || std::derived_from<std::string,T>) { return _source; }
   constexpr std::string_view as_string_view() const noexcept { return {_source.data(), _source.size()}; }
   constexpr T& get_source() const noexcept { return _source; }
   constexpr const char *data() const noexcept { return _source.data(); }
   constexpr auto size() const noexcept { return _source.size(); }
	constexpr int assign(T src) noexcept requires is_mutable<T>
	{
		_source = src;
		clear<countof>();
		return parse();
	}

   static constexpr auto max_size() noexcept
		requires (requires { { T::max_size() } -> std::same_as<size_t>; }) { return T::max_size(); }
   constexpr auto max_size() const noexcept
		requires (requires(T t) { { t.max_size() } -> std::same_as<typename T::size_type>; }) { return _source.max_size(); }

	constexpr std::string_view get_uri() const noexcept { return as_string_view(); }

	template<component what>
	constexpr std::string_view get_component() const noexcept
	{
		if constexpr (what < countof)
			return as_string_view().substr(_ranges[what].first, _ranges[what].second);
		else
			return std::string_view();
	}
	constexpr std::string_view get_component(component what) const noexcept
		{ return what < countof ? as_string_view().substr(_ranges[what].first, _ranges[what].second) : std::string_view(); }

	/*! Provides const direct access to the offset and length of the specifed component and is used to create a `std::string_view`.
	  	\param idx index into table
		\return a `const range_pair&` which is a `std::pair<uri_len_t, uri_len_t>&` to the specified component at the index given in the ranges table. */
	constexpr const range_pair& operator[](component idx) const noexcept { return _ranges[idx]; }

	template<component what>
	constexpr const range_pair& at() const noexcept { return _ranges[what]; }

	/*! Provides direct access to the offset and length of the specifed component and is used to create a `std::string_view`. USE CAREFULLY.
	  	\param idx index into table
		\return a `range_pair&` which is a `std::pair<uri_len_t, uri_len_t>&` to the specified component at the index given in the ranges table. */
	constexpr range_pair& operator[](component idx) noexcept { return _ranges[idx]; }

	template<component what>
	constexpr range_pair& at() noexcept { return _ranges[what]; }

	constexpr operator bool() const noexcept { return count(); }
	constexpr error_t get_error() const noexcept
		{ return has_any() ? error_t::no_error : static_cast<error_t>(_ranges[0].first); }
	constexpr void set_error(error_t what) noexcept
	{
		if (!has_any())
			_ranges[0].first = static_cast<uri_len_t>(what);
	}

	// syntactic sugar get,add,remove
#define df(x) \
	constexpr std::string_view get_##x() const noexcept { return get_component<x>(); } \
	constexpr auto& add_##x(std::string_view what) requires is_mutable<T> { return edit({{x, what}}); } \
	constexpr auto& remove_##x() requires is_mutable<T> { return edit({{x, ""}}); }
	df(scheme); df(authority); df(userinfo); df(user); df(password); df(host); df(port); df(path); df(query); df(fragment);
#undef df

	constexpr int parse() noexcept
	{
		using namespace std::literals::string_view_literals;
		auto svsrc { as_string_view() };
		while(true)
		{
			if (svsrc.empty())
				set_error(error_t::empty_src);
			else if (svsrc.size() > uri_max_len)
				set_error(error_t::too_long);
			else if (svsrc.find_first_of(" \t\n\f\r\v"sv) != std::string_view::npos)
			{
				auto qur { svsrc.find_first_of('?') }, sps { svsrc.find_first_of(' ') };
				if (qur != std::string_view::npos && sps != std::string_view::npos && qur < sps)
					break;
				set_error(error_t::illegal_chars);
			}
			else
				break;
			return 0;	// refuse to parse
		}
		std::string_view::size_type pos{}, hst{}, pth{std::string_view::npos};
		if (const auto sch {svsrc.find_first_of(':')}; sch != std::string_view::npos)
		{
			_ranges[scheme] = {0, sch};
			set<scheme>();
			pos = sch + 1;
		}
		bool scq{};
		if (svsrc[pos] == '?')	// short circuit query eg. magnet
			scq = true;
		else if (auto auth {svsrc.find("//"sv, pos)}; auth != std::string_view::npos)
		{
			auth += 2;
			if ((pth = svsrc.find_first_of('/', auth)) == std::string_view::npos) // unterminated path
				pth = svsrc.size();
			_ranges[authority] = {auth, pth - auth};
			set<authority>();
			if (const auto usr {svsrc.find_first_of('@', auth)}; usr != std::string_view::npos && usr < pth)
			{
				if (const auto pw {svsrc.find_first_of(':', auth)}; pw != std::string_view::npos && pw < usr) // no nested ':' before '@'
				{
					_ranges[user] = {auth, pw - auth};
					if (usr - pw - 1 > 0)
					{
						_ranges[password] = {pw + 1, usr - pw - 1};
						set<password>();
					}
				}
				else
					_ranges[user] = {auth, usr - auth};
				_ranges[userinfo] = {auth, usr - auth};
				set_all<userinfo,user>();
				hst = pos = usr + 1;
			}
			else
				hst = pos = auth;

			if (auto prt { svsrc.find_first_of(':', pos) }; prt != std::string_view::npos)
			{
				if (auto autstr {get_component<authority>()}; autstr.front() != '[' && autstr.back() != ']')
				{
					++prt;
					if (svsrc.size() - prt > 0)
					{
						_ranges[port] = {prt, svsrc.size() - prt};
						set<port>();
					}
				}
			}
		}
		if (pth != std::string_view::npos)
		{
			if (has_port())
			{
				if (pth - _ranges[port].first == 0) // remove empty port
					clear<port>();
				else
					_ranges[port].second = pth - _ranges[port].first;
				_ranges[host] = {hst, _ranges[port].first - 1 - hst};
			}
			else
				_ranges[host] = {hst, pth - hst};
			if (_ranges[host].second)
				set<host>();
			_ranges[path] = {pth, svsrc.size() - pth};
			set<path>();
		}
		if (pth == std::string_view::npos && !scq)
		{
			set<path>();
			if ((pth = svsrc.find_first_of('/', pos)) != std::string_view::npos)
				_ranges[path] = {pth, svsrc.size() - pth};
			else if (has_scheme())
				_ranges[path] = {pos, svsrc.size() - pos};
			else
				clear<path>();
		}
		if (const auto qur {svsrc.find_first_of('?', pos)}; qur != std::string_view::npos)
		{
			if (has_path())
				_ranges[path].second = qur - _ranges[path].first;
			_ranges[query] = {qur + 1, svsrc.size() - qur};
			set<query>();
		}
		if (const auto fra {svsrc.find_first_of('#', pos)}; fra != std::string_view::npos)
		{
			if (has_query())
				_ranges[query].second = fra - _ranges[query].first;
			_ranges[fragment] = {fra + 1, svsrc.size() - fra};
			set<fragment>();
		}
		return count();
	}

	template<char valuepair='&',char valueequ='='>
	constexpr query_result decode_query(bool sort=false) const noexcept
	{
		query_result result;
		if (has_query())
		{
			constexpr auto decpair([](std::string_view src) noexcept ->value_pair
			{
				if (auto fnd { src.find_first_of(valueequ) }; fnd != std::string_view::npos)
					return {src.substr(0, fnd), src.substr(fnd + 1)};
				else if (src.size())
					return {src, ""};
				return {};
			});
			std::string_view src{get_component<query>()};
			for (std::string_view::size_type pos{};;)
			{
				if (auto fnd { src.find_first_of(valuepair, pos) }; fnd != std::string_view::npos)
				{
					result.emplace_back(decpair(src.substr(pos, fnd - pos)));
					pos = fnd + 1;
					continue;
				}
				if (pos < src.size())
					result.emplace_back(decpair(src.substr(pos, src.size() - pos)));
				break;
			}
			if (sort)
				sort_query(result);
		}
		return result;
	}

	constexpr int in_range(std::string_view::size_type pos) const noexcept
	{
		int result{};
		for (int comp{}; const auto [start,len] : _ranges)
		{
			if (pos >= start && pos < start + len)
				result |= 1 << comp;
			++comp;
		}
		return result;
	}

	constexpr segments decode_segments(bool filter=true) const noexcept
	{
		segments result;
		if (has_path())
		{
			constexpr auto decruntil([](std::string_view src) noexcept ->std::string_view
			{
				if (auto fnd { src.find_first_of('/') }; fnd != std::string_view::npos)
					return src.substr(0, fnd);
				else if (src.size())
					return src;
				return {};
			});
			std::string_view src{get_component<path>()};
			using namespace std::literals::string_view_literals;
			for (std::string_view::size_type pos{};;)
			{
				if (filter && src.substr(pos, 2) == "./"sv)
					++pos;
				if (auto fnd { src.find_first_of('/', pos) }; fnd != std::string_view::npos)
				{
					if (auto r1 { decruntil(src.substr(pos, fnd - pos)) }; r1.size() || pos)
						result.emplace_back(r1);
					pos = fnd + 1;
					continue;
				}
				if (pos <= src.size())
					result.emplace_back(decruntil(src.substr(pos, src.size() - pos)));
				break;
			}
		}
		return result;
	}

   /// for_each
   template<typename Fn, typename... Args>
   requires std::invocable<Fn&&, comp_pair, Args...>
   [[maybe_unused]] constexpr auto for_each(Fn&& func, Args&&... args) const noexcept
   {
		for (component ii{}; ii != countof; ii = component(ii + 1))
			if (test(ii))
				std::invoke(std::forward<Fn>(func), comp_pair(ii, get_component(ii)), std::forward<Args>(args)...);
      return std::bind(std::forward<Fn>(func), std::placeholders::_1, std::forward<Args>(args)...);
   }

   template<typename Fn, typename C, typename... Args> // specialisation for member function with object
   requires std::invocable<Fn&&, C, comp_pair, Args...>
   [[maybe_unused]] constexpr auto for_each(Fn&& func, C *obj, Args&&... args) const noexcept
   {
      return for_each(std::bind(std::forward<Fn>(func), obj, std::placeholders::_1, std::forward<Args>(args)...));
   }

	// dispatch
	template<std::size_t I, typename Fn, typename... Args>
	requires std::invocable<Fn&&, basic_uri, component, Args...>
	[[maybe_unused]] constexpr int dispatch(const std::array<std::tuple<component, Fn>, I>& disp, Args&&... args) noexcept
	{
		return dispatcher(disp, nullptr, std::forward<Args>(args)...);
	}

	template<std::size_t I, typename Fn, typename C, typename... Args> // specialisation for member function
	requires std::invocable<Fn&&, C, basic_uri, component, Args...>
	[[maybe_unused]] constexpr int dispatch(const std::array<std::tuple<component, Fn>, I>& disp, C *obj, Args&&... args) noexcept
	{
		return dispatcher(disp, obj, std::forward<Args>(args)...);
	}

private:
	template<std::size_t I, typename Fn, typename C, typename... Args>
	[[maybe_unused]] constexpr int dispatcher(const std::array<std::tuple<component, Fn>, I>& disp, C *obj, Args&&... args) noexcept
	{
		if (disp.empty())
			return 0;
		uri_common done;
		int called{};
		for (const auto& [comp,func] : disp)
		{
			if (test(comp))
			{
				if constexpr(std::same_as<C, std::nullptr_t>)
					std::invoke(func, *this, comp, std::forward<Args>(args)...);
				else
					std::invoke(func, obj, *this, comp, std::forward<Args>(args)...);
				++called;
				done.set(comp);
			}
		}
		if (const auto& [lastcomp,lastfunc] {*std::prev(disp.cend())}; lastcomp == countof && done._present != _present)
		{
			for (component ii{}; ii != countof; ii = component(ii + 1))
			{
				if (!done.test(ii) && test(ii))
				{
					if constexpr(std::same_as<C, std::nullptr_t>)
						std::invoke(lastfunc, *this, ii, std::forward<Args>(args)...);
					else
						std::invoke(lastfunc, obj, *this, ii, std::forward<Args>(args)...);
					++called;
				}
			}
		}
		return called;
	}

	constexpr std::string make_edit(std::initializer_list<comp_pair> from) const noexcept
	{
		uri_common ibase;
		std::unordered_map<component, std::string_view> ilmap;
		for_each([&ibase,&ilmap](comp_pair comp)
		{
			const auto [cp, str] { comp };
			ibase.set(cp);
			ilmap.emplace(cp, str);
		});
		for (const auto& [comp,str] : from)
		{
			if (comp < countof)
			{
				ibase.set(comp);
				ilmap[comp] = str;
			}
		}
		if (!ibase.has_any())
			return {};
		if (ibase.has_authority())
		{
			if (auto itr{ilmap.find(authority)}; itr != ilmap.cend() && itr->second.empty())
			{
				ibase.clear<userinfo>();
				ibase.clear<authority>();
				ibase.set(host);
				ibase.set(port);
				ibase.set(user);
				ibase.set(password);
				ilmap[user] = ilmap[password] = ilmap[host] = ilmap[port] = "";
			}
		}
		if (ibase.has_any_authority())
			ibase.clear<authority>();
		if (ibase.has_userinfo() && has_any_userinfo())
		{
			if (auto itr{ilmap.find(userinfo)}; itr != ilmap.cend() && itr->second.empty())
			{
				ibase.clear<userinfo>();
				ibase.set(user);
				ilmap[user] = "";
				ibase.set(password);
				ilmap[user] = ilmap[password] = "";
			}
		}
		comp_list ilist{countof};
		for(const auto& [comp,str] : ilmap)
			ilist[comp] = str;
		return make_uri(ibase, std::move(ilist));
	}

public:
	constexpr std::string replace(std::string src) noexcept requires is_mutable<T>
	{
		std::string old{as_string_view()};
		assign(src);
		return old;
	}
	constexpr auto& edit(std::initializer_list<comp_pair> from) noexcept requires is_mutable<T>
	{
		assign(make_edit(std::move(from)));
		return *this;
	}
	constexpr auto normalize() requires is_mutable<T> { return replace(normalize_str(this->get_uri())); }
	constexpr auto normalize_http() requires is_mutable<T> { return replace(normalize_http_str(this->get_uri())); }

	constexpr bool host_is_ipv4() const noexcept { return has_host() && is_valid_ipv4(get_host()); }
	constexpr uint32_t host_as_ipv4() const noexcept { return host_is_ipv4() ? ipv4_to_uint32(get_host()) : 0; };

	constexpr std::ostream& print(std::ostream& os) const
	{
		if (!count())
			os << "error_t: " << static_cast<int>(get_error()) << '\n';
		os << std::setw(12) << std::left << "uri" << as_string_view() << " (" << size() << ")\n";
		for_each([&os,this](comp_pair cp)
		{
			os << std::setw(12) << std::left << get_name(std::get<component>(cp))
				<< (!std::get<1>(cp).empty() ? std::get<1>(cp) : "(empty)") << '\n';
			if (std::get<component>(cp) == path)
				if (const auto presult { decode_segments() }; presult.size() > 1)
					for (const auto tag : presult)
						os << "   " << (!tag.empty() ? tag : "(empty)") << '\n';
			if (std::get<component>(cp) == query)
				if (const auto qresult { decode_query() }; qresult.size() > 1)
					for (const auto [tag,value] : qresult)
						os << "   " << std::setw(12) << std::left << tag << (!value.empty() ? value : "(empty)") << '\n';
		});
		return os;
	}

	/// static methods --------------------------------------------------------------------------------
	static constexpr void sort_query(query_result& query) noexcept { std::sort(query.begin(), query.end(), query_comp); }
	static constexpr std::string_view find_port(std::string_view what) noexcept
	{
		const auto result { std::equal_range(_default_ports.cbegin(), _default_ports.cend(), value_pair(what, std::string_view()), query_comp) };
		return result.first == result.second ? std::string_view() : result.first->second;

	}
	static constexpr std::string_view find_query(std::string_view what, const query_result& from) noexcept
	{
		const auto result { std::equal_range(from.cbegin(), from.cend(), value_pair(what, std::string_view()), query_comp) };
		return result.first == result.second ? std::string_view() : result.first->second;

	}
	static constexpr std::string_view::size_type find_hex(std::string_view src, std::string_view::size_type pos=0) noexcept
	{
		for (std::string_view::size_type fnd{pos}; ((fnd = src.find_first_of('%', fnd))) != std::string_view::npos && fnd + 2 < src.size(); ++fnd)
			if (std::isxdigit(static_cast<unsigned char>(src[fnd + 1])) && std::isxdigit(static_cast<unsigned char>(src[fnd + 2])))
				return fnd;
		return std::string_view::npos;
	}
	static constexpr bool has_hex(std::string_view src) noexcept { return find_hex(src) != std::string_view::npos; }
	static constexpr std::string decode_hex(std::string_view src, bool unreserved=false)
	{
		std::string result{src};
		decode_to(result, unreserved);
		return result;
	}
	static constexpr std::string& decode_hex(std::string& result, bool unreserved=false) // inplace decode
		{ return decode_to(result, unreserved); }
	static constexpr std::string_view get_name(component what) noexcept
		{ return what < countof ? _component_names[what] : std::string_view(); }

	template<component what>
	static constexpr std::string_view get_name() noexcept
	{
		if constexpr (what < countof)
			return _component_names[what];
		else
			return std::string_view();
	}
	static constexpr std::string normalize_str(std::string_view src, int components=all_components)
	{
		using namespace std::literals::string_view_literals;
		constexpr auto isup([](std::string_view src) noexcept ->bool
			{ return std::any_of(std::cbegin(src), std::cend(src), [](const auto c) noexcept { return std::isupper(c); }); });
		constexpr auto tolo([](auto c) noexcept ->auto { return std::tolower(c); });

		std::string result{src};
		basic_uri bu{result};
		auto sch { bu.get_scheme() }, hst { bu.get_host() };
		if (has_bit<scheme>(components) && isup(sch)) // 1. scheme => lower case
			transform(sch.begin(), sch.end(), std::string::iterator(result.data() + bu[scheme].first), tolo);
		if (has_bit<host>(components) && isup(hst)) // 2. host => lower case
			transform(hst.begin(), hst.end(), std::string::iterator(result.data() + bu[host].first), tolo);
		if (has_hex(result))
		{
			for (std::string_view::size_type hv, pos{}; (hv = find_hex(result, pos)) != std::string_view::npos; pos += 3) // 3. %hex => upper case
				for (auto pos{hv + 1}; pos < hv + 3; ++pos)
					if (std::islower(result[pos]))
						result[pos] = std::toupper(result[pos]);
			decode_to(result, true); // 5. Decode unreserved hex
			bu.assign(result);
		}
		if (has_bit<port>(components) && !bu.has_port() && bu.get_authority().back() == ':') // remove unused port ':'
		{
			result.erase(bu[authority].first + bu[authority].second - 1, 1);
			bu.assign(result);
		}
		if (has_bit<path>(components))
		{
			if (auto segs { bu.decode_segments(false) }; !segs.empty()) // 6. Remove Dot Segments
			{
				for (auto itr{segs.cbegin()}; itr != segs.cend();)
				{
					if (*itr == "."sv)
						itr = segs.erase(itr);
					else if (*itr == ".."sv)
					{
						if (itr != segs.cbegin())
							itr = segs.erase(itr - 1);
						itr = segs.erase(itr);
					}
					else
						++itr;
				}
				std::string nspath;
				for (const auto pp : segs)
				{
					if (!pp.empty())
					{
						nspath += '/';
						nspath += pp;
					}
				}
				if (nspath.empty())
					nspath += '/';

				if (nspath != bu.get_path())
					result.replace(std::string::iterator(result.data() + bu[path].first),
						std::string::iterator(result.data() + bu[path].first + bu[path].second), nspath);
			}
		}
		bu.assign(result);
		if (has_bit<path>(components) && bu.has_any_authority() && bu.get_path().empty()) // 7. Convert empty path to "/"
			result += '/';
		return result;
	}
	static constexpr std::string normalize_http_str(std::string_view src)
	{
		auto result{normalize_str(src)};
		if (basic_uri bu{result}; bu.has_port())
		{
			if (auto prec{basic_uri::find_port(bu.get_scheme())}; prec == bu.get_port()
			  && (prec == _default_ports[static_cast<int>(scheme_t::http)].second
			  || prec == _default_ports[static_cast<int>(scheme_t::https)].second))
			{
				result.erase(bu[port].first - 1, bu[port].second + 1); // remove ":80"
				bu.assign(result);
				return std::string(bu.get_uri());
			}
		}
		return result;
	}

	static constexpr std::string encode_hex(std::string_view src)
	{
		std::string result;
		for (const auto pp : src)
		{
			if (is_reserved(pp) || std::isspace(pp) || !std::isprint(static_cast<unsigned char>(pp)))
				result += {'%', _hexds[pp >> 4], _hexds[pp & 0xF]};
			else
				result += pp;
		}
		return result;
	}

	static constexpr std::string encode_hex_spaces(std::string_view src)
	{
		std::string result{src};
		for(std::string::size_type pos{}; (pos = result.find_first_of(' ')) != std::string::npos;
			result.replace(pos, 1, "%20"));
		return result;
	}

	static constexpr std::string make_uri(std::initializer_list<comp_pair> from) noexcept
	{
		uri_common ibase;
		comp_list ilist{countof};
		for (const auto& [comp,str] : from)
		{
			if (comp < countof)
			{
				ibase.set(comp);
				ilist[comp] = str;
			}
		}
		return make_uri(ibase, std::move(ilist));
	}

	static constexpr auto factory(std::initializer_list<comp_pair> from) noexcept
		{ return basic_uri<std::string>(make_uri(std::move(from))); }

#if __has_include(<format>)
	/// format helper
	template<typename U=basic_uri, typename... Args>
	static constexpr auto format(std::format_string<Args...> fmt, Args&&... args)
		{ return U(std::vformat(fmt.get(), std::make_format_args(args...))); }
#endif

	friend constexpr bool operator==(const basic_uri& lhs, const basic_uri& rhs) noexcept
		{ return lhs._source == rhs._source; }
	/// normalize equality
	friend constexpr bool operator<=(const basic_uri& lhs, const basic_uri& rhs)
		{ return normalize_str(lhs.get_uri()) == normalize_str(rhs.get_uri()); }
	/// normalize_http equality
	friend constexpr bool operator%(const basic_uri& lhs, const basic_uri& rhs)
		{ return normalize_http_str(lhs.get_uri()) == normalize_http_str(rhs.get_uri()); }

	friend std::ostream& operator<<(std::ostream& os, const basic_uri& what) { return os << what.get_uri(); }

protected:
	static constexpr std::string make_uri(uri_common ibase, comp_list ilist) noexcept
	{
		if (!ibase.has_any())
			return {};
		using namespace std::literals::string_view_literals;
		uri_common done;
		std::string result;
		for (component ii{}; ii != countof; ii = component(ii + 1))
		{
			if (!ibase.test(ii) || done.test(ii))
				continue;
			switch(auto str{ilist[ii]}; ii)
			{
			case scheme:
				if (!str.empty())
				{
					result += str;
					result += ':';
					if (ibase.has_any_authority())
						result += "//"sv;
				}
				break;
			case authority:
				if (!ibase.has_any_authority())
					result += "//"sv;
				result += str;
				break;
			case userinfo:
				if (ibase.has_authority() || ibase.has_any_userinfo())
					continue;
				result += str;
				break;
			case user:
				if (str.empty() && ibase.test_any<authority,userinfo>())
					continue;
				result += str;
				break;
			case password:
				if (str.empty() && ibase.test_any<authority,userinfo>())
					continue;
				if (!str.empty())
				{
					result += ':';
					result += str;
				}
				break;
			case host:
				if (ibase.has_authority())
					continue;
				if ((!ilist[user].empty() || !ilist[password].empty()) && done.test_any<user,password>())
					result += '@';
				result += str;
				break;
			case port:
				if (ibase.has_authority())
					continue;
				if (!str.empty())
				{
					result += ':';
					result += str;
				}
				break;
			case path:
				result += str;
				break;
			case query:
				if (!str.empty())
				{
					result += '?';
					result += str;
				}
				break;
			case fragment:
				if (!str.empty())
				{
					result += '#';
					result += str;
				}
				break;
			default:
				continue;
			}
			done.set(ii);
		}
		return result;
	}
};

//-----------------------------------------------------------------------------------------
template<std::size_t sz=1024, typename T=uri_storage<sz>>
requires(sz > 0)
class uri_static : public basic_uri<T>
{
public:
   constexpr uri_static(auto sv) noexcept : basic_uri<T>{sv} {}
   constexpr uri_static() = default;
   constexpr ~uri_static() = default;
};

//-----------------------------------------------------------------------------------------
template<std::size_t sz, typename T=const uri_storage_immutable<sz>>
class uri_fixed : public basic_uri<T>
{
public:
   constexpr uri_fixed(std::span<const char, sz> sv) noexcept : basic_uri<T>{sv} {}
   constexpr uri_fixed() = delete;
   constexpr ~uri_fixed() = default;
};

template<std::size_t N>
uri_fixed(const char(&)[N]) -> uri_fixed<N>;

//-----------------------------------------------------------------------------------------
using uri_view = basic_uri<std::string_view>;
using uri = basic_uri<std::string>;

//-----------------------------------------------------------------------------------------
} // FIX8
#endif // FIX8_URI_HPP_
