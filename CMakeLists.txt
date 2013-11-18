cmake_minimum_required (VERSION 2.6)
project (spotifyd)

# version
set (spotifyd_VERSION_MAJOR 0)
set (spotifyd_VERSION_MINOR 1)
set (spotifyd_VERSION_PATCH 0)

OPTION(DEBUG "Activate debug flags" OFF)

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
find_package(Boost COMPONENTS system filesystem program_options REQUIRED)
find_package(Thrift REQUIRED)
find_package(Spotify REQUIRED)
if(UNIX AND NOT APPLE)
	find_package(ALSA)
	find_package(OpenAL)
	find_library(LIBRT_LIBRARIES rt)
elseif(APPLE)
	find_package(OpenAL)
	find_library(COREAUDIO CoreAudio)
	find_library(AUDIOUNIT AudioUnit)
	find_library(AUDIOTOOLBOX AudioToolbox)
endif()
link_directories(${PROJECT_SOURCE_DIR}/lib)

include_directories(${Boost_INCLUDE_DIR})
include_directories(${Thrift_INCLUDE_DIR})
include_directories(${Spotify_INCLUDE_DIR})

if(UNIX AND NOT APPLE)
	set(ALSA_DEB_INCLUDE_DIR /usr/include/alsa)
	if(ALSA_FOUND)
		include_directories(${ALSA_INCLUDE_DIR})
		include_directories(${ALSA_DEB_INCLUDE_DIR})
		add_definitions("-DHAS_ALSA")
	endif()
	if(OpenAL_FOUND)
		add_definitions("-DHAS_OPENAL")
	endif()
endif()

include_directories(${PROJECT_BINARY_DIR}
	${PROJECT_SOURCE_DIR}/include)

#compiler stuff
set(CMAKE_C_FLAGS "-ggdb -Wall -W -fbuiltin")

#should check for linux explicitly...
if(APPLE)
	#set(CMAKE_OSX_ARCHITECTURES "x86_64")
	add_definitions("-D_OSX")
	if(OPENAL_FOUND)
		add_definitions("-DHAS_OPENAL")
	else()
		add_definitions("-DHAS_AUDIOTOOLKIT")
	endif()
	add_definitions(-DHAVE_INTTYPES_H -DHAVE_NETINET_IN_H)
	#	set(CMAKE_CXX_FLAGS "-std=c++11 -stdlib=libc++ -ggdb -Wall -W -fbuiltin -pthread")
	if(DEBUG)
		set(CMAKE_CXX_FLAGS "-std=c++11 -stdlib=libc++ -g -Wall -W -fbuiltin")
		add_definitions("-D_DEBUG")
	else()
		set(CMAKE_CXX_FLAGS "-std=c++11 -stdlib=libc++ -Wall -W -fbuiltin")
	endif(DEBUG)
else()
	add_definitions("-D_LINUX")
	add_definitions(-DHAVE_INTTYPES_H -DHAVE_NETINET_IN_H)
	if(DEBUG)
		set(CMAKE_CXX_FLAGS "-std=c++0x -g -O0 -Wall -W -fbuiltin -pthread")
		add_definitions("-D_DEBUG")
	else()
		set(CMAKE_CXX_FLAGS "-std=c++0x -Wall -W -fbuiltin -pthread")
	endif(DEBUG)
endif()


# compile as C++0x
#add_definitions(-std=c++0x -DHAVE_INTTYPES_H -DHAVE_NETINET_IN_H)
add_subdirectory(src)
