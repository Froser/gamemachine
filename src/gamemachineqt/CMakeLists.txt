CMAKE_MINIMUM_REQUIRED (VERSION 2.6)

project (gamemachineqt C CXX)

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

		src/forwards.h
		src/gamemachinewidget.h
		src/gamemachinewidget.cpp
	)

gm_source_group_by_dir(SOURCES)

gm_add_msvc_precompiled_header("stdafx.h" "stdafx.cpp" ${SOURCES})

add_library(${PROJECT_NAME}
		${SOURCES}
	)

target_link_libraries(gamemachineqt
	PUBLIC
	Qt5::Core
	Qt5::Gui 
	Qt5::Widgets
	)

gm_end_project(${PROJECT_NAME})