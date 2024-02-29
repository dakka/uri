//-----------------------------------------------------------------------------------------
//
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
//----------------------------------------------------------------------------------------
#ifndef FIX8_URI_HPP_
#define FIX8_URI_HPP_

//----------------------------------------------------------------------------------------
#include <iostream>
#include <iomanip>
#include <type_traits>
#include <array>
#include <string>
#include <string_view>
#include <stdexcept>
#include <utility>
#include <limits>
#include <cstdint>
#include <compare>
#include <vector>
#include <list>
#include <algorithm>
#include <bit>

//-----------------------------------------------------------------------------------------
namespace FIX8 {

//-----------------------------------------------------------------------------------------
class basic_uri
{
public:
	using value_pair = std::pair<std::string_view, std::string_view>;
	using query_result = std::vector<value_pair>;
	using uri_len_t = std::uint16_t;
	using range_pair = std::pair<uri_len_t, uri_len_t>; // offset, len
	enum component { scheme, authority, userinfo, user, password, host, port, path, query, fragment, countof };
	enum class error : uri_len_t { no_error, too_long, illegal_chars, empty_src, countof };
	enum class scheme_t { ftp, http, https, imap, ldap, smtp, telnet, countof };
	static constexpr const auto uri_max_len {std::numeric_limits<uri_len_t>::max()};
	using comp_pair = std::pair<component, std::string_view>;
	using comp_list = std::vector<std::string_view>;
	using segments = comp_list;
	using port_pair = value_pair;
private:
	std::string_view _source;
	std::array<range_pair, component::countof> _ranges{};
	std::uint16_t _present{};
	static constexpr const std::array _component_names { "scheme", "authority", "userinfo", "user", "password", "host", "port", "path", "query", "fragment", };
	static constexpr const char _hexds[] { "0123456789ABCDEF" };
	static constexpr const std::string_view _reserved { ":/?#[]@!$&'()*+,;=" };
	static constexpr const std::array _default_ports { std::to_array<port_pair>
		({ { "ftp", "21" }, { "http", "80" }, { "https", "443" }, { "imap", "143" }, { "ldap", "389" }, { "smtp", "25" }, { "telnet", "23" }, }) };
public:
	constexpr basic_uri(std::string_view src) noexcept : _source(src) { parse(); }
	constexpr basic_uri(int bits) noexcept : _present(bits) {}
	constexpr basic_uri() = default;
	~basic_uri() = default;

	constexpr int assign(std::string_view src)
	{
		_source = src;
		clear();
		return parse();
	}
	constexpr std::string_view get_uri() const noexcept { return _source; }
	constexpr std::string_view get(component what) const noexcept
		{ return _source.substr(_ranges[what].first, _ranges[what].second); }
	constexpr std::string_view get_component(component what) const noexcept
		{ return what < countof ? get(what) : std::string_view(); }

	/*! Provides const direct access to the offset and length of the specifed component and is used to create a `std::string_view`.
	  	\param idx index into table
		\return a `const range_pair&` which is a `std::pair<uri_len_t, uri_len_t>&` to the specified component at the index given in the ranges table. */
	constexpr const range_pair& operator[](component idx) const noexcept { return _ranges[idx]; }

	/*! Provides direct access to the offset and length of the specifed component and is used to create a `std::string_view`. USE CAREFULLY.
	  	\param idx index into table
		\return a `range_pair&` which is a `std::pair<uri_len_t, uri_len_t>&` to the specified component at the index given in the ranges table. */
	constexpr range_pair& operator[](component idx) noexcept { return _ranges[idx]; }

	constexpr int count() const noexcept { return std::popcount(_present); } // upgrade to std::bitset when constexpr in c++23
	constexpr std::uint16_t get_present() const noexcept { return _present; }
	constexpr void set(component what=countof) noexcept
		{ what == countof ? _present = (1 << countof) - 1 : _present |= (1 << what); }
	constexpr void clear(component what=countof) noexcept
		{ what == countof ? _present = 0 : _present &= ~(1 << what); }
	constexpr bool test(component what=countof) const noexcept
		{ return what == countof ? _present : _present & (1 << what); }
	constexpr operator bool() const noexcept { return count(); }
	constexpr error get_error() const noexcept
		{ return test() ? error::no_error : static_cast<error>(_ranges[0].first); }
	constexpr void set_error(error what) noexcept
	{
		if (!test())
			_ranges[0].first = static_cast<uri_len_t>(what);
	}
	// syntactic sugar has/get
	constexpr bool has_scheme() const noexcept { return test(scheme); }
	constexpr bool has_authority() const noexcept { return test(authority); }
	constexpr bool has_any_authority() const noexcept
		{ return _present & (1 << host | 1 << password | 1 << port | 1 << user | 1 << userinfo); }
	constexpr bool has_userinfo() const noexcept { return test(userinfo); }
	constexpr bool has_any_userinfo() const noexcept
		{ return _present & (1 << password | 1 << user); }
	constexpr bool has_user() const noexcept { return test(user); }
	constexpr bool has_password() const noexcept { return test(password); }
	constexpr bool has_host() const noexcept { return test(host); }
	constexpr bool has_port() const noexcept { return test(port); }
	constexpr bool has_path() const noexcept { return test(path); }
	constexpr bool has_query() const noexcept { return test(query); }
	constexpr bool has_fragment() const noexcept { return test(fragment); }
	constexpr std::string_view get_scheme() const noexcept { return get(scheme); }
	constexpr std::string_view get_authority() const noexcept { return get(authority); }
	constexpr std::string_view get_userinfo() const noexcept { return get(userinfo); }
	constexpr std::string_view get_user() const noexcept { return get(user); }
	constexpr std::string_view get_password() const noexcept { return get(password); }
	constexpr std::string_view get_host() const noexcept { return get(host); }
	constexpr std::string_view get_port() const noexcept { return get(port); }
	constexpr std::string_view get_path() const noexcept { return get(path); }
	constexpr std::string_view get_query() const noexcept { return get(query); }
	constexpr std::string_view get_fragment() const noexcept { return get(fragment); }

	constexpr int parse() noexcept
	{
		using namespace std::literals::string_view_literals;
		while(true)
		{
			if (_source.empty())
				set_error(error::empty_src);
			else if (_source.size() > uri_max_len)
				set_error(error::too_long);
			else if (_source.find_first_of(" \t\n\f\r\v"sv) != std::string_view::npos)
			{
				auto qur { _source.find_first_of('?') }, sps { _source.find_first_of(' ') };
				if (qur != std::string_view::npos && sps != std::string_view::npos && qur < sps)
					break;
				set_error(error::illegal_chars);
			}
			else
				break;
			return 0;	// refuse to parse
		}
		std::string_view::size_type pos{}, hst{}, pth{std::string_view::npos};
		bool scq{};
		if (const auto sch {_source.find_first_of(':')}; sch != std::string_view::npos)
		{
			_ranges[scheme] = {0, sch};
			set(scheme);
			pos = sch + 1;
		}
		if (_source[pos] == '?')	// short circuit query eg. magnet
			scq = true;
		else if (auto auth {_source.find("//"sv, pos)}; auth != std::string_view::npos)
		{
			auth += 2;
			if ((pth = _source.find_first_of('/', auth)) == std::string_view::npos) // unterminated path
				pth = _source.size();
			_ranges[authority] = {auth, pth - auth};
			set(authority);
			if (const auto usr {_source.find_first_of('@', auth)}; usr != std::string_view::npos && usr < pth)
			{
				if (const auto pw {_source.find_first_of(':', auth)}; pw != std::string_view::npos && pw < usr) // no nested ':' before '@'
				{
					_ranges[user] = {auth, pw - auth};
					if (usr - pw - 1 > 0)
					{
						_ranges[password] = {pw + 1, usr - pw - 1};
						set(password);
					}
				}
				else
					_ranges[user] = {auth, usr - auth};
				set(user);
				_ranges[userinfo] = {auth, usr - auth};
				set(userinfo);
				hst = pos = usr + 1;
			}
			else
				hst = pos = auth;

			if (auto prt { _source.find_first_of(':', pos) }; prt != std::string_view::npos)
			{
				if (auto autstr {get(authority)}; autstr.front() != '[' && autstr.back() != ']')
				{
					++prt;
					if (_source.size() - prt > 0)
					{
						_ranges[port] = {prt, _source.size() - prt};
						set(port);
					}
				}
			}
		}
		if (pth != std::string_view::npos)
		{
			if (test(port))
			{
				if (pth - _ranges[port].first == 0) // remove empty port
					clear(port);
				else
					_ranges[port].second = pth - _ranges[port].first;
				_ranges[host] = {hst, _ranges[port].first - 1 - hst};
			}
			else
				_ranges[host] = {hst, pth - hst};
			if (_ranges[host].second)
				set(host);
			_ranges[path] = {pth, _source.size() - pth};
			set(path);
		}
		if (pth == std::string_view::npos && !scq)
		{
			set(path);
			if ((pth = _source.find_first_of('/', pos)) != std::string_view::npos)
				_ranges[path] = {pth, _source.size() - pth};
			else if (test(scheme))
				_ranges[path] = {pos, _source.size() - pos};
			else
				clear(path);
		}
		if (const auto qur {_source.find_first_of('?', pos)}; qur != std::string_view::npos)
		{
			if (test(path))
				_ranges[path].second = qur - _ranges[path].first;
			_ranges[query] = {qur + 1, _source.size() - qur};
			set(query);
		}
		if (const auto fra {_source.find_first_of('#', pos)}; fra != std::string_view::npos)
		{
			if (test(query))
				_ranges[query].second = fra - _ranges[query].first;
			_ranges[fragment] = {fra + 1, _source.size() - fra};
			set(fragment);
		}
		return count();
	}

	template<char valuepair='&',char valueequ='='>
	constexpr query_result decode_query(bool sort=false) const noexcept
	{
		query_result result;
		if (test(query))
		{
			constexpr auto decpair([](std::string_view src) noexcept ->value_pair
			{
				if (auto fnd { src.find_first_of(valueequ) }; fnd != std::string_view::npos)
					return {src.substr(0, fnd), src.substr(fnd + 1)};
				else if (src.size())
					return {src, ""};
				return {};
			});
			std::string_view src{get(query)};
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
		}
		if (sort)
			sort_query(result);
		return result;
	}
	constexpr segments decode_segments(bool filter=true) const noexcept
	{
		segments result;
		if (test(path))
		{
			constexpr auto decuntil([](std::string_view src) noexcept ->std::string_view
			{
				if (auto fnd { src.find_first_of('/') }; fnd != std::string_view::npos)
					return src.substr(0, fnd);
				else if (src.size())
					return src;
				return {};
			});
			std::string_view src{get(path)};
			using namespace std::literals::string_view_literals;
			for (std::string_view::size_type pos{};;)
			{
				if (filter && src.substr(pos, 2) == "./"sv)
					++pos;
				if (auto fnd { src.find_first_of('/', pos) }; fnd != std::string_view::npos)
				{
					if (auto r1 { decuntil(src.substr(pos, fnd - pos)) }; r1.size() || pos)
						result.emplace_back(r1);
					pos = fnd + 1;
					continue;
				}
				if (pos <= src.size())
					result.emplace_back(decuntil(src.substr(pos, src.size() - pos)));
				break;
			}
		}
		return result;
	}

	static constexpr void sort_query(query_result& query) noexcept
		{ std::sort(query.begin(), query.end(), query_comp); }
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
		for (std::string_view::size_type fnd{pos}; ((fnd = src.find_first_of('%', fnd))) != std::string_view::npos; ++fnd)
		{
			if (fnd + 2 >= src.size())
				break;
			if (std::isxdigit(static_cast<unsigned char>(src[fnd + 1]))
			 && std::isxdigit(static_cast<unsigned char>(src[fnd + 2])))
				return fnd;
		}
		return std::string_view::npos;
	}
	static constexpr bool has_hex(std::string_view src) noexcept
		{ return find_hex(src) != std::string_view::npos; }
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
	static constexpr std::string normalize(std::string_view src) noexcept
	{
		using namespace std::literals::string_view_literals;
		std::string result{src};
		basic_uri bu{result};
		const auto isup([](std::string_view src) noexcept ->bool
			{ return std::any_of(std::cbegin(src), std::cend(src), [](const auto c) noexcept { return std::isupper(c); }); });
		const auto tolo([](auto c) noexcept ->auto { return std::tolower(c); });
		auto sch { bu.get_scheme() }, hst { bu.get_host() };
		if (isup(sch)) // 1. scheme => lower case
			transform(sch.begin(), sch.end(), std::string::iterator(result.data() + bu[scheme].first), tolo);
		if (isup(hst)) // 2. host => lower case
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
			for (const auto& pp : segs)
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
		else if (!bu.has_any_authority()) // 7. Convert empty path to "/"
			result += '/';
		return result;
	}
	static constexpr std::string normalize_http(std::string_view src) noexcept
	{
		auto result{normalize(src)};
		basic_uri bu{result};
		if (!bu.get_port().empty() && (bu.get_port() == _default_ports[static_cast<int>(scheme_t::http)].second
			|| bu.get_port() == _default_ports[static_cast<int>(scheme_t::https)].second))
		{
			auto prec{basic_uri::find_port(bu.get_scheme())};
			result.erase(bu[port].first - 1, bu[port].second + 1); // remove ":80"
			bu.assign(result);
			return std::string(bu.get_uri());
		}
		return result;
	}

	static constexpr std::string make_edit(const auto& what, std::initializer_list<comp_pair> from)
	{
		basic_uri ibase;
		comp_list ilist{countof};
		for (component ii{}; ii != countof; ii = component(ii + 1))
		{
			if (what.test(ii))
			{
				ibase.set(ii);
				ilist[ii] = what.get_component(ii);
			}
		}
		for (const auto& [comp,str] : from)
		{
			if (comp < countof)
			{
				ibase.set(comp);
				ilist[comp] = str;
			}
		}
		if (!ibase.test())
			return 0;
		if (ibase.has_any_authority())
			ibase.clear(authority);
		if (ibase.test(userinfo) && ibase.has_any_userinfo())
			ibase.clear(userinfo);
		return make_uri(ibase, std::move(ilist));
	}
	static constexpr std::string encode_hex(std::string_view src) noexcept
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

	static constexpr std::string make_uri(std::initializer_list<comp_pair> from) noexcept
	{
		basic_uri ibase;
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

	friend std::ostream& operator<<(std::ostream& os, const basic_uri& what)
	{
		if (!what)
			os << "error: " << static_cast<int>(what.get_error()) << '\n';
		os << std::setw(12) << std::left << "uri" << what._source << '\n';
		for (component ii{}; ii != countof; ii = component(ii + 1))
		{
			if (what.test(ii))
			{
				os << std::setw(12) << std::left << what.get_name(ii)
					<< (!what.get(ii).empty() ? what.get(ii) : "(empty)") << '\n';
				if (ii == path)
					if (const auto presult { what.decode_segments() }; presult.size() > 1)
						for (const auto& tag : presult)
							os << "   " << (!tag.empty() ? tag : "(empty)") << '\n';
				if (ii == query)
					if (const auto qresult { what.decode_query() }; qresult.size() > 1)
						for (const auto& [tag,value] : qresult)
							os << "   " << std::setw(12) << std::left << tag << (!value.empty() ? value : "(empty)") << '\n';
			}
		}
		return os;
	}

	friend constexpr auto operator==(const basic_uri& lhs, const basic_uri& rhs) noexcept
		{ return lhs._source == rhs._source; }

private:
	static constexpr bool is_reserved(char c) noexcept
		{ return _reserved.find_first_of(c) != std::string_view::npos; }
	static constexpr bool is_unreserved(char c) noexcept
		{ return std::isalnum(c) || c == '-' || c == '.' || c == '_' || c == '~'; }
	static constexpr bool is_unreserved_ascii(std::string_view src) noexcept // is %XX unreserved?
		{ return is_unreserved(cvt_hex_octet(src[1]) << 4 | cvt_hex_octet(src[2])); }
	static constexpr bool query_comp(const value_pair& pl, const value_pair& pr) noexcept { return pl.first < pr.first; }
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
	static constexpr std::string make_uri(basic_uri ibase, comp_list ilist) noexcept
	{
		if (!ibase.test())
			return {};
		using namespace std::literals::string_view_literals;
		basic_uri done;
		std::string result;
		for (component ii{}; ii != countof; ii = component(ii + 1))
		{
			if (!ibase.test(ii) || done.test(ii))
				continue;
			switch(const auto& str{ilist[ii]}; ii)
			{
			case scheme:
				result += str;
				result += ':';
				if (ibase.has_any_authority())
					result += "//"sv;
				break;
			case authority:
				if (!ibase.has_any_authority())
					result += "//"sv;
				result += str;
				break;
			case userinfo:
				if (ibase.test(authority) || ibase.has_any_userinfo())
					continue;
				result += str;
				break;
			case user:
				if (str.empty() && (ibase.test(authority) || ibase.test(userinfo)))
					continue;
				result += str;
				break;
			case password:
				if (ibase.test(authority) || ibase.test(userinfo))
					continue;
				if (!str.empty())
				{
					result += ':';
					result += str;
				}
				break;
			case host:
				if (ibase.test(authority))
					continue;
				if ((!ilist[user].empty() || !ilist[password].empty()) && (done.test(user) || done.test(password)))
					result += '@';
				result += str;
				break;
			case port:
				if (ibase.test(authority))
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
/// static storage
template<size_t sz>
class uri_storage
{
	std::array<char, sz> _buffer;
	size_t _sz{};
protected:
	constexpr uri_storage(std::string src) noexcept : _sz(src.size() > sz ? 0 : src.size())
		{ std::copy_n(src.cbegin(), _sz, _buffer.begin()); }
	constexpr uri_storage() = default;
	~uri_storage() = default;
	constexpr std::string swap(std::string src) noexcept
	{
		if (src.size() > sz)
			return {};
		std::string old(_buffer.cbegin(), _sz);
		std::copy_n(src.cbegin(), _sz = src.size(), _buffer.begin());
		return old;
	}
public:
	constexpr std::string_view buffer() const noexcept { return {_buffer.cbegin(), _sz}; }
	static constexpr auto max_storage() noexcept { return sz; }
};

//-----------------------------------------------------------------------------------------
/// specialisation: dynamic storage
template<>
class uri_storage<0>
{
	std::string _buffer;
protected:
	constexpr uri_storage(std::string src) noexcept : _buffer(std::move(src)) {}
	constexpr uri_storage() = default;
	~uri_storage() = default;
	constexpr std::string swap(std::string src) noexcept { return std::exchange(_buffer, std::move(src)); }
public:
	constexpr std::string_view buffer() const noexcept { return _buffer; }
	static constexpr auto max_storage() noexcept { return basic_uri::uri_max_len; }
};

//-----------------------------------------------------------------------------------------
class uri : public uri_storage<0>, public basic_uri
{
public:
	constexpr uri(std::string src) noexcept
		: uri_storage(uri::has_hex(src) ? uri::decode_hex(src, true) : std::move(src)), basic_uri(buffer()) {}
	constexpr uri(std::string_view src) noexcept : uri(std::string(src)) {}
	constexpr uri(const char *src) noexcept : uri(std::string(src)) {}
	constexpr uri() = default;
	~uri() = default;

	constexpr std::string replace(std::string src)
	{
		auto rbuf { swap(std::move(src)) };
		assign(buffer());
		return rbuf;
	}
	constexpr int edit(std::initializer_list<comp_pair> from)
	{
		replace(make_edit(*this, std::move(from)));
		return count();
	}
	static constexpr auto factory(std::initializer_list<comp_pair> from) noexcept
		{ return uri(make_uri(std::move(from))); }
	static constexpr auto normalize_uri(std::string_view src) noexcept
		{ return uri(normalize(src)); }
	static constexpr auto normalize_http_uri(std::string_view src) noexcept
		{ return uri(normalize_http(src)); }

	friend constexpr auto operator==(const uri& lhs, const uri& rhs) noexcept
		{ return normalize(lhs.get_uri()) == normalize(rhs.get_uri()); }
};

//-----------------------------------------------------------------------------------------
template<size_t sz=1024>
class uri_static : public uri_storage<sz>, public basic_uri
{
public:
	constexpr uri_static(std::string src) noexcept
		: uri_storage<sz>(uri::has_hex(src) ? uri::decode_hex(src, true) : std::move(src)), basic_uri(this->buffer()) {}
	constexpr uri_static(std::string_view src) noexcept : uri_static(std::string(src)) {}
	constexpr uri_static(const char *src) noexcept : uri_static(std::string(src)) {}
	constexpr uri_static() = default;
	~uri_static() = default;

	constexpr std::string replace(std::string src)
	{
		auto rbuf { this->swap(std::move(src)) };
		assign(this->buffer());
		return rbuf;
	}
	constexpr int edit(std::initializer_list<comp_pair> from)
	{
		replace(make_edit(*this, std::move(from)));
		return count();
	}
	static constexpr auto factory(std::initializer_list<comp_pair> from) noexcept
		{ return uri_static(make_uri(std::move(from))); }
	static constexpr auto normalize_uri(std::string_view src) noexcept
		{ return uri_static(normalize(src)); }
	static constexpr auto normalize_http_uri(std::string_view src) noexcept
		{ return uri_static(normalize_http(src)); }

	friend constexpr auto operator==(const uri_static& lhs, const uri_static& rhs) noexcept
		{ return normalize(lhs.get_uri()) == normalize(rhs.get_uri()); }
};

} // FIX8
#endif // FIX8_URI_HPP_
