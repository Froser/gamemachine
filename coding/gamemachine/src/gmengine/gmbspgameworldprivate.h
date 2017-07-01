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

struct DrawPiece;
struct DrawMeshFaceJob;
struct DrawPatchJob;
struct DrawEntityJob;

// 每个作业的分片数
CONST_EXPR GMint DRAW_PIECE_COUNT = 1;

GM_PRIVATE_OBJECT(GMBSPGameWorld)
{
	AutoPtr<GMBSPPhysicsWorld> physics;
	BSP bsp;
	GMGameObject* sky = nullptr;

	GMBSPRender render;
	GMBSPShaderLoader shaderLoader;
	BSPModelLoader modelLoader;
	std::map<GMint, std::set<GMBSPEntity*> > entities;

	Vector<GMGameObject*> polygonFaceBuffer;
	Vector<GMGameObject*> meshFaceBuffer;
	Vector<GMGameObject*> patchBuffer;
	Vector<GMGameObject*> entityBuffer;

	//DrawPolygonFaceJob* drawPolygonFaceJob;
	DrawPiece *drawPolygonFacePieces[DRAW_PIECE_COUNT];
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