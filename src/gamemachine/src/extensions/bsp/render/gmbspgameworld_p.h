#ifndef __BSPGAMEWORLD_P_H__
#define __BSPGAMEWORLD_P_H__
#include <gmcommon.h>
#include <extensions/bsp/gmbsp.h>
#include <extensions/bsp/gmbspphysicsworld.h>
BEGIN_NS

class GMBSPPhysicsWorld;
GM_PRIVATE_OBJECT_ALIGNED(GMBSPGameWorld)
{
	GMBSP bsp;
	GMGameObject* sky = nullptr;
	GMBSPPhysicsWorld* physics = nullptr;
	GMBSPRender render;
	GMBSPShaderLoader shaderLoader;
	Map<GMint32, Set<GMBSPEntity*> > entities;
	GMDebugConfig debugConfig;
	GMConfig bspRenderConfig;
	GMBSPRenderConfig bspRenderConfigWrapper;
	GMSpriteGameObject* sprite = nullptr;
};

class GMBSPGameWorld;
struct BSPGameWorldEntityReader
{
	static void import(const GMBSPEntity& entity, GMBSPGameWorld* world);
};

END_NS
#endif