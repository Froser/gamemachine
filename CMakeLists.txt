cmake_minimum_required (VERSION 3.8)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

macro(gm_folder foldername)
	set_target_properties(${foldername} PROPERTIES FOLDER ${PROJECT_NAME})
endmacro(gm_folder)

macro(gm_source_group_by_dir source_files)
	foreach(sgbd_file ${${source_files}})
		string(REGEX REPLACE "(.*)/(.*)" \\1 sgbd_group_name ${sgbd_file})
		string(COMPARE EQUAL ${sgbd_file} ${sgbd_group_name} sgbd_nogroup)
		string(REPLACE "/" "\\" sgbd_group_name ${sgbd_group_name})
		if(sgbd_nogroup)
			set(sgbd_group_name ".")
		endif(sgbd_nogroup)
		source_group(${sgbd_group_name} FILES ${sgbd_file})
	endforeach(sgbd_file)
endmacro(gm_source_group_by_dir)

macro(gm_add_msvc_precompiled_header PrecompiledHeader PrecompiledSource SourcesVar)
	if(MSVC)
		get_filename_component(PrecompiledBasename ${PrecompiledHeader} NAME_WE)
		set(PrecompiledBinary "${CMAKE_CURRENT_BINARY_DIR}/${PrecompiledBasename}.pch")
		set(Sources ${${SourcesVar}})

		set_source_files_properties(${PrecompiledSource}
				PROPERTIES COMPILE_FLAGS "/Yc\"${PrecompiledHeader}\" /Fp\"${PrecompiledBinary}\""
				OBJECT_OUTPUTS "${PrecompiledBinary}"
			)
		set_source_files_properties(${Sources}
				PROPERTIES COMPILE_FLAGS "/Yu\"${PrecompiledHeader}\" /FI\"${PrecompiledHeader}\" /Fp\"${PrecompiledBinary}\""
				OBJECT_DEPENDS "${PrecompiledBinary}"
			)
		# Add precompiled header to SourcesVar
		list(APPEND ${SourcesVar} ${PrecompiledSource})
	endif(MSVC)
endmacro(gm_add_msvc_precompiled_header)

#Runtime library
set(RUNTIME_LIBRARY MD)

macro(gm_apply_runtime_library)
	if (WIN32)
		if (${RUNTIME_LIBRARY} STREQUAL MD)
			set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
			set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")
		elseif(${RUNTIME_LIBRARY} STREQUAL MT)
			set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
			set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
		endif(${RUNTIME_LIBRARY} STREQUAL MD)
	endif(WIN32)
endmacro(gm_apply_runtime_library)
gm_apply_runtime_library()

macro(gm_init_solution)
	add_compile_options(-std=c++14)
	add_definitions(-DFPM_DEFAULT)
	add_definitions(-DUNICODE -D_UNICODE)

	if(MINGW)
		add_definitions(-D_GLIBCXX_USE_WCHAR_T)
		if (WIN32)
			add_definitions(-D_WINDOWS)
		endif(WIN32)
	endif(MINGW)
endmacro(gm_init_solution)

macro(gm_link_dependencies projectName)
	if(MINGW)
		find_package (Threads REQUIRED)
		target_link_libraries (${projectName} ${CMAKE_THREAD_LIBS_INIT})
	endif(MINGW)
endmacro(gm_link_dependencies)

# Options:
option(GM_BUILD_DOCS "Build GameMachine documentation" ON)
option(GM_BUILD_DEMO "Build GameMachine Demos" ON)
option(GM_BUILD_UNITTEST "Build GameMachine Unit Tests" ON)

if (GM_BUILD_DOCS)
	set (DOXYGEN_SKIP_DOT TRUE)
	find_package(Doxygen)
endif()

# Projects:
add_subdirectory(src)

if (DOXYGEN_FOUND AND GM_BUILD_DOCS)
	add_subdirectory(doc gamemachinedocs)
endif()