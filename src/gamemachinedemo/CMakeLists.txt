CMAKE_MINIMUM_REQUIRED (VERSION 2.6)

project (gamemachinedemo)
gm_init_solution()

include_directories(
		../3rdparty/glew-2.1.0/include
		../3rdparty/zlib
		../3rdparty/glm-0.9.9-a2
		../3rdparty/lua/src
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
		demo/texture.h
		demo/texture.cpp
		demo/normalmap.h
		demo/normalmap.cpp
		demo/particles1.h
		demo/particles1.cpp
		demo/particles2.h
		demo/particles2.cpp
		demo/effects.h
		demo/effects.cpp
		demo/quake3_bsp.h
		demo/quake3_bsp.cpp
		demo/border.h
		demo/border.cpp
		demo/sound.h
		demo/sound.cpp
		demo/literature.h
		demo/literature.cpp
		demo/button.h
		demo/button.cpp
		demo/model.h
		demo/model.cpp
		demo/collision.h
		demo/collision.cpp
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