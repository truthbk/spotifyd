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

#build thrift lib
add_library(SpotifydThrift ${ThriftGen_SRCS})

#build audio lib
if(APPLE)
add_library(spotaudio audio/osx-audio.c audio/dummy-audio.c audio/audio.c)
else(APPLE)
if(OpenAL_FOUND)
	set(Audio_SRCS ${Audio_SRCS} audio/openal-audio.c) 
endif(OpenAL_FOUND)
if(ALSA_FOUND)
	set(Audio_SRCS ${Audio_SRCS} audio/alsa-audio.c) 
endif(ALSA_FOUND)
add_library(spotaudio audio/audio.c audio/dummy-audio.c ${Audio_SRCS} )
endif(APPLE)

add_executable(spotifyd ${Spotifyd_SRCS})
target_link_libraries(spotifyd SpotifydThrift spotaudio ${Spotify_LIB} ${Thrift_LIBS} ${LibEvent_LIBS})

add_custom_command(
	OUTPUT    ${ThriftGen_SRCS}
	COMMAND   thrift
	ARGS      -r --gen cpp -o ${CMAKE_CURRENT_SOURCE_DIR}
	${PROJECT_SOURCE_DIR}/thrift/spotify.thrift
	COMMENT   "thrift2cpp"
	)

