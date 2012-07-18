# - Find Spotify
# Find the native Google Sparse Hash Etc includes
#
#  Spotify_INCLUDE_DIR - where to find sparse_hash_set, etc.
#  Spotify_FOUND       - True if Spotify found.


if (Spotify_INCLUDE_DIR)
	# Already in cache, be silent
	set(Spotify_FIND_QUIETLY TRUE)
endif ()

find_path(Spotify_INCLUDE_DIR api.h PATHS
	/opt/local/include/libspotify
	/usr/local/include/libspotify
	/usr/include/libspotify
	)

set(Spotify_LIB_PATHS /usr/local/lib /opt/local/lib)
find_library(Spotify_LIB NAMES spotify PATHS ${Spotify_LIB_PATHS})

if (Spotify_INCLUDE_DIR AND Spotify_LIB)
	set(Spotify_FOUND TRUE)
else ()
	set(Spotify_FOUND FALSE)
endif ()

if (Spotify_FOUND)
	if (NOT Spotify_FIND_QUIETLY)
		message(STATUS "Found Spotify: ${Spotify_INCLUDE_DIR}")
	endif ()
else ()
	message(STATUS "Not Found Spotify: ${Spotify_INCLUDE_DIR}")
	if (Spotify_FIND_REQUIRED)
		message(FATAL_ERROR "Could NOT find Spotify includes")
	endif ()
endif ()


mark_as_advanced(
	Spotify_LIB
	Spotify_INCLUDE_DIR
	)
