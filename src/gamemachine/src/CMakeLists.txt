include_directories(
		./
		../include
		../../3rdparty/glew-2.1.0/include
		../../3rdparty/libpng
		../../3rdparty/jpeg-6b
		../../3rdparty/zlib
		../../3rdparty/freetype-2.8.1/include
		../../3rdparty/libmad
		../../3rdparty/lua/src
		../../3rdparty/glm-0.9.9-a2
		../../3rdparty/bullet3-2.87/src
	)

gm_init_solution()

set(SOURCES
		stdafx.cpp
		stdafx.h
		common.h

		gmdata/glyph/gmglyphmanager.h
		gmdata/glyph/gmglyphmanager.cpp
		gmdata/imagereader/gmimagereader.h
		gmdata/imagereader/gmimagereader.cpp
		gmdata/imagereader/gmimagereader_bmp.h
		gmdata/imagereader/gmimagereader_bmp.cpp
		gmdata/imagereader/gmimagereader_dds.h
		gmdata/imagereader/gmimagereader_dds.cpp
		gmdata/imagereader/gmimagereader_jpg.h
		gmdata/imagereader/gmimagereader_jpg.cpp
		gmdata/imagereader/gmimagereader_png.h
		gmdata/imagereader/gmimagereader_png.cpp
		gmdata/imagereader/gmimagereader_tga.h
		gmdata/imagereader/gmimagereader_tga.cpp
		gmdata/modelreader/gmmodelreader.h
		gmdata/modelreader/gmmodelreader.cpp
		gmdata/modelreader/gmmodelreader_obj.h
		gmdata/modelreader/gmmodelreader_obj.cpp
		gmdata/gamepackage/gmgamepackage.h
		gmdata/gamepackage/gmgamepackage.cpp
		gmdata/gamepackage/gmgamepackagehandler.h
		gmdata/gamepackage/gmgamepackagehandler.cpp
		gmdata/gmimage.h
		gmdata/gmimage.cpp
		gmdata/gmimagebuffer.h
		gmdata/gmimagebuffer.cpp
		gmdata/gmmodel.h
		gmdata/gmmodel.cpp
		gmdata/gmshader.h
		gmengine/gmassets.h
		gmengine/gmassets.cpp
		gmengine/gmgameworld.h
		gmengine/gmgameworld.cpp
		gmengine/gmcamera.h
		gmengine/gmcamera.cpp
		gmengine/gmdemogameworld.h
		gmengine/gmdemogameworld.cpp
		gmengine/gameobjects/gmgameobject.h
		gmengine/gameobjects/gmgameobject.cpp
		gmengine/gameobjects/gmparticles.h
		gmengine/gameobjects/gmparticles.cpp
		gmengine/gameobjects/gmspritegameobject.h
		gmengine/gameobjects/gmspritegameobject.cpp
		gmengine/gameobjects/gm2dgameobject.h
		gmengine/gameobjects/gm2dgameobject.cpp
		gmengine/gameobjects/gmcontrolgameobject.h
		gmengine/gameobjects/gmcontrolgameobject.cpp
		gmengine/gmtypoengine.h
		gmengine/gmtypoengine.cpp
		gmengine/gmanimation.h
		gmengine/gmanimation.cpp
		gmgl/gmglrenderers.h
		gmgl/gmglrenderers.cpp
		gmgl/gmglfactory.h
		gmgl/gmglfactory.cpp
		gmgl/gmglgbuffer.h
		gmgl/gmglgbuffer.cpp
		gmgl/gmglglyphmanager.h
		gmgl/gmglglyphmanager.cpp
		gmgl/gmglgraphic_engine.h
		gmgl/gmglgraphic_engine.cpp
		gmgl/gmglmodelpainter.h
		gmgl/gmglmodelpainter.cpp
		gmgl/gmglshaderprogram.h
		gmgl/gmglshaderprogram.cpp
		gmgl/gmgltexture.h
		gmgl/gmgltexture.cpp
		gmgl/shader_constants.h
		gmphysics/gmphysicsworld.h
		gmphysics/gmphysicsworld.cpp
		gmphysics/gmphysicsobject.h
		gmphysics/gmphysicsobject.cpp
		gmphysics/gmdiscretedynamicsworld.h
		gmphysics/gmdiscretedynamicsworld.cpp
		gmphysics/gmbullethelper.h
		gmphysics/gmbullethelper.cpp
		gmphysics/gmbulletincludes.h
		gmphysics/gmbulletforward.h
		foundation/utilities/tinyxml/tinystr.cpp
		foundation/utilities/tinyxml/tinystr.h
		foundation/utilities/tinyxml/tinyxml.cpp
		foundation/utilities/tinyxml/tinyxml.h
		foundation/utilities/tinyxml/tinyxmlerror.cpp
		foundation/utilities/tinyxml/tinyxmlparser.cpp
		foundation/utilities/tools.h
		foundation/utilities/tools.cpp
		foundation/utilities/utilities.h
		foundation/utilities/utilities.cpp
		foundation/utilities/gmcom.h
		foundation/input.h
		foundation/defines.h
		foundation/gmmessages.h
		foundation/debug.h
		foundation/debug.cpp
		foundation/assert.h
		foundation/assert.cpp
		foundation/vector.h
		foundation/memory.h
		foundation/memory.cpp
		foundation/linearmath.h
		foundation/event_enum.h
		foundation/gmobject.h
		foundation/gmobject.cpp
		foundation/gamemachine.h
		foundation/gamemachine.cpp
		foundation/interfaces.h
		foundation/gmstates.h
		foundation/gmstates.cpp
		foundation/gmthread.h
		foundation/gmprofile.h
		foundation/gmprofile.cpp
		foundation/gmstring.h
		foundation/gmstring.cpp
		foundation/gmenums.h
		gmlua/gmlua.h
		gmlua/gmlua.cpp
		gmlua/gmlua_functions.h
		gmlua/gmlua_functions.cpp

		extensions/bsp/bsp_factory.h
		extensions/bsp/bsp_factory.cpp
		extensions/bsp/data/gmbsp.h
		extensions/bsp/data/gmbsp.cpp
		extensions/bsp/data/gmbsp_render.h
		extensions/bsp/data/gmbsp_render.cpp
		extensions/bsp/data/gmbsp_shader_loader.h
		extensions/bsp/data/gmbsp_shader_loader.cpp
		extensions/bsp/data/gmbsp_surface_flags.h
		extensions/bsp/data/bsp_model_loader.h
		extensions/bsp/data/bsp_model_loader.cpp
		extensions/bsp/physics/gmbsptrace.h
		extensions/bsp/physics/gmbsptrace.cpp
		extensions/bsp/physics/gmbspphysicsworld.h
		extensions/bsp/physics/gmbspphysicsworld.cpp
		extensions/bsp/physics/gmbspphysicsstructs.h
		extensions/bsp/physics/gmbsppatch.h
		extensions/bsp/physics/gmbsppatch.cpp
		extensions/bsp/physics/gmbspmove.h
		extensions/bsp/physics/gmbspmove.cpp
		extensions/bsp/render/gmbspgameworldprivate.h
		extensions/bsp/render/gmbspgameworldprivate.cpp
		extensions/bsp/render/gmbspgameworld.h
		extensions/bsp/render/gmbspgameworld.cpp
	)

set(WIN32_SOURCES
		foundation/platforms/windows/check.h
		foundation/platforms/windows/event.cpp
		foundation/platforms/windows/path.cpp
		foundation/platforms/windows/timer.cpp
		foundation/platforms/windows/screen.cpp
		foundation/platforms/windows/thread.cpp
	)

if(WIN32)
	list(APPEND SOURCES ${WIN32_SOURCES})
endif(WIN32)

gm_source_group_by_dir(SOURCES)
gm_add_msvc_precompiled_header("stdafx.h" "stdafx.cpp" ${SOURCES})

add_library(${PROJECT_NAME}
		${SOURCES}
	)

if(MSVC)
	set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/W3" ) 
	set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/WX" ) 
endif(MSVC)

gm_link_dependencies(${PROJECT_NAME})
gm_link_bullet3()
target_link_libraries(${PROJECT_NAME} glew_s)
target_link_libraries(${PROJECT_NAME} lua)
target_link_libraries(${PROJECT_NAME} png_static)
target_link_libraries(${PROJECT_NAME} zlibstatic)
target_link_libraries(${PROJECT_NAME} libjpeg)
target_link_libraries(${PROJECT_NAME} freetype)