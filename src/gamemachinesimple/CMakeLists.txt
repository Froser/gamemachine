CMAKE_MINIMUM_REQUIRED (VERSION 2.6)

project (gamemachinesimple)
gm_init_solution()

include_directories(
		../3rdparty/glm-0.9.9-a2
		../gamemachine/include
		../gamemachineui/include
		../gamemachinemedia/include
		./
	)

link_libraries(
		glu32.lib
		opengl32.lib
	)

set(SOURCES
		stdafx.cpp
		stdafx.h
		main.cpp

		demostration_world.h
		demostration_world.cpp
	)

gm_source_group_by_dir(SOURCES)
add_executable(${PROJECT_NAME}
		${SOURCES}
	)

if(MSVC)
	set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS") 
endif(MSVC)

gm_add_msvc_precompiled_header("stdafx.h" "stdafx.cpp" ${SOURCES})

target_link_libraries(${PROJECT_NAME} gamemachine)
target_link_libraries(${PROJECT_NAME} gamemachineui)
target_link_libraries(${PROJECT_NAME} gamemachinemedia)