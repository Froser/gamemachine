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

add_library(${PROJECT_NAME}
		${SOURCES}
	)

add_definitions(-DFPM_DEFAULT)
add_definitions(-DUNICODE -D_UNICODE)

# Use openal static library
add_definitions(-DAL_LIBTYPE_STATIC)

if(MSVC)
	set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/W3" ) 
	set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/WX" ) 
endif(MSVC)

target_link_libraries(${PROJECT_NAME} libmad)
target_link_libraries(${PROJECT_NAME} OpenAL)
gm_end_project(${PROJECT_NAME})