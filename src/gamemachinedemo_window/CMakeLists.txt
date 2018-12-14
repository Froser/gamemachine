CMAKE_MINIMUM_REQUIRED (VERSION 2.6)

project (gamemachinedemo_window)
gm_begin_project()

include_directories(
		../3rdparty/glm-0.9.9-a2
		../gamemachine/include
		./
	)

if(WIN32)
	link_libraries(
			glu32.lib
			opengl32.lib
		)
endif(WIN32)

set(SOURCES
		stdafx.h
		stdafx.cpp
		main.cpp
		main.rc
		resource.h
	)

gm_source_group_by_dir(SOURCES)
add_definitions(-DUNICODE -D_UNICODE)

add_executable(${PROJECT_NAME}
		${SOURCES}
	)

if(MSVC)
	set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS" ) 
endif(MSVC)

gm_add_msvc_precompiled_header("stdafx.h" "stdafx.cpp" ${SOURCES})

gm_gamemachine_project(${PROJECT_NAME} TRUE)

gm_end_project(${PROJECT_NAME})
