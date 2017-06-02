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
#include "gmphysics/bsp/gmbspphysicsworld.h"
#include "gmdatacore/bsp/bsp_render.h"
#include "gmdatacore/bsp_model_loader.h"
BEGIN_NS

GM_PRIVATE_OBJECT(GMBSPGameWorld)
{
	GM_PRIVATE_CONSTRUCT(GMBSPGameWorld)
		: sky(nullptr)
	{
	}

	AutoPtr<GMBSPPhysicsWorld> physics;
	BSP bsp;
	GMGameObject* sky;

	BSPRender render;
	GMBSPShaderLoader shaderLoader;
	BSPModelLoader modelLoader;
	std::map<GMint, std::set<BSPEntity*> > entities;
};

class GMBSPGameWorld;
struct BSPGameWorldEntityReader
{
	static void import(const BSPEntity& entity, GMBSPGameWorld* world);
};

END_NS
#endif