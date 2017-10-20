cmake_minimum_required (VERSION 3.8)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

macro(source_group_by_dir source_files)
	foreach(sgbd_file ${${source_files}})
		string(REGEX REPLACE "(.*)/(.*)" \\1 sgbd_group_name ${sgbd_file})
		string(COMPARE EQUAL ${sgbd_file} ${sgbd_group_name} sgbd_nogroup)
		string(REPLACE "/" "\\" sgbd_group_name ${sgbd_group_name})
		if(sgbd_nogroup)
			set(sgbd_group_name ".")
		endif(sgbd_nogroup)
		source_group(${sgbd_group_name} FILES ${sgbd_file})
	endforeach(sgbd_file)
endmacro(source_group_by_dir)

macro(add_msvc_precompiled_header PrecompiledHeader PrecompiledSource SourcesVar)
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
endmacro(add_msvc_precompiled_header)

#Runtime library
set(RUNTIME_LIBRARY MD)

macro(apply_runtime_library)
	if (WIN32)
		if (${RUNTIME_LIBRARY} STREQUAL MD)
			set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
			set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")
		elseif(${RUNTIME_LIBRARY} STREQUAL MT)
			set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
			set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
		endif(${RUNTIME_LIBRARY} STREQUAL MD)
	endif(WIN32)
endmacro(apply_runtime_library)
apply_runtime_library()

macro(init_solution)
	add_compile_options(-std=c++14)
	add_definitions(-DFPM_DEFAULT)
	add_definitions(-DUNICODE -D_UNICODE)

	if(MINGW)
		add_definitions(-D_GLIBCXX_USE_WCHAR_T)
		if (WIN32)
			add_definitions(-D_WINDOWS)
		endif(WIN32)
	endif(MINGW)
endmacro(init_solution)

macro(link_dependencies projectName)
	if(MINGW)
		find_package (Threads REQUIRED)
		target_link_libraries (${projectName} ${CMAKE_THREAD_LIBS_INIT})
	endif(MINGW)
endmacro(link_dependencies)

add_subdirectory(./3rdparty/zlib zlib)
add_subdirectory(./3rdparty/libpng libpng)
add_subdirectory(./3rdparty/libmad libmad)
add_subdirectory(./3rdparty/jpeg-6b libjpeg)
add_subdirectory(./3rdparty/lua lua)
if(WIN32)
	add_subdirectory(./3rdparty/duilib duilib)
endif(WIN32)

add_subdirectory(gamemachine gamemachine)
add_subdirectory(gamemachineui gamemachineui)
add_subdirectory(gamemachinemedia gamemachinemedia)
add_subdirectory(gamemachinedemo gamemachinedemo)
