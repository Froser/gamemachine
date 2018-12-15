project (gamemachinemedia)
gm_begin_project()

include_directories(
		./
		./include
		./src
		../3rdparty/openal-soft-1.18.2/include
		../3rdparty/libmad
		../gamemachine/include
		../3rdparty/glm-0.9.9-a2
	)

set(SOURCES
		stdafx.cpp
		stdafx.h
		common.h

		src/gmm/gmm.h
		src/common/gmmfactory.cpp
		src/common/utilities/gmmstream.cpp
		src/common/utilities/gmmstream.h
		src/common/audio/aldlist.cpp
		src/common/audio/aldlist.h
		src/common/audio/gmmaudioplayer.h
		src/common/audio/gmmaudioplayer.cpp
		src/common/audio/gmmaudioreader.h
		src/common/audio/gmmaudioreader_stream.h
		src/common/audio/gmmaudioreader_stream.cpp
		src/common/audio/gmmaudioreader_mp3.h
		src/common/audio/gmmaudioreader_mp3.cpp
		src/common/audio/gmmaudioreader.cpp
		src/common/audio/gmmaudioreader_wav.h
		src/common/audio/gmmaudioreader_wav.cpp
		src/common/audio/alframework/cwaves.cpp
		src/common/audio/alframework/cwaves.h
	)

gm_source_group_by_dir(SOURCES)

if(WIN32)
	gm_add_msvc_precompiled_header("stdafx.h" "stdafx.cpp" ${SOURCES})
endif(WIN32)

add_definitions(-DFPM_DEFAULT)
add_definitions(-DUNICODE -D_UNICODE)

# Use openal static library
add_definitions(-DAL_LIBTYPE_STATIC)

set (gm_static_name gamemachinemedia_static)
add_library(${gm_static_name} STATIC
		${SOURCES}
	)
gm_set_target_properties(${gm_static_name})

if(MSVC)
	set (gm_shared_name gamemachinemedia)
	add_library(${gm_shared_name} SHARED
		${SOURCES}
	)
	set_target_properties(${gm_shared_name} PROPERTIES DEFINE_SYMBOL GM_MEDIA_DLL)
	gm_set_target_properties(${gm_shared_name})
	target_link_libraries(${gm_shared_name} PRIVATE gamemachine)
	target_link_libraries(${gm_shared_name} PRIVATE libmad)
	target_link_libraries(${gm_shared_name} PRIVATE OpenAL)
else(MSVC)
	# GCC requires link following library
	target_link_libraries(${gm_static_name} gamemachine_static)
endif(MSVC)
