set(ThriftGen_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/gen-cpp/Spotify.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/gen-cpp/spotify_constants.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/gen-cpp/spotify_types.cpp
	)

set(ThriftGen_IPC_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/gen-cpp/SpotifyIPC.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/gen-cpp/spotify_ipc_constants.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/gen-cpp/spotify_ipc_types.cpp
	)

set(ThriftPyGen_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/gen-py/__init__.py
	${CMAKE_CURRENT_SOURCE_DIR}/gen-py/spotify/__init__.py
	${CMAKE_CURRENT_SOURCE_DIR}/gen-py/spotify/constants.py
	${CMAKE_CURRENT_SOURCE_DIR}/gen-py/spotify/types.py
	${CMAKE_CURRENT_SOURCE_DIR}/gen-py/spotify/Spotify.py
	)

set(Spotifyd_SRCS
	spotifyd_srv.cpp
	spotify_ipc_srv.cpp
	xplodify_sess.cpp
	xplodify_plc.cpp
	xplodify_pl.cpp
	xplodify_track.cpp
	)

#if this doesn't work add it to the top-level
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/gen-cpp)

#build thrift lib
add_library(SpotifydThrift ${ThriftGen_SRCS})
add_library(SpotifydIPCThrift ${ThriftGen_IPC_SRCS})

#build audio lib
if(APPLE)
	if(OPENAL_FOUND)
		set(Audio_SRCS ${Audio_SRCS} audio/openal-audio.c) 
	endif(OPENAL_FOUND)
	add_library(spotaudio audio/audio.c audio/osx-audio.c audio/dummy-audio.c ${Audio_SRCS})
else(APPLE)
	if(OPENAL_FOUND)
		set(Audio_SRCS ${Audio_SRCS} audio/openal-audio.c) 
	endif(OPENAL_FOUND)
	if(ALSA_FOUND)
		set(Audio_SRCS ${Audio_SRCS} audio/alsa-audio.c) 
	endif(ALSA_FOUND)
	add_library(spotaudio audio/audio.c audio/dummy-audio.c ${Audio_SRCS} )
endif(APPLE)

add_executable(spotifyd ${Spotifyd_SRCS})
if(UNIX AND NOT APPLE)
	target_link_libraries(spotifyd SpotifydThrift SpotifydIPCThrift spotaudio ${Boost_LIBRARIES} ${ALSA_LIBRARIES} ${Spotify_LIB} ${Thrift_LIBS} ${LibEvent_LIBS} ${LIBRT_LIBRARIES})
elseif(APPLE)
	target_link_libraries(spotifyd SpotifydThrift SpotifydIPCThrift spotaudio ${Boost_LIBRARIES} ${Spotify_LIB} ${Thrift_LIBS} ${LibEvent_LIBS} ${COREAUDIO} ${AUDIOUNIT} ${AUDIOTOOLBOX} ${OPENAL_LIBRARY})
endif()

add_custom_command(
	OUTPUT    ${ThriftGen_SRCS}
	COMMAND   thrift
	ARGS      -r --gen cpp -o ${CMAKE_CURRENT_SOURCE_DIR}
	${PROJECT_SOURCE_DIR}/thrift/spotify.thrift
	COMMENT   "thrift2cpp SRV"
	)

add_custom_command(
	OUTPUT    ${ThriftGen_IPC_SRCS}
	COMMAND   thrift
	ARGS      -r --gen cpp -o ${CMAKE_CURRENT_SOURCE_DIR}
	${PROJECT_SOURCE_DIR}/thrift/spotify_ipc.thrift
	COMMENT   "thrift2cpp IPC"
	)

add_custom_command(
	OUTPUT    ${ThriftPyGen_SRCS}
	COMMAND   thrift
	ARGS      -r --gen py -o ${CMAKE_CURRENT_SOURCE_DIR}
	${PROJECT_SOURCE_DIR}/thrift/spotify.thrift
	COMMENT   "thrift2py SRV"
	)

add_custom_target(
	run ALL
	DEPENDS ${ThriftPyGen_SRCS}
	)

# Gather list of all .xml and .conf files in "/config"
file(GLOB ThriftHeaderFiles ${CMAKE_CURRENT_SOURCE_DIR}/gen-cpp/*.h)

foreach(HeaderFile ${ThriftHeaderFiles})
	add_custom_command(TARGET SpotifydThrift POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E
		copy ${HeaderFile} ${CMAKE_CURRENT_SOURCE_DIR}/../include/
		COMMENT   "Copying Thrift header file: ${HeaderFile}" )
endforeach()
