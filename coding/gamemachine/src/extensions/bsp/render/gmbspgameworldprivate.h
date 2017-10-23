#ifndef __BSPGAMEWORLDPRIVATE_H__
#define __BSPGAMEWORLDPRIVATE_H__
#include <gmcommon.h>
#include <shader.h>
#include <gmbsp.h>
#include <gmbspphysicsworld.h>
BEGIN_NS

// 每个作业的分片数
constexpr GMint DRAW_PIECE_COUNT = 2;

class GMBSPPhysicsWorld;
GM_PRIVATE_OBJECT(GMBSPGameWorld)
{
	GMBSPPhysicsWorld* physics = nullptr;
	GMBSP bsp;
	GMGameObject* sky = nullptr;

	GMBSPRender render;
	GMBSPShaderLoader shaderLoader;
	BSPModelLoader modelLoader;
	Map<GMint, Set<GMBSPEntity*> > entities;
	Vector<GMGameObject*> renderBuffer;
};

class GMBSPGameWorld;
struct BSPGameWorldEntityReader
{
	static void import(const GMBSPEntity& entity, GMBSPGameWorld* world);
};

END_NS
#endif