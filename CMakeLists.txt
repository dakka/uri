# ----------------------------------------------------------------------------------------
# SPDX-License-Identifier: MIT
# SPDX-FileCopyrightText: Copyright (C) 2024 Fix8 Market Technologies Pty Ltd
# SPDX-FileType: SOURCE
#
# uri (header only)
# Copyright (C) 2024 Fix8 Market Technologies Pty Ltd
#   by David L. Dight
# see https://github.com/fix8mt/uri
#
# Licensed under the MIT License <http://opensource.org/licenses/MIT>.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is furnished
# to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice (including the next paragraph)
# shall be included in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
# INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
# PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
# HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
# OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
# SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
# ----------------------------------------------------------------------------------------
# cmake config
#	clang
#		CXX="clang++" CC="clang" cmake ..
#	gcc
#		CXX="g++" CC="gcc" cmake ..
#
# min cmake version 3.20 (Mar 24, 2021)
# ----------------------------------------------------------------------------------------
cmake_minimum_required (VERSION 3.20)
project (uri
	LANGUAGES CXX
	HOMEPAGE_URL https://github.com/fix8mt/${PROJECT_NAME}
	DESCRIPTION "Lightweight header-only C++20 URI parser"
	VERSION 1.4.0
)

# to disable an option
# cmake -DOPTION=false ..
set(CUSTOM_OPTIONS
	"BUILD_UNITTESTS:enable building unit tests:true"
	"BUILD_STATICTEST:enable building statictest:true"
	"BUILD_BENCHMARKS:enable building benchmarking:true"
)
foreach(x ${CUSTOM_OPTIONS})
	string(REPLACE ":" ";" part ${x})
	list(GET part 0 option_name)
	list(GET part 1 option_description)
	list(GET part 2 option_default)
	if(NOT DEFINED ${option_name})
		set(${option_name} ${option_default})
	endif()
	option(${option_name} "${option_description}" ${${option_name}})
	message("-- Build: ${option_description}: ${${option_name}}")
endforeach()

set(files uritest.cpp)

if(BUILD_BENCHMARKS)
	include(FetchContent)
	list(APPEND files benchmarks.cpp)
	message(STATUS "Downloading Criterion...")
	FetchContent_Declare(Criterion GIT_REPOSITORY https://github.com/p-ranav/criterion.git GIT_SHALLOW	ON)
	FetchContent_MakeAvailable(Criterion)
	if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
		target_compile_options(Criterion INTERFACE -fbracket-depth=257)
	endif()
endif()

if(BUILD_UNITTESTS)
	include(FetchContent)
	list(APPEND files unittests.cpp)
	list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
	message(STATUS "Downloading Catch2...")
	FetchContent_Declare(Catch2 GIT_REPOSITORY https://github.com/catchorg/Catch2.git GIT_SHALLOW ON GIT_TAG devel)
	FetchContent_MakeAvailable(Catch2)
endif()

if(BUILD_STATICTEST)
	list(APPEND files statictest.cpp)
endif()

foreach(x IN LISTS files)
	cmake_path(GET x STEM LAST_ONLY target)
	add_executable(${target} examples/${x})
	set_target_properties(${target} PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED true)
	target_include_directories(${target} PRIVATE include examples)
	if(${target} STREQUAL benchmarks)
		target_link_libraries(${target} PRIVATE Criterion)
	endif()
	if(${target} STREQUAL unittests)
		target_link_libraries(${target} PRIVATE Catch2::Catch2WithMain)
		include(Catch)
		enable_testing()
		catch_discover_tests(${target})
	endif()
	if(${target} STREQUAL statictest)
		if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
			target_compile_options(${target} PRIVATE -fconstexpr-ops-limit=50000000)
		elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
			target_compile_options(${target} PRIVATE -fconstexpr-depth=50000000 -fconstexpr-steps=50000000)
		endif()
	endif()
	get_target_property(cppstd ${target} CXX_STANDARD)
	message("-- adding source ${x} (C++${cppstd}, ${CMAKE_CXX_COMPILER_ID})")
endforeach()

# make include visible to inheriting projects
add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

