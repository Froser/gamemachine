CMAKE_MINIMUM_REQUIRED (VERSION 2.6)

PROJECT (demo)

include_directories(
		../3rdparty/glew/include
		../3rdparty/glew
		../3rdparty/zlib
		../3rdparty/duilib
		../3rdparty/lua/src
		../gamemachine/src
	)

link_directories(
		${demo_SOURCE_DIR}/../3rdparty/glew/lib/release/win32
		${demo_SOURCE_DIR}/../3rdparty/freetype-2.3.5/lib
		${demo_SOURCE_DIR}/../3rdparty/dx #windows only
	)

link_libraries(
		glu32.lib
		opengl32.lib
		glew32s.lib
		freetype.lib
		dxguid.lib #windows only
	)

set(SOURCES
		main.cpp
	)

source_group_by_dir(SOURCES)
add_definitions(-DUNICODE -D_UNICODE)

add_executable(demo
		${SOURCES}
	)

if(MSVC)
	set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS" ) 
endif(MSVC)

target_link_libraries(${PROJECT_NAME} gamemachine)
target_link_libraries(${PROJECT_NAME} png_static)
target_link_libraries(${PROJECT_NAME} zlibstatic)
target_link_libraries(${PROJECT_NAME} libmad)
target_link_libraries(${PROJECT_NAME} duilib)
target_link_libraries(${PROJECT_NAME} libjpeg)
target_link_libraries(${PROJECT_NAME} lua)
