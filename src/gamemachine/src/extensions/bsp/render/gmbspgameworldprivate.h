#ifndef __BSPGAMEWORLDPRIVATE_H__
#define __BSPGAMEWORLDPRIVATE_H__
#include <gmcommon.h>
#include <extensions/bsp/gmbsp.h>
#include <extensions/bsp/gmbspphysicsworld.h>
BEGIN_NS

enum class GMBSPRenderConfigs
{
	DrawSkyOnly_Bool,
	CalculateFace_Bool,
	Max,
};

GM_DEFINE_CONFIG(GMBSPRenderConfigs, GMBSPRenderConfig);

class GMBSPPhysicsWorld;
GM_PRIVATE_OBJECT(GMBSPGameWorld)
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