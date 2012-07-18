cmake_minimum_required (VERSION 2.6)
project (spotifyd)

# version
set (spotifyd_VERSION_MAJOR 0)
set (spotifyd_VERSION_MINOR 1)
set (spotifyd_VERSION_PATCH 0)


#should we bundle the libs??? 
#spotify yes, thrift no. Make use of FindThrift.cmake 
# Sample FindThrift: http://cloudscribe.googlecode.com/svn-history/r2/trunk/cmake/FindThrift.cmake
#
# Actually, as a matter of fact, the entire cloudscribe cmake environment is a great example/mirror
# for what we're trying to achieve with out build env + thrift. It even includes the thrift
# code generation snippets, such that when you make with cmake those are made too from the thrift
# definition files. Plenty to be done, but looking good.

# Locate required packages
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

find_package(LibEvent REQUIRED)
find_package(Boost REQUIRED)
find_package(Thrift REQUIRED)
find_package(Spotify REQUIRED)
link_directories(${PROJECT_SOURCE_DIR}/lib)

include_directories(${PROJECT_BINARY_DIR}
	${PROJECT_SOURCE_DIR}/include
	${Thrift_INCLUDE_DIR} ${Thrift_INCLUDE_DIR}/thrift)

#compiler stuff
set(CMAKE_C_FLAGS "-ggdb -Wall -W -fbuiltin")
set(CMAKE_CXX_FLAGS "-ggdb -Wall -W -fbuiltin -pthread")
# compile as C++0x
add_definitions(-std=c++0x -DHAVE_INTTYPES_H -DHAVE_NETINET_IN_H)
add_subdirectory(src)
