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

# to disable building unit tests:
# cmake -DBUILD_UNITTESTS=false ..
option(BUILD_UNITTESTS "enable building unit tests" true)
message("-- Build: unit tests ${BUILD_UNITTESTS}")

# to disable building benchmarking:
# cmake -DBUILD_BENCHMARKS=false ..
option(BUILD_BENCHMARKS "enable building benchmarking" true)
message("-- Build benchmarking: ${BUILD_BENCHMARKS}")

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

foreach(x IN LISTS files)
	cmake_path(GET x STEM LAST_ONLY target)
	add_executable(${target} examples/${x})
	set_target_properties(${target} PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED true)
	target_include_directories(${target} PRIVATE include examples)
	if(BUILD_BENCHMARKS AND ${target} STREQUAL benchmarks)
		target_link_libraries(${target} PRIVATE Criterion)
	endif()
	if(BUILD_UNITTESTS AND ${target} STREQUAL unittests)
		target_link_libraries(${target} PRIVATE Catch2::Catch2WithMain)
		include(Catch)
		enable_testing()
		catch_discover_tests(${target})
	endif()
	get_target_property(cppstd ${target} CXX_STANDARD)
	message("-- adding source ${x} (C++${cppstd})")
endforeach()

# make include visible to inheriting projects
add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

