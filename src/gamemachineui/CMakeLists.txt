project (gamemachineui)
gm_init_solution()

include_directories(
		./
		./include
		../3rdparty/glew-2.1.0/include
		../3rdparty/glm-0.9.9-a2
		../gamemachine/include
	)

set(SOURCES
		stdafx.cpp
		stdafx.h
		common.h

		src/gmui/gmui.h
	)

set(WIN32_SOURCES
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

set(WIN32_DX11
		src/windows/gmuidx11window.cpp
		src/windows/gmuidx11window.h
	)

set(DUILIB_SOURCES
		src/windows/duilib/gmuiduilibwindow.cpp
		src/windows/duilib/gmuiduilibwindow.h
		src/windows/duilib/gmuiconsole.cpp
		src/windows/duilib/gmuiconsole.h
		src/windows/duilib/gmuiconsole_ui.h
		src/windows/duilib/gmuicontrols.cpp
		src/windows/duilib/gmuicontrols.h
	)
	
if(WIN32)
	list(APPEND SOURCES ${WIN32_SOURCES})
endif(WIN32)

if(GM_USE_DX11)
	list(APPEND SOURCES ${WIN32_DX11})
	link_libraries(
		d3d11.lib
	)
endif(GM_USE_DX11)

if(GM_USE_DUILIB)
	list(APPEND SOURCES ${DUILIB_SOURCES})
	include_directories(
			../3rdparty/duilib
		)
endif(GM_USE_DUILIB)

gm_source_group_by_dir(SOURCES)

if(WIN32)
	gm_add_msvc_precompiled_header("stdafx.h" "stdafx.cpp" ${SOURCES})
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

if(GM_USE_DUILIB)
	target_link_libraries(${PROJECT_NAME} duilib)
endif(GM_USE_DUILIB)