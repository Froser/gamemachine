#ifndef __BSPGAMEWORLDPRIVATE_H__
#define __BSPGAMEWORLDPRIVATE_H__
#include "common.h"
#include "gmdatacore/bsp/bsp.h"
#include "gmdatacore/shader.h"
#include <map>
#include <string>
#include "gmengine/controllers/graphic_engine.h"
#include "gmdatacore/bsp/bsp_shader_loader.h"
BEGIN_NS

class GameObject;
class BSPGameWorld;
struct BSPGameWorldPrivate
{
	BSPGameWorldPrivate()
		: sky(nullptr)
		, ready(false)
	{
	}

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

struct BSPGameWorldEntityReader
{
	static void import(const BSPEntity& entity, BSPGameWorld* world);
};

END_NS
#endif