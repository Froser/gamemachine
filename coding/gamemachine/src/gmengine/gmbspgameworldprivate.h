#ifndef __BSPGAMEWORLDPRIVATE_H__
#define __BSPGAMEWORLDPRIVATE_H__
#include "common.h"
#include "gmdatacore/shader.h"
#include <string>
#include <map>
#include <set>
#include "gmengine/controllers/graphic_engine.h"
#include "gmdatacore/bsp/gmbsp.h"
#include "gmdatacore/bsp/gmbsp_shader_loader.h"
#include "gmdatacore/bsp/gmbsp_render.h"
#include "gmphysics/bsp/gmbspphysicsworld.h"
#include "gmdatacore/bsp_model_loader.h"
BEGIN_NS

struct DrawPolygonFaceJob;
struct DrawMeshFaceJob;
struct DrawPatchJob;
struct DrawEntityJob;

GM_PRIVATE_OBJECT(GMBSPGameWorld)
{
	GM_PRIVATE_CONSTRUCT(GMBSPGameWorld)
		: sky(nullptr)
	{
	}

	AutoPtr<GMBSPPhysicsWorld> physics;
	BSP bsp;
	GMGameObject* sky;

	GMBSPRender render;
	GMBSPShaderLoader shaderLoader;
	BSPModelLoader modelLoader;
	std::map<GMint, std::set<GMBSPEntity*> > entities;

	Vector<GMGameObject*> polygonFaceBuffer;
	Vector<GMGameObject*> meshFaceBuffer;
	Vector<GMGameObject*> patchBuffer;
	Vector<GMGameObject*> entityBuffer;

	DrawPolygonFaceJob* drawPolygonFaceJob;
	DrawMeshFaceJob* drawMeshFaceJob;
	DrawPatchJob* drawPatchJob;
	DrawEntityJob* drawEntityJob;
};

class GMBSPGameWorld;
struct BSPGameWorldEntityReader
{
	static void import(const GMBSPEntity& entity, GMBSPGameWorld* world);
};

END_NS
#endif