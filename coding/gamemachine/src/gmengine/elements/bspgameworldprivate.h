#ifndef __BSPGAMEWORLDPRIVATE_H__
#define __BSPGAMEWORLDPRIVATE_H__
#include "common.h"
#include "gmdatacore/bsp/bsp.h"
#include "gmdatacore/shader.h"
#include <string>
#include <map>
#include <set>
#include "gmengine/controllers/graphic_engine.h"
#include "gmdatacore/bsp/bsp_shader_loader.h"
#include "gmphysics/bsp/bspphysicsworld.h"
#include "gmdatacore/bsp/bsp_render.h"
#include "gmdatacore/bsp_model_loader.h"
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
	GameObject* sky;

	// list to be drawn each frame
	DrawingList drawingList;

	BSPRender render;
	BSPShaderLoader shaderLoader;
	BSPModelLoader modelLoader;
	std::map<GMint, std::set<BSPEntity*> > entities;
	bool ready;
};

class BSPGameWorld;
struct BSPGameWorldEntityReader
{
	static void import(const BSPEntity& entity, BSPGameWorld* world);
};

END_NS
#endif