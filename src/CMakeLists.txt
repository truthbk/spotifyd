set(ThriftGen_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/gen-cpp/Spotify.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/gen-cpp/spotify_constants.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/gen-cpp/spotify_types.cpp
	)

set(Spotifyd_SRCS
	spotifyd_srv.cpp
	)

#if this doesn't work add it to the top-level
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/gen-cpp)

add_library(SpotifydTrhift ${ThriftGen})
add_library(spotify SHARED IMPORTED)
add_executable(spotifyd Spotifyd_SRCS)
target_link_libraries(${Thrift_LIB} spotify)

add_custom_command(
	OUTPUT    ${ThriftGen_SRCS}
	COMMAND   thrift
	ARGS      -r --gen cpp -o ${CMAKE_CURRENT_SOURCE_DIR}
	${PROJECT_SOURCE_DIR}/thrift/spotify.thrift
	COMMENT   "thrift2cpp"
	)

