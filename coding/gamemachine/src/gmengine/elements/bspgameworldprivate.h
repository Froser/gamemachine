#ifndef __BSPGAMEWORLDPRIVATE_H__
#define __BSPGAMEWORLDPRIVATE_H__
#include "common.h"
#include "gmdatacore/bsp/bsp.h"
#include "gmdatacore/shader.h"
#include <map>
#include <string>
#include "gmengine/controllers/graphic_engine.h"
#include "gmdatacore/bsp/bsp_shader_loader.h"
#include "gmphysics/bspphysicsworld.h"
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

	std::map<BSP_Drawing_BiquadraticPatch*, GameObject*> biquadraticPatchObjects;
	std::map<BSP_Drawing_PolygonFace*, GameObject*> polygonFaceObjects;
	std::map<BSP_Drawing_MeshFace*, GameObject*> meshFaceObjects;

	// list to be drawn each frame
	DrawingList drawingList;

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