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
//
// Note: don't change the order of these records
//-----------------------------------------------------------------------------------------
using enum uri::component;
const std::vector<std::pair<const char *, std::vector<std::pair<uri::component, const char *>>>> tests
{
	{ "https://www.blah.com/",
		{ // 1
			{ scheme, "https" },
			{ authority, "www.blah.com" },
			{ host, "www.blah.com" },
			{ path, "/" },
		}
	},
	{ "https://www.blah.com",
		{ // 2
			{ scheme, "https" },
			{ authority, "www.blah.com" },
			{ host, "www.blah.com" },
			{ path, "" }, // empty path
		}
	},
	{ "https://www.blah.com:3000/test",
		{ // 3
			{ scheme, "https" },
			{ authority, "www.blah.com:3000" },
			{ host, "www.blah.com" },
			{ port, "3000" },
			{ path, "/test" },
		}
	},
	{ "https://dakka@www.blah.com:3000/",
		{ // 4
			{ scheme, "https" },
			{ authority, "dakka@www.blah.com:3000" },
			{ userinfo, "dakka" },
			{ user, "dakka" },
			{ host, "www.blah.com" },
			{ port, "3000" },
			{ path, "/" },
		}
	},
	{ "https://example.com/over/there?name=ferret&time=any#afrag",
		{ // 5
			{ scheme, "https" },
			{ authority, "example.com" },
			{ host, "example.com" },
			{ path, "/over/there" },
			{ query, "name=ferret&time=any" },
			{ fragment, "afrag" },
		}
	},
	{ "https://example.org/./a/../b/./c",
		{ // 6
			{ scheme, "https" },
			{ authority, "example.org" },
			{ host, "example.org" },
			{ path, "/./a/../b/./c" },
		}
	},
	{ "ws://localhost:9229/f46db715-70df-43ad-a359-7f9949f39868",
		{ // 7
			{ scheme, "ws" },
			{ authority, "localhost:9229" },
			{ host, "localhost" },
			{ port, "9229" },
			{ path, "/f46db715-70df-43ad-a359-7f9949f39868" },
		}
	},
	{ "ldap://[2001:db8::7]/c=GB?objectClass?one",
		{ // 8
			{ scheme, "ldap" },
			{ authority, "[2001:db8::7]" },
			{ host, "[2001:db8::7]" },
			{ path, "/c=GB" },
			{ query, "objectClass?one" },
		}
	},
	{ "file:///foo/bar/test/node.js",
		{ // 9
			{ scheme, "file" },
			{ authority, "" }, // empty authority
			{ path, "/foo/bar/test/node.js" },
		}
	},
	{	"http://nodejs.org:89/docs/latest/api/foo/bar/qua/13949281/0f28b/5d49/b3020/url.html"
		"?payload1=true&payload2=false&test=1&benchmark=3&foo=38.38.011.293"
		"&bar=1234834910480&test=19299&3992&key=f5c65e1e98fe07e648249ad41e1cfdb0#test",
		{ // 10
			{ scheme, "http" },
			{ authority, "nodejs.org:89" },
			{ host, "nodejs.org" },
			{ port, "89" },
			{ path, "/docs/latest/api/foo/bar/qua/13949281/0f28b/5d49/b3020/url.html" },
			{ query, "payload1=true&payload2=false&test=1&benchmark=3&foo=38.38.011.293"
						"&bar=1234834910480&test=19299&3992&key=f5c65e1e98fe07e648249ad41e1cfdb0" },
			{ fragment, "test" },
		}
	},
	{ "https://user:password@example.com/path?search=1",
		{ // 11
			{ scheme, "https" },
			{ authority, "user:password@example.com" },
			{ host, "example.com" },
			{ userinfo, "user:password" },
			{ user, "user" },
			{ password, "password" },
			{ path, "/path" },
			{ query, "search=1" },
		}
	},
	{ "javascript:alert(\"nodeisawesome\");",
		{ // 12
			{ scheme, "javascript" },
			{ path, "alert(\"nodeisawesome\");" },
		}
	},
	{ "https://%E4%BD%A0/foo",
		{ // 13
			{ scheme, "https" },
			{ authority, "你" },
			{ host, "你" },
			{ path, "/foo" },
		}
	},
	{ "http://你好你好.在",
		{ // 14
			{ scheme, "http" },
			{ authority, "你好你好.在" },
			{ host, "你好你好.在" },
			{ path, "" }, // empty path
		}
	},
	{ "urn:oasis:names:specification:docbook:dtd:xml",
		{ // 15
			{ scheme, "urn" },
			{ path, "oasis:names:specification:docbook:dtd:xml" },
		}
	},
	{ "mailto:John.Smith@example.com",
		{ // 16
			{ scheme, "mailto" },
			{ path, "John.Smith@example.com" },
		}
	},
	{ "news:comp.infosystems.www.servers.unix",
		{ // 17
			{ scheme, "news" },
			{ path, "comp.infosystems.www.servers.unix" },
		}
	},
	{ "tel:+1-816-555-1212",
		{ // 18
			{ scheme, "tel" },
			{ path, "+1-816-555-1212" },
		}
	},
	{ "telnet://user:password@192.0.2.16:8888/",
		{ // 19
			{ scheme, "telnet" },
			{ authority, "user:password@192.0.2.16:8888" },
			{ userinfo, "user:password" },
			{ user, "user" },
			{ password, "password" },
			{ host, "192.0.2.16" },
			{ port, "8888" },
			{ path, "/" },
		}
	},
	{ "http://-.~_!$&'()*+,;=:%40:80%2f::::::@example.com",
		{ // 20
			{ scheme, "http" },
			{ authority, "-.~_!$&'()*+,;=:@:80" },
			{ userinfo, "-.~_!$&'()*+,;=:" },
			{ user, "-.~_!$&'()*+,;=" },
			{ port, "80" },
			{ path, "/::::::@example.com" },
		}
	},
	{ "http://foo.com/blah_blah_(wikipedia)_(again)",
		{ // 21
			{ scheme, "http" },
			{ authority, "foo.com" },
			{ host, "foo.com" },
			{ path, "/blah_blah_(wikipedia)_(again)" },
		}
	},
	{ "http://उदाहरण.परीक्षा",
		{ // 22
			{ scheme, "http" },
			{ authority, "उदाहरण.परीक्षा" },
			{ host, "उदाहरण.परीक्षा" },
			{ path, "" }, // empty path
		}
	},
	{ "http://foo.com/(something)?after=parens",
		{ // 23
			{ scheme, "http" },
			{ authority, "foo.com" },
			{ host, "foo.com" },
			{ path, "/(something)" },
			{ query, "after=parens" },
		}
	},
	{ "http://foo.com/unicode_(✪)_in_parens",
		{ // 24
			{ scheme, "http" },
			{ authority, "foo.com" },
			{ host, "foo.com" },
			{ path, "/unicode_(✪)_in_parens" },
		}
	},
	{ "http://➡.ws/䨹",
		{ // 25
			{ scheme, "http" },
			{ authority, "➡.ws" },
			{ host, "➡.ws" },
			{ path, "/䨹" },
		}
	},
	{ "epgm://127.0.0.1;224.0.0.0:11042",
		{ // 26
			{ scheme, "epgm" },
			{ authority, "127.0.0.1;224.0.0.0:11042" },
			{ host, "127.0.0.1;224.0.0.0" },
			{ port, "11042" },
			{ path, "" }, // empty path
		}
	},
	{ "https://!$%25:)(*&^@www.netmeister.org/blog/urls.html",
		{ // 27
			{ scheme, "https" },
			{ authority, "!$%:)(*&^@www.netmeister.org" },
			{ host, "www.netmeister.org" },
			{ userinfo, "!$%:)(*&^" },
			{ user, "!$%" },
			{ password, ")(*&^" },
			{ path, "/blog/urls.html" },
		}
	},
	{ "https://www.netmeister.org/t/h/e/s/e/../../../../../d/i/r/e/c/t/o/"
		"r/i/e/s/../../../../../../../../../../../d/o/../../n/o/t/../../../e/x/i/s/t/../../../../../blog/urls.html",
		{ // 28
			{ scheme, "https" },
			{ authority, "www.netmeister.org" },
			{ host, "www.netmeister.org" },
			{ path, "/t/h/e/s/e/../../../../../d/i/r/e/c/t/o/r/i/e/s/../../../../../../../../../../../d/o/../../n/o/t/"
				"../../../e/x/i/s/t/../../../../../blog/urls.html" },
		}
	},
	{ "https://www.blah.com:/test",
		{ // 29
			{ scheme, "https" },
			{ authority, "www.blah.com:" },
			{ host, "www.blah.com" },
			{ path, "/test" },
		}
	},
	{ "https://www.netmeister.org/%62%6C%6F%67/%75%72%6C%73.%68%74%6D%6C?!@#$%25=+_)(*&^#top%3C",
		{ // 30
			{ scheme, "https" },
			{ authority, "www.netmeister.org" },
			{ host, "www.netmeister.org" },
			{ path, "/blog/urls.html" },
			{ query, "!@" },
			{ fragment, "$%=+_)(*&^#top<" },
		}
	},
	{ "https://en.wikipedia.org/wiki/C%2B%2B20",
		{ // 31
			{ scheme, "https" },
			{ authority, "en.wikipedia.org" },
			{ host, "en.wikipedia.org" },
			{ path, "/wiki/C++20" },
		}
	},
	{ "https://www.netmeister.org/%62%63%70/%%4%",
		{ // 32
			{ scheme, "https" },
			{ authority, "www.netmeister.org" },
			{ host, "www.netmeister.org" },
			{ path, "/bcp/%%4%" },
		}
	},
	{ "www.hello.com/",
		{ // 33
			{ path, "/" },
		}
	},
	{ "www.hello.com",
		{ // 34
		}
	},
	{ "http://host.com/?third=3rd&first=1st&second=2nd",
		{ // 35
			{ scheme, "http" },
			{ authority, "host.com" },
			{ host, "host.com" },
			{ path, "/" },
			{ query, "third=3rd&first=1st&second=2nd" },
		}
	},
	{ "magnet:?xt=urn:btih:08ada5a7a6183aae1e09d831df6748d566095a10&dn=Sintel&tr=udp%3A%2F%2Fexplodie.org%3A6969&tr=udp"
		"%3A%2F%2Ftracker.coppersurfer.tk%3A6969&tr=udp%3A%2F%2Ftracker.empire-js.us%3A1337&tr=udp%3A%2F%2Ftracker.leechers-paradise.org"
		"%3A6969&tr=udp%3A%2F%2Ftracker.opentrackr.org%3A1337&tr=wss%3A%2F%2Ftracker.btorrent.xyz&tr=wss%3A%2F%2Ftracker.fastcast.nz&tr=wss"
		"%3A%2F%2Ftracker.openwebtorrent.com&ws=https%3A%2F%2Fwebtorrent.io%2Ftorrents%2F&xs=https%3A%2F%2Fwebtorrent.io%2Ftorrents%2Fsintel.torrent",
		{ // 36
			{ scheme, "magnet" },
			{ query,	"xt=urn:btih:08ada5a7a6183aae1e09d831df6748d566095a10&dn=Sintel&tr=udp://explodie.org:6969"
						"&tr=udp://tracker.coppersurfer.tk:6969&tr=udp://tracker.empire-js.us:1337&tr=udp://tracker.leechers-paradise.org:6969"
						"&tr=udp://tracker.opentrackr.org:1337&tr=wss://tracker.btorrent.xyz&tr=wss://tracker.fastcast.nz&tr=wss://tracker.openwebtorrent.com"
						"&ws=https://webtorrent.io/torrents/&xs=https://webtorrent.io/torrents/sintel.torrent" },
		}
	},
};

