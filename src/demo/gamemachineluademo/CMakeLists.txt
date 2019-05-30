CMAKE_MINIMUM_REQUIRED (VERSION 2.6)

project (gamemachineluademo C CXX)
gm_begin_project()

include_directories(
		../../3rdparty/glm-0.9.9-a2
		../../gamemachine/include
		../../gamemachineui/include
		../../gamemachinemedia/include
		./
	)

IF(WIN32)
	link_libraries(
			glu32.lib
			opengl32.lib
		)
endif(WIN32)

set(SOURCES
		stdafx.cpp
		stdafx.h
		main.cpp
	)

gm_source_group_by_dir(SOURCES)

add_executable(${PROJECT_NAME}
		${SOURCES}
	)
gm_folder_with_name(${PROJECT_NAME} gamemachinedemo)
gm_gamemachine_project(${PROJECT_NAME} TRUE)

if(MSVC)
	add_executable(${PROJECT_NAME}_dll
		${SOURCES}
	)
	gm_folder_with_name(${PROJECT_NAME}_dll gamemachinedemo)

	set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE") 
	set_target_properties(${PROJECT_NAME}_dll PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE") 
	gm_gamemachine_project(${PROJECT_NAME}_dll FALSE)
endif(MSVC)

gm_add_msvc_precompiled_header("stdafx.h" "stdafx.cpp" ${SOURCES})
gm_end_project(${PROJECT_NAME})