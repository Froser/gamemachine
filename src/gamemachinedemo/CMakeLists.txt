CMAKE_MINIMUM_REQUIRED (VERSION 2.6)

project (gamemachinedemo C CXX)
gm_begin_project()

include_directories(
		../3rdparty/glm-0.9.9-a2
		../gamemachine/include
		../gamemachineui/include
		../gamemachinemedia/include
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
		demo/light.h
		demo/light.cpp
		demo/sound.h
		demo/sound.cpp
		demo/literature.h
		demo/literature.cpp
		demo/model.h
		demo/model.cpp
		demo/csm.h
		demo/csm.cpp
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
		demo/customshader.h
		demo/customshader.cpp
		demo/assimp.h
		demo/assimp.cpp
		demo/customlight.h
		demo/customlight.cpp
	)

gm_source_group_by_dir(SOURCES)

gm_add_msvc_precompiled_header("stdafx.h" "stdafx.cpp" ${SOURCES})

add_executable(${PROJECT_NAME}
		${SOURCES}
	)
gm_gamemachine_project(${PROJECT_NAME} TRUE)

if(MSVC)
	set (gamemachine_dll ${PROJECT_NAME}_dll)
	add_executable(${gamemachine_dll}
		${SOURCES}
	)
	set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS")
	set_target_properties(${gamemachine_dll} PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS")
	gm_gamemachine_project(${gamemachine_dll} FALSE)
endif(MSVC)

gm_end_project(${PROJECT_NAME})