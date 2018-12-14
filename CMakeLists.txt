cmake_minimum_required (VERSION 3.8)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake-modules" )

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

if(CMAKE_SIZEOF_VOID_P EQUAL 4)
	set(GM_TARGET_X86 TRUE)
else(CMAKE_SIZEOF_VOID_P EQUAL 4)
	set(GM_TARGET_X86 FALSE)
endif(CMAKE_SIZEOF_VOID_P EQUAL 4)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(GM_TARGET_64 TRUE)
else(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(GM_TARGET_64 FALSE)
endif(CMAKE_SIZEOF_VOID_P EQUAL 8)

if (NOT MSVC)
	set(CMAKE_POSITION_INDEPENDENT_CODE OFF)
	set(CMAKE_CXX_FLAGS "-fPIC")
	set(CMAKE_C_FLAGS "-fPIC")
endif (NOT MSVC)

macro(gm_use_gamemachine_dll libraryName)
	set_target_properties(${libraryName} PROPERTIES DEFINE_SYMBOL GM_USE_DLL)
	set_target_properties(${libraryName} PROPERTIES DEFINE_SYMBOL GM_USE_MEDIA_DLL)
endmacro(gm_use_gamemachine_dll libraryName)

macro(gm_link_bullet3 libraryName ACCESS)
	target_link_libraries(${libraryName} ${ACCESS} Bullet3Collision)
	target_link_libraries(${libraryName} ${ACCESS} Bullet3Common)
	target_link_libraries(${libraryName} ${ACCESS} Bullet3Dynamics)
	target_link_libraries(${libraryName} ${ACCESS} BulletCollision)
	target_link_libraries(${libraryName} ${ACCESS} BulletDynamics)
	target_link_libraries(${libraryName} ${ACCESS} BulletInverseDynamics)
	target_link_libraries(${libraryName} ${ACCESS} BulletSoftBody)
	target_link_libraries(${libraryName} ${ACCESS} LinearMath)
	message("GameMachine: ${libraryName} was linked against bullet3.")
endmacro(gm_link_bullet3)

macro(gm_set_target_properties)
	if(MSVC)
		set_target_properties(${libraryName} PROPERTIES COMPILE_FLAGS "/W3" )
		set_target_properties(${libraryName} PROPERTIES COMPILE_FLAGS "/WX" )
	endif(MSVC)
endmacro(gm_set_target_properties)

macro(gm_link_libraries libraryName ACCESS)
	message("GameMachine: ${libraryName} was linked against necessary libraries.")
	target_link_libraries(${libraryName} ${ACCESS} glew_s)
	target_link_libraries(${libraryName} ${ACCESS} lua)
	target_link_libraries(${libraryName} ${ACCESS} png_static)
	target_link_libraries(${libraryName} ${ACCESS} zlibstatic)
	target_link_libraries(${libraryName} ${ACCESS} libjpeg)
	target_link_libraries(${libraryName} ${ACCESS} freetype)
	target_link_libraries(${libraryName} ${ACCESS} assimp)
	target_link_libraries(${libraryName} ${ACCESS} libmad)
	target_link_libraries(${libraryName} ${ACCESS} OpenAL)
endmacro(gm_link_libraries)

macro(gm_link_x11 libraryName ACCESS)
	if(UNIX)
		find_package(X11 REQUIRED)
		target_link_libraries(${libraryName} ${ACCESS} "X11")
		message("GameMachine: ${libraryName} was linked against x11.")
	endif(UNIX)
endmacro(gm_link_x11)

macro(gm_link_pthread libraryName ACCESS)
	if(UNIX)
		# Use pthread
		set(THREADS_PREFER_PTHREAD_FLAG ON)
		find_package(Threads REQUIRED)
		message("GameMachine: ${libraryName} was linked against pthread.")
		target_link_libraries(${libraryName} ${ACCESS} pthread)
	endif(UNIX)
endmacro(gm_link_pthread)

macro(gm_begin_project)
	if(APPLE)
		if(POLICY CMP0025)
			cmake_policy(SET CMP0025 NEW)
		endif()
	endif(APPLE)
	if(WIN32)
		message("GameMachine: Windows system detected.")
		add_definitions(-DGM_WINDOWS)
	endif(WIN32)
	if(UNIX)
		message("GameMachine: Unix-Like system (except APPLE) detected.")
		add_definitions(-DGM_UNIX)
	endif(UNIX)
	set(CMAKE_CXX_STANDARD 14)
	add_definitions(-DFPM_DEFAULT)
	add_definitions(-DUNICODE -D_UNICODE)
	if(MSVC)
		add_definitions(-DGM_MSVC)
	else(MSVC)
		add_definitions(-DGM_GCC)
		add_definitions(-D_GLIBCXX_USE_WCHAR_T)
	endif(MSVC)
	# DirectX Environments:
	if(GM_USE_DX11)
		set(GM_DX11_SDK_PATH "C:/Program Files (x86)/Microsoft DirectX SDK (June 2010)/Include")
		set(GM_DX11_SDK_LIB86_PATH "C:/Program Files (x86)/Microsoft DirectX SDK (June 2010)/Lib/x86")
		set(GM_DX11_SDK_LIB64_PATH "C:/Program Files (x86)/Microsoft DirectX SDK (June 2010)/Lib/x64")
		include_directories(
				${GM_DX11_SDK_PATH}
			)

		if (GM_TARGET_X86)
			link_directories(
					${GM_DX11_SDK_LIB86_PATH}
				)
		endif(GM_TARGET_X86)

		if (GM_TARGET_64)
			link_directories(
					${GM_DX11_SDK_LIB64_PATH}
				)
		endif(GM_TARGET_64)

		link_libraries(
				dxgi.lib
				d3d11.lib
				d3dx10.lib
				d3dx11.lib
				dxerr.lib
				d3dcompiler.lib
				dxguid.lib
			)
	endif(GM_USE_DX11)
endmacro(gm_begin_project)


macro(gm_link_gamemachine_libraries libraryName ACCESS)
	gm_link_bullet3(${libraryName} ${ACCESS})
	gm_link_x11(${libraryName} ${ACCESS})
	gm_link_pthread(${libraryName} ${ACCESS})
	gm_link_libraries(${libraryName} ${ACCESS})
endmacro(gm_link_gamemachine_libraries)

macro(gm_end_project libraryName)
# do nothing
endmacro(gm_end_project)

macro(gm_gamemachine_project libraryName is_static)
	if (${is_static})
		message("GameMachine: ${libraryName} is a static gamemachine program.")
		target_link_libraries(${libraryName} PUBLIC gamemachine_static)
		target_link_libraries(${libraryName} PUBLIC gamemachinemedia_static)
		gm_link_gamemachine_libraries(${libraryName} PUBLIC)
	else(${is_static})
	message("GameMachine: ${libraryName} is a dynamic gamemachine program.")
		target_link_libraries(${libraryName} PUBLIC gamemachine)
		target_link_libraries(${libraryName} PUBLIC gamemachinemedia)
	endif(${is_static})
endmacro(gm_gamemachine_project)

# Options:
option(GM_BUILD_DOCS "Build GameMachine documentation" ON)
option(GM_BUILD_DEMO "Build GameMachine Demos" ON)
option(GM_BUILD_UNITTEST "Build GameMachine Unit Tests" ON)
option(GM_DETECT_MEMORY_LEAK "Detect memory leaking" OFF)

if(WIN32)
	find_package( DirectX )
	if (${DirectX_FOUND})
		message("GameMachine: DirectX found.")
	endif()
	option(GM_USE_DX11 "Use DirectX11 (SDK required)" ${DirectX_FOUND})
endif(WIN32)

# Environments:
if(GM_BUILD_DOCS)
	set (DOXYGEN_SKIP_DOT TRUE)
	find_package(Doxygen)
endif(GM_BUILD_DOCS)

if(GM_DETECT_MEMORY_LEAK)
	add_definitions(-DGM_DETECT_MEMORY_LEAK)
endif(GM_DETECT_MEMORY_LEAK)

if(GM_USE_DX11)
	add_definitions(-DGM_USE_DX11)
endif(GM_USE_DX11)

if(UNIX)
	if(CMAKE_BUILD_TYPE STREQUAL "Debug")
		message("GameMachine: Debug build type detected.")
		add_definitions(-DGM_DEBUG_FLAG)
	endif(CMAKE_BUILD_TYPE STREQUAL "Debug")
endif(UNIX)

# Includes:
set(GM_ZLIB_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/3rdparty/zlib")
set(GM_LIBPNG_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/3rdparty/libpng")

# Projects:
add_subdirectory(src)

if (DOXYGEN_FOUND AND GM_BUILD_DOCS)
	add_subdirectory(doc gamemachinedocs)
endif(DOXYGEN_FOUND AND GM_BUILD_DOCS)