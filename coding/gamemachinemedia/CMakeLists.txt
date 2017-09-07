project (gamemachinemedia)
init_solution()

include_directories(
		./
		./include
		../3rdparty/glew/include
		../3rdparty/openal1.1/include
		../gamemachine/include
	)

set(SOURCES
		stdafx.cpp
		stdafx.h
		common.h

		src/gmm/gmm.h
		src/common/audio/gmmaudioplayer.h
		src/common/audio/gmmaudioplayer.cpp
	)

set(WIN32_SOURCES
		src/windows/gmmdef.h
		src/windows/audio/gmmaudioreader.h
		src/windows/audio/gmmaudioreader.cpp
		src/windows/audio/gmmaudioreader_wav.h
		src/windows/audio/gmmaudioreader_wav.cpp
	)

source_group_by_dir(SOURCES)

if(WIN32)
	add_msvc_precompiled_header("stdafx.h" "stdafx.cpp" ${SOURCES} ${WIN32_SOURCES})
	add_library(gamemachinemedia
			${SOURCES}
			${WIN32_SOURCES}
		)
	source_group_by_dir(WIN32_SOURCES)
endif(WIN32)

add_definitions(-DFPM_DEFAULT)
add_definitions(-DUNICODE -D_UNICODE)

if(MSVC)
	set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/W3" ) 
	set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/WX" ) 
endif(MSVC)
