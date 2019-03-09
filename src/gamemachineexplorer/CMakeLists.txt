CMAKE_MINIMUM_REQUIRED (VERSION 2.6)

project (gamemachineexplorer C CXX)

gm_begin_project()
# Instruct CMake to run moc automatically when needed
set(CMAKE_AUTOMOC ON)

# Create code from a list of Qt designer ui files
set(CMAKE_AUTOUIC ON)

# Include QT
include_directories(
	${Qt5Core_INCLUDE_DIRS}
	${Qt5Widgets_INCLUDE_DIRS}
)

target_link_libraries(${Qt5Widgets_LIBRARIES})

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

		src/shell/application.h
		src/shell/application.cpp
	)

gm_source_group_by_dir(SOURCES)

gm_add_msvc_precompiled_header("stdafx.h" "stdafx.cpp" ${SOURCES})

add_executable(${PROJECT_NAME}
		${SOURCES}
	)
gm_gamemachine_project(${PROJECT_NAME} TRUE)

if(MSVC)
	set (gamemachineexplorer_dll ${PROJECT_NAME}_dll)
	add_executable(${gamemachineexplorer_dll}
		${SOURCES}
	)
	set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS")
	set_target_properties(${gamemachineexplorer_dll} PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS")
	gm_gamemachine_project(${gamemachineexplorer_dll} FALSE)
endif(MSVC)

gm_end_project(${PROJECT_NAME})