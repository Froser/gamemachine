cmake_minimum_required (VERSION 3.0)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

macro(source_group_by_dir source_files)
	if(MSVC)
		foreach(sgbd_file ${${source_files}})
			string(REGEX REPLACE "(.*)/(.*)" \\1 sgbd_group_name ${sgbd_file})
			string(COMPARE EQUAL ${sgbd_file} ${sgbd_group_name} sgbd_nogroup)
			string(REPLACE "/" "\\" sgbd_group_name ${sgbd_group_name})
			if(sgbd_nogroup)
				set(sgbd_group_name ".")
			endif(sgbd_nogroup)
			source_group(${sgbd_group_name} FILES ${sgbd_file})
		endforeach(sgbd_file)
	endif(MSVC)

endmacro(source_group_by_dir)

macro(add_msvc_precompiled_header PrecompiledHeader PrecompiledSource SourcesVar)
  if(MSVC)
    get_filename_component(PrecompiledBasename ${PrecompiledHeader} NAME_WE)
    set(PrecompiledBinary "${CMAKE_CURRENT_BINARY_DIR}/${PrecompiledBasename}.pch")
    set(Sources ${${SourcesVar}})

    set_source_files_properties(${PrecompiledSource}
                                PROPERTIES COMPILE_FLAGS "/Yc\"${PrecompiledHeader}\" /Fp\"${PrecompiledBinary}\""
                                           OBJECT_OUTPUTS "${PrecompiledBinary}")
    set_source_files_properties(${Sources}
                                PROPERTIES COMPILE_FLAGS "/Yu\"${PrecompiledHeader}\" /FI\"${PrecompiledHeader}\" /Fp\"${PrecompiledBinary}\""
                                           OBJECT_DEPENDS "${PrecompiledBinary}")  
    # Add precompiled header to SourcesVar
    list(APPEND ${SourcesVar} ${PrecompiledSource})
  endif(MSVC)
endmacro(add_msvc_precompiled_header)

add_subdirectory(./3rdparty/zlib zlib)
add_subdirectory(./3rdparty/libpng libpng)
add_subdirectory(gamemachine gamemachine)
add_subdirectory(demo demo)
