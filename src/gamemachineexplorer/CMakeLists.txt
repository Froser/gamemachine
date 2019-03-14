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

include_directories(
		../3rdparty/glm-0.9.9-a2
		../gamemachine/include
		../gamemachineui/include
		../gamemachinemedia/include
		../gamemachineqt/include
		./
		./src
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
		src/shell/mainwindow.h
		src/shell/mainwindow.cpp
		src/shell/mainwindow_actions.h
		src/shell/mainwindow_actions.cpp
		src/shell/view.h
		src/shell/view.cpp

		src/core/handler.h
		src/core/handler.cpp
		src/core/scene_control.h
		src/core/scene_control.cpp
		src/core/scene.h
		src/core/scene.cpp
		src/core/scene_model.h
		src/core/scene_model.cpp
		src/core/util.h
		src/core/util.cpp
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

target_link_libraries(gamemachineexplorer 
	PUBLIC
	Qt5::Core
	Qt5::Gui 
	Qt5::Widgets
	gamemachineqt
	)
target_link_libraries(gamemachineexplorer_dll 
	PUBLIC
	Qt5::Core
	Qt5::Gui 
	Qt5::Widgets
	gamemachineqt
	)

# Copy Dlls (On windows)
if(WIN32)
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt5::Core> $<TARGET_FILE_DIR:${PROJECT_NAME}>
		COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt5::Gui> $<TARGET_FILE_DIR:${PROJECT_NAME}>
		COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt5::Widgets> $<TARGET_FILE_DIR:${PROJECT_NAME}> )
endif(WIN32)

gm_end_project(${PROJECT_NAME})