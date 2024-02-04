<p align="center">
  <a href="https://www.fix8mt.com"><img src="https://github.com/fix8mt/uri/blob/master/assets/fix8mt_Master_Logo_Green_Trans.png" width="200"></a>
</p>

# uri

### A lightweight C++20 URI parser

------------------------------------------------------------------------
## Introduction
This is a lightweight URI parser implementation featuring zero-copy, minimal storage and high performance.

## Quick links
|**Link**|**Description**|
--|--
|[Here](https://github.com/fix8mt/uri/blob/master/include/fix8/uri.hpp)| for implementation|
|[Examples](https://github.com/fix8mt/uri/blob/master/examples)| for examples and test cases|

## Motivation
- header-only
- zero-copy where possible (base class uses references only)
- no external dependencies
- simplicity, lightweight
- make use of C++20 features
- entirely `constexpr`
- high performance
- non-validating

## Features
- single _header-only_
- base class is zero-copy, using `std::string_view`
- derived class moves (or copies) source string once
- all methods `constexpr`; no virtual methods
- extracts all components `scheme`, `authority`, `userinfo`, `user`, `password`, `host`, `port`, `path`, `query`, `fragment`
- query components returned as `std::string_view`
- query decode and search - no copying, results point to uri source
- fast, very lightweight, predictive non brute force parser
- small memory footprint - base class object is only 64 bytes
- built-in unit test cases with exhaustive test URI cases
- support for [**RFC 3986**](https://datatracker.ietf.org/doc/html/rfc3986)

# Examples
## 1. Parse and hold a URI
This example parses a URI string and prints out all the contained elements. Then individual components are queried and printed if present.

<details><summary><i>source</i></summary>
<p>

```c++
#include <iostream>
#include <fix8/uri.hpp>
using namespace FIX8;

int main(int argc, char *argv[])
{
   const uri u1 {"http://nodejs.org:89/docs/latest/api/foo/bar/qua/13949281/0f28b/5d49/b3020/url.html"
      "?payload1=true&payload2=false&test=1&benchmark=3&foo=38.38.011.293"
      "&bar=1234834910480&test=19299&3992&key=f5c65e1e98fe07e648249ad41e1cfdb0#test"};

   std::cout << u1 << '\n';

   std::cout << u1.get_component(uri::authority) << '\n'
      << u1.get_component(uri::host) << '\n'
      << u1.get_component(uri::port) << '\n'
      << u1.get_component(uri::query) << '\n'
      << u1.get_component(uri::fragment) << '\n';
   if (u1.test(uri::user)) // should be no user
      std::cout << u1.get_component(uri::user) << '\n';
   auto result{u1.decode_query(true)}; // sort result
   std::cout << "key = " << uri::find_query("key", result) << '\n';
   return 0;
}
```

</p>
</details>

<details><summary><i>output</i></summary>
</p>

```CSV
$ ./example1
source      http://nodejs.org:89/docs/latest/api/foo/bar/qua/13949281/0f28b/5d49/b3020/url.html?payload1=true&payload2=false&test=1&benchmark=3&foo=38.38.011.293&bar=1234834910480&test=19299&3992&key=f5c65e1e98fe07e648249ad41e1cfdb0#test
scheme      http
authority   nodejs.org:89
host        nodejs.org
port        89
path        /docs/latest/api/foo/bar/qua/13949281/0f28b/5d49/b3020/url.html
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

nodejs.org:89
nodejs.org
89
payload1=true&payload2=false&test=1&benchmark=3&foo=38.38.011.293&bar=1234834910480&test=19299&3992&key=f5c65e1e98fe07e648249ad41e1cfdb0
test
key = f5c65e1e98fe07e648249ad41e1cfdb0
$
```

</p>
</details>

## 2. Parse a URI
This example parses a URI string and prints out all the contained elements. Note the use of `basic_uri`. The lifetime of the `u1` object
is undefined after the `if` statement is executed. The source string is not stored.

<details><summary><i>source</i></summary>
<p>

```c++
#include <iostream>
#include <fix8/uri.hpp>
using namespace FIX8;

int main(int argc, char *argv[])
{
   if (const basic_uri u1{ "ws://localhost:9229/f46db715-70df-43ad-a359-7f9949f39868" }; u1)
      std::cout << u1 << '\n';
   return 0;
}
```

</p>
</details>

<details><summary><i>output</i></summary>
</p>

```CSV
$ ./example2
source      ws://localhost:9229/f46db715-70df-43ad-a359-7f9949f39868
scheme      ws
authority   localhost:9229
host        localhost
port        9229
path        /f46db715-70df-43ad-a359-7f9949f39868
$
```

</p>
</details>

# Building
This implementation is header only. Apart from standard C++20 includes there are no external dependencies needed in your application.
[Catch2](https://github.com/catchorg/Catch2.git) is used for the built-in unit tests.

## 1. Obtaining the source, building the examples
To clone and default build all the examples, including the unit tests.
```bash
git clone git@github.com:fix8mt/uri.git
cd uri
mkdir build
cd build
cmake ..
make -j4
make test (or ctest)
```
## 2. Using in your application
In `CMakeLists.txt` set your include path to:
```cmake
include_directories([uri directory]/include)
# e.g.
set(uridir /home/dd/prog/uri)
include_directories(${uridir}/include)
```
and just include:
```c++
#include <fix8/uri.hpp>
```
in your application. Everything in this class is within the namespace `FIX8`, so you can add:
```c++
using namespace FIX8;
```

# API
## Class hierarchy
The base class `basic_uri` performs the bulk of the work, holding a `std::string_view` of the source uri string. If you wish to manage the scope of the source uri yourself then
this class is the most efficient way to do so.

The derived class `uri` stores the source string and then builds a `basic_uri` using that string as its reference. `uri` derives from `basic_uri` and a private storage class
`uri_storage`. The supplied string is moved or copied and stored by the object. If your application needs the uri to hold and persist the source uri, this class is suitable.

![class diagram](https://github.com/fix8mt/uri/blob/master/assets/classdiag.png)

## Types
### component
```c++
enum component { scheme, authority, userinfo, user, password, host, port, path, query, fragment, countof };
```
Components are named by a public enum called `component`.  Note that the component `user` and `password` are populated if present and
`userinfo` will also be populated.

### other types
| Type | Typedef of |Description |
| --- | --- | --- |
| `uri_len_t`  | `std::uint16_t` | the integral type used to store offsets and lengths|
| `value_pair`  | `std::pair<std::string_view,std::string_view>` |used to return tag value pairs|
| `query_result`  | `std::vector<value_pair>` |used to return a collection of query pairs|
| `range_pair`  | `std::pair<uri_len_t,uri_len_t>` |used to store offset and length |
| `comp_pair` | `std::pair<component, std::string_view>`|used by `factory` to pass individual `component` pairs|
| `value_list` | `std::vector<std::string_view>`|used by `factory`,`edit` and `make_source` to pass individual `component` values, each position in the vector corresponds to the component index|

### consts
| Const | Description |
| ------------- | ------------- |
| `uri_max_len`  | the maximum length of a supplied uri|

## Construction and destruction
### ctor
```c++
constexpr basic_uri(std::string_view src);                           (1)
constexpr basic_uri(uri_len_t bits);                                 (2)
constexpr basic_uri() = default;                                     (3)
constexpr uri(std::string src, bool decode=true);                    (4)
constexpr uri() = default;                                           (5)
```

1. Construct a `basic_uri` from a `std::string_view`. This base class does not store the string. Calls `parse()`. The source string must not go out of scope to use this object. Throws a `std::exception` if parsing fails.
1. Construct a `basic_uri` that has the corresponding bitset passed in `bits`. No components are present. It is used to permit the object to be used as a bitset.
1. Construct an empty `basic_uri`. It can be populated using `assign()`.
1. Construct a `uri` from a `std::string`. By default, the source string is percent decoded before parsing. Calls `parse()`. Optionally pass `false` to prevent percent decoding.
The supplied string is moved or copied and stored by the object. Throws a `std::exception` if parsing fails.
1. Construct an empty `uri`. It can be populated using `replace()`.

All of `uri` is within the namespace **`FIX8`**.

### dtor
```c++
~basic_uri();
~uri();
```

Destroy the `uri` or `basic_uri`. The `uri` object will release the stored string.

## Accessors
### `test`
```c++
constexpr bool uri::test(uri::component what=countof) const;
```
Return `true` if the specified component is present in the uri. With no parameter (default) returns `true` if any component is present.

### `get_component`
```c++
constexpr std::string_view get_component(component what) const;
```
Return a `std::string_view` of the specified component or empty if component not found. Throws a `std::out_of_range` if not a legal component.

### `get_present`
```c++
constexpr uri_len_t get_present() const;
```
Return the present bitset as `uri_len_t` which has bits set corresponding to the component's enum position.

### `get`
```c++
constexpr std::string_view get(component what) const;
```
Return a `std::string_view` of the specified component.
> [!WARNING]
> This is _not_ range checked.

### `const operator[component]`
```c++
constexpr const range_pair& operator[](component idx) const;
```
Return a `const range_pair&` which is a `std::pair<uri_len_t, uri_len_t>&` to the specified component at the index given in the ranges table. This provides read-only
access to the offset and length of the specified component and is used to create a `std::string_view`. No copying, results point to uri source.
> [!WARNING]
> This is _not_ range checked.

### `decode_query`
```c++
template<char separator='&',char tagequ='='>
constexpr query_result decode_query(bool sort=false) const;
```
Returns a `std::vector` of pairs of `std::string_view` of the query component if present.  You can optionally override the value pair separator character using
the first non-type template parameter - some queries use `;`. You can also optionally override the value equality separator character using the second non-type
template parameter (some queries use `:`). Pass `true` to optionally sort the `query_result` lexographically by the key. No copying, results point to uri source.
Returns an empty vector if no query was found. The query is assumed to be in the form:
```
&tag=value[&tag=value...]
```
Or if you override, say
```
;tag:value[;tag:value...]
```
If no value is present, just the tag will be populated with an empty value.

### `find_query`
```c++
static constexpr std::string_view find_query (std::string_view what, const query_result& from);
```
Find the specified query key and return its value from the given `query_result`. `query_result` must be sorted by key, as returned by
passing `true` to `decode_query`. If key not found return empty `std::string_view`. No copying, results point to uri source.

### `decode_hex`
```c++
static constexpr std::string decode_hex(std::string_view src);
```
Decode any hex values present in the supplied string. Hex values are only recognised if
they are in the form `%XX` where X is a hex digit (octet) `[0-9a-fA-F]`. Return in a new string.

### `has_hex`
```c++
static constexpr bool has_hex(std::string_view src);
```
Return true if any hex values are present in the supplied string. Hex values are only recognised if
they are in the form `%XX` where X is a hex digit (octet) `[0-9a-fA-F]`.

### `find_hex`
```c++
static constexpr std::string_view::size_type find_hex(std::string_view src);
```
Return the position of the first hex value (if any) in the supplied string. Hex values are only recognised if
they are in the form `%XX` where X is a hex digit (octet) `[0-9a-fA-F]`. If not found returns `std::string_view::npos`.

### `get_name`
```c++
static constexpr std::string_view get_name(component what);
```
Return a `std::string_view` of the specified component name. Throws a `std::out_of_range` if not a legal component.

### `get_source`
```c++
constexpr std::string_view get_source() const;
```
Return a `std::string_view` of the source uri. If not set return value will be empty.

### `get_named_pair`
```c++
constexpr value_pair get_named_pair(component what) const;
```
Return a `std::pair` of `std::string_view` for the specified component. The `first` will be the component name, `second` the component value. Suitable
for populating a JSON field. Throws a `std::out_of_range` if not a legal component. No copying, results point to uri source.

### `count`
```c++
constexpr int count() const;
```
Return the count of components in the uri.

### `operator<<`
```c++
friend std::ostream& operator<<(std::ostream& os, const basic_uri& what);
```
Print the uri object to the specified stream. The source and individual components are printed. If a query is present, each tag value pair is also printed.

### `get_buffer`
```c++
constexpr const std::string& get_buffer() const;
```
Return a `const std::string&` to the stored buffer. Only available from `uri`.

## Mutators
### `set`
```c++
constexpr void uri::set(uri::component what=countof);
```
Set the specified component bit as present in the uri. By default sets all bits. Use carefully.

### `clear`
```c++
constexpr void uri::clear(uri::component what=countof);
```
Clear the specified component bit in the uri. By default clears all bits. Use carefully.

### `assign`
```c++
constexpr int assign(std::string_view src);
```
Replace the current uri reference with the given reference. No storage is allocated. Return the number of components found.

### `replace`
```c++
constexpr std::string replace(std::string&& src);
```
Replace the current uri with the given string. The storage is updated with a move (or copy) of the string. The old string is returned.

## `operator[component]`
```c++
constexpr range_pair& operator[](component idx);
```
Return a `range_pair&` which is a `std::pair<uri_len_t, uri_len_t>&` to the specified component at the index given in the ranges table. This provides direct
access to the offset and length of the specified component and is used to create a `std::string_view`.
> [!WARNING]
> This is _not_ range checked. Allows for modification of the `string_view` range. Use carefully.

## `parse`
```c++
constexpr int parse();
```
Parse the source string into components. Return the count of components found. Will reset a uri if already parsed. Throws a `std::exception` if parsing fails.

## Generation and editing
### `factory`
```c++
static constexpr uri factory(std::initializer_list<comp_pair> from);
```
Create a `uri` from the supplied components. The `initializer_list` contains a 1..n `comp_pair` objects. The following constraints apply:

# Testing
## Test cases
The header file `uriexamples.hpp` contains a data structure holding test cases used by the Catch2 unit test app `uritest2` and by the CLI test app `uritest`.
You can add your own test cases to `uriexamples.hpp` - the structure is easy enough to follow.

<details><summary><i>sample</i></summary>
<p>

```c++
const std::vector<std::pair<const char *, std::vector<std::pair<uri::component, const char *>>>> tests
{
   { "https://www.blah.com/",
      {
         { scheme, "https" },
         { authority, "www.blah.com" },
         { host, "www.blah.com" },
         { path, "/" },
      }
   },
   { "https://www.blah.com",
      {
         { scheme, "https" },
         { authority, "www.blah.com" },
         { host, "www.blah.com" },
         { path, "" }, // empty path
      }
   },
   { "https://www.blah.com:3000/test",
      {
         { scheme, "https" },
         { authority, "www.blah.com:3000" },
         { host, "www.blah.com" },
         { port, "3000" },
         { path, "/test" },
      }
   },
   { "https://dakka@www.blah.com:3000/",
      {
         { scheme, "https" },
         { authority, "dakka@www.blah.com:3000" },
         { user, "dakka" },
         { host, "www.blah.com" },
         { port, "3000" },
         { path, "/" },
      }
   },
.
.
.
```

</p>
</details>

## `uritest2`
This application is run by default if you run `make test` or `ctest`. When running using `ctest` use the following command:

```bash
$ ctest --output-on-failure
```

## `uritest`
This is a simple CLI test app which allows you to run individual or all tests from `uriexamples.hpp`, or test a uri passed from the command line.

```bash
$ ./uritest -h
Usage: ./uritest [uri...] [-t:d:hlas]
 -a run all tests
 -d [uri] parse uri from CLI, show debug output
 -h help
 -l list tests
 -s show sizes
 -t [num] test to run
$
```

You can run adhoc tests from the CLI as follows:
<details><summary><i>output</i></summary>
</p>

```CSV
$ ./uritest -d "https://user:password@example.com:3000/path?search=1&key=val&when=now#frag"
source      https://user:password@example.com:3000/path?search=1&key=val&when=now#frag
scheme      https
authority   user:password@example.com:3000
user        user
password    password
host        example.com
port        3000
path        /path
query       search=1&key=val&when=now
   search      1
   key         val
   when        now
fragment    frag

111111111 (511)
scheme 0 (5)
authority 8 (30)
user 8 (4)
password 13 (8)
host 22 (11)
port 34 (4)
path 38 (5)
query 44 (25)
fragment 70 (5)
$
```

</p>
</details>

# Discussion
## Non-validating
This class is non-validating. The source URI is expected to be normalised or at least parsable. Validation is out of scope for this implementation.
We decided against validating for a few reasons:
1. Performance - validating is expensive; most URIs are generally parsable
1. Complex - validation rules are complicated; for most use cases, simple rejection for gross rule violation is sufficient.
See [URL Standard](https://url.spec.whatwg.org/) for complete validation rules.

## Low level access
There are two methods that provide unchecked direct access to the `range` table and `component`. You must ensure that you don't pass an invalid component
index when using these. Making changes to the range object with `operator[]` can have serious consequences. Use carefully!
1. `constexpr std::string_view get(component what) const;`
1. `constexpr range_pair& operator[](component idx)`;

## Sanity checking
This class will perform basic sanity checks on the source URI and throws `std::exception` on failure. These are:
1. Length - source must not exceed `uri_max_len` (`UINT16_MAX`)
1. Illegal chars - source must not contain any whitespace characters
