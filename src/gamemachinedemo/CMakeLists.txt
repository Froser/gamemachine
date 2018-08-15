CMAKE_MINIMUM_REQUIRED (VERSION 2.6)

project (gamemachinedemo)
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

		demonstration_world.h
		demonstration_world.cpp
		demo/texture.h
		demo/texture.cpp
		demo/normalmap.h
		demo/normalmap.cpp
		demo/effects.h
		demo/effects.cpp
		demo/quake3_bsp.h
		demo/quake3_bsp.cpp
		demo/sound.h
		demo/sound.cpp
		demo/literature.h
		demo/literature.cpp
		demo/model.h
		demo/model.cpp
		demo/collision.h
		demo/collision.cpp
		demo/specularmap.h
		demo/specularmap.cpp
		demo/pbr.h
		demo/pbr.cpp
		demo/phong_pbr.h
		demo/phong_pbr.cpp
		demo/controls.h
		demo/controls.cpp
		demo/lua.h
		demo/lua.cpp
		demo/particle.h
		demo/particle.cpp
		demo/md5mesh.h
		demo/md5mesh.cpp
		demo/terrain.h
		demo/terrain.cpp
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
target_link_libraries(${PROJECT_NAME} gamemachinemedia)