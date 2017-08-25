#ifndef __BSPGAMEWORLDPRIVATE_H__
#define __BSPGAMEWORLDPRIVATE_H__
#include "common.h"
#include "gmdatacore/shader.h"
#include <string>
#include <map>
#include <set>
#include "gmdatacore/bsp/gmbsp.h"
#include "gmdatacore/bsp/gmbsp_shader_loader.h"
#include "gmdatacore/bsp/gmbsp_render.h"
#include "gmphysics/bsp/gmbspphysicsworld.h"
#include "gmdatacore/bsp_model_loader.h"
BEGIN_NS

#if 0
struct DrawPiece;
#endif;
struct DrawPolygonFaceJob;
struct DrawMeshFaceJob;
struct DrawPatchJob;
struct DrawEntityJob;

// 每个作业的分片数
constexpr GMint DRAW_PIECE_COUNT = 2;

GM_PRIVATE_OBJECT(GMBSPGameWorld)
{
	AutoPtr<GMBSPPhysicsWorld> physics;
	GMBSP bsp;
	GMGameObject* sky = nullptr;

	GMBSPRender render;
	GMBSPShaderLoader shaderLoader;
	BSPModelLoader modelLoader;
	Map<GMint, Set<GMBSPEntity*> > entities;
	Map<GMString, GMModelContainerItemIndex> entitiesCache;
	Vector<GMGameObject*> renderBuffer;

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