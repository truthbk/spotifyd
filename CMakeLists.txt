cmake_minimum_required (VERSION 2.6)
project (spotifyd)

# version
set (spotifyd_VERSION_MAJOR 0)
set (spotifyd_VERSION_MINOR 1)
set (spotifyd_VERSION_PATCH 0)

add_subdirectory(src thrift lib)

# compiler stuff
set(CMAKE_CXX_FLAGS "-ggdb -Wall -W")

# compile as C99
add_definitions(-std=c++0x)
