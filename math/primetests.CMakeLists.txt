# THIS FILE IS FOR TESTING PRIME-FIELD MATRICES

cmake_minimum_required (VERSION 3.1)
project (prime_field_test)
set (CMAKE_CXX_STANDARD 14)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-ftemplate-depth=1024 -Wall -Wextra -pedantic")

#include_directories("..")

add_executable(prime_field_test
	map_matrix.cpp
	index_matrix.cpp
	bool_array.cpp
	../debug.cpp)

