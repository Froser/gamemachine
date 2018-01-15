#ifndef __BSPGAMEWORLDPRIVATE_H__
#define __BSPGAMEWORLDPRIVATE_H__
#include <gmcommon.h>
#include <gmbsp.h>
#include <gmbspphysicsworld.h>
BEGIN_NS

// 每个作业的分片数
constexpr GMint DRAW_PIECE_COUNT = 2;

class GMBSPPhysicsWorld;
GM_PRIVATE_OBJECT(GMBSPGameWorld)
{
	GMBSP bsp;
	GMGameObject* sky = nullptr;
	GMBSPPhysicsWorld* physics = nullptr;
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