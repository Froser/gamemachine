project (gamemachineui)
init_solution()

include_directories(
		./
		./include
		../3rdparty/glew/include
		../3rdparty/duilib
		../gamemachine/include
	)

set(SOURCES
		stdafx.cpp
		stdafx.h
		common.h

		src/gmui/gmui.h
	)

set(WIN32_SOURCES
		src/windows/gmuiconsole.cpp
		src/windows/gmuiconsole.h
		src/windows/gmuiconsole_ui.h
		src/windows/gmuicontrols.cpp
		src/windows/gmuicontrols.h
		src/windows/gmuidef.h
		src/windows/gmuifactory.cpp
		src/windows/gmuiglwindow.cpp
		src/windows/gmuiglwindow.h
		src/windows/gmuiguiwindow.cpp
		src/windows/gmuiguiwindow.h
		src/windows/gmuiwindow.cpp
		src/windows/gmuiwindow.h
		src/windows/gmuiinput.cpp
		src/windows/gmuiinput.h
	)
	
if(WIN32)
	list(APPEND SOURCES ${WIN32_SOURCES})
endif(WIN32)

source_group_by_dir(SOURCES)

if(WIN32)
	add_msvc_precompiled_header("stdafx.h" "stdafx.cpp" ${SOURCES})
endif(WIN32)

add_library(gamemachineui
		${SOURCES}
	)

add_definitions(-DFPM_DEFAULT)
add_definitions(-DUNICODE -D_UNICODE)

if(MSVC)
	set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/W3" ) 
	set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/WX" ) 
endif(MSVC)
