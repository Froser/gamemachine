#ifndef __BSPGAMEWORLDPRIVATE_H__
#define __BSPGAMEWORLDPRIVATE_H__
#include "common.h"
#include "gmdatacore/bsp/bsp.h"
#include "gmdatacore/shader.h"
#include <string>
#include "gmengine/controllers/graphic_engine.h"
#include "gmdatacore/bsp/bsp_shader_loader.h"
#include "gmphysics/bspphysicsworld.h"
#include "gmdatacore/bsp/bsp_render.h"
BEGIN_NS

class GameObject;
struct BSPGameWorldPrivate
{
	BSPGameWorldPrivate()
		: sky(nullptr)
		, ready(false)
	{
	}

	AutoPtr<BSPPhysicsWorld> physics;
	BSP bsp;
	std::string bspWorkingDirectory;
	GameObject* sky;

	// list to be drawn each frame
	DrawingList drawingList;

	BSPRender render;
	BSPShaderLoader shaderLoader;
	bool ready;
};

class BSPGameWorld;
struct BSPGameWorldEntityReader
{
	static void import(const BSPEntity& entity, BSPGameWorld* world);
};

END_NS
#endif