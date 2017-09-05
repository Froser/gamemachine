project (gamemachineui)

include_directories(
		./
		../3rdparty/glew/include
		../3rdparty/duilib
		../gamemachine/include
	)

set(SOURCES
		stdafx.cpp
		stdafx.h
		common.h

		src/gmui/gmui.h
		src/gmui/gmui.cpp
		src/gmui/gmuicontrols.h
		src/gmui/gmuicontrols.cpp
		src/gmui/gmui_glwindow.h
		src/gmui/gmui_glwindow.cpp
		src/gmui/gmui_console.h
		src/gmui/gmui_console_ui.h
		src/gmui/gmui_console.cpp
	)

source_group_by_dir(SOURCES)
add_msvc_precompiled_header("stdafx.h" "stdafx.cpp" ${SOURCES})

add_library(gamemachineui
		${SOURCES}
	)

add_definitions(-DFPM_DEFAULT)
add_definitions(-DUNICODE -D_UNICODE)

if(MSVC)
	set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/W3" ) 
	set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/WX" ) 
endif(MSVC)
