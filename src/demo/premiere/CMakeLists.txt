CMAKE_MINIMUM_REQUIRED (VERSION 2.6)

project (premiere C CXX)
gm_begin_project()

include_directories(
		../../3rdparty/glm-0.9.9-a2
		../../gamemachine/include
		../../gamemachineui/include
		../../gamemachinemedia/include
		./
	)

if(WIN32)
	link_libraries(
			glu32.lib
			opengl32.lib
		)
endif(WIN32)

set(SOURCES
		stdafx.cpp
		stdafx.h
		main.cpp

		handler.h
		handler.cpp
		timeline.h
		timeline.cpp
		procedures.h
		procedures.cpp
		helper.h
		helper.cpp
	)

if (MSVC)
	set(RESOURCES
		resource.h
		Resource.rc
	)
	list (APPEND SOURCES ${RESOURCES})
endif(MSVC)

gm_source_group_by_dir(SOURCES)

gm_add_msvc_precompiled_header("stdafx.h" "stdafx.cpp" ${SOURCES})

add_executable(${PROJECT_NAME}
		${SOURCES}
	)

gm_gamemachine_project(${PROJECT_NAME} TRUE)
gm_folder_with_name(${PROJECT_NAME} gamemachinedemo)

gm_copy_resource(${PROJECT_NAME})
add_custom_command(
		TARGET ${PROJECT_NAME}
		POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E tar cv
		"$<TARGET_FILE_DIR:${PROJECT_NAME}>/gm.pk1"
		--format=zip --
		"."
		WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/media/premiere"
		VERBATIM
)

if(MSVC)
	set (premiere_dll ${PROJECT_NAME}_dll)
	add_executable(${premiere_dll}
		${SOURCES}
	)
	gm_folder_with_name(${PROJECT_NAME}_dll gamemachinedemo)
	set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS")
	set_target_properties(${premiere_dll} PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS")
	gm_gamemachine_project(${premiere_dll} FALSE)

	gm_copy_resource(${premiere_dll})
	add_custom_command(
			TARGET ${premiere_dll}
			POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E tar cv
			"$<TARGET_FILE_DIR:${premiere_dll}>/gm.pk1"
			--format=zip --
			"."
			WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/media/premiere"
			VERBATIM
	)

endif(MSVC)

gm_end_project(${PROJECT_NAME})