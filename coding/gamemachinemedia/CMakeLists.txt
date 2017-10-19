project (gamemachinemedia)
init_solution()

include_directories(
		./
		./include
		./src
		../3rdparty/glew/include
		../3rdparty/openal1.1/include
		../3rdparty/libmad
		../3rdparty/lua/src
		../gamemachine/include
	)

set(SOURCES
		stdafx.cpp
		stdafx.h
		common.h

		src/gmm/gmm.h
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
	)

set(WIN32_SOURCES
		src/windows/gmmfactory.cpp
		src/windows/gmmdef.h
		src/windows/audio/alframework/cwaves.cpp
		src/windows/audio/alframework/cwaves.h
		src/windows/audio/gmmaudioreader.cpp
		src/windows/audio/gmmaudioreader_wav.h
		src/windows/audio/gmmaudioreader_wav.cpp
	)

if(WIN32)
	list(APPEND SOURCES ${WIN32_SOURCES})
endif(WIN32)

source_group_by_dir(SOURCES)

if(WIN32)
	add_msvc_precompiled_header("stdafx.h" "stdafx.cpp" ${SOURCES})
endif(WIN32)

add_library(${PROJECT_NAME}
		${SOURCES}
	)

add_definitions(-DFPM_DEFAULT)
add_definitions(-DUNICODE -D_UNICODE)

if(MSVC)
	set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/W3" ) 
	set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/WX" ) 
endif(MSVC)
