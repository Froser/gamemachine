CMAKE_MINIMUM_REQUIRED (VERSION 2.6)

project (gamemachinedemo)
init_solution()

include_directories(
		../3rdparty/glew/include
		../3rdparty/glew
		../3rdparty/zlib
		../3rdparty/duilib
		../3rdparty/lua/src
		../gamemachine/include
		../gamemachineui/include
		../gamemachinemedia/include
	)

link_directories(
		${demo_SOURCE_DIR}/../3rdparty/glew/lib/release/win32
		${demo_SOURCE_DIR}/../3rdparty/freetype-2.3.5/lib
		${demo_SOURCE_DIR}/../3rdparty/openal1.1/libs/Win32
	)

link_libraries(
		glu32.lib
		opengl32.lib
		glew32s.lib
		freetype.lib
		OpenAL32.lib #windows only
	)

set(SOURCES
		stdafx.cpp
		stdafx.h
		main.cpp

		demostration_world.h
		demostration_world.cpp
		demo/simple.h
		demo/simple.cpp
	)

source_group_by_dir(SOURCES)
add_definitions(-DUNICODE -D_UNICODE)

add_executable(${PROJECT_NAME}
		${SOURCES}
	)

if(MSVC)
	set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS" ) 
endif(MSVC)

add_msvc_precompiled_header("stdafx.h" "stdafx.cpp" ${SOURCES})

link_dependencies(${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME} gamemachine)
target_link_libraries(${PROJECT_NAME} gamemachineui)
target_link_libraries(${PROJECT_NAME} gamemachinemedia)
target_link_libraries(${PROJECT_NAME} png_static)
target_link_libraries(${PROJECT_NAME} zlibstatic)
target_link_libraries(${PROJECT_NAME} libmad)
target_link_libraries(${PROJECT_NAME} duilib)
target_link_libraries(${PROJECT_NAME} libjpeg)
target_link_libraries(${PROJECT_NAME} lua)
