#ifndef __BSPGAMEWORLDPRIVATE_H__
#define __BSPGAMEWORLDPRIVATE_H__
#include <gmcommon.h>
#include <gmbsp.h>
#include <gmbspphysicsworld.h>
BEGIN_NS

enum class GMBSPRenderConfigs
{
	DrawSkyOnly_Bool,
	CalculateFace_Bool,
	Max,
};

namespace gm_config_guids
{
	const GMString BSPConfigGUID[(GMuint)GMBSPRenderConfigs::Max] = {
		L"EF47C8CA-3F7C-4cc9-927B-11624F0032DB",
		L"E31D6601-0066-4c35-920E-322BAC700916",
	};
}

GM_DEFINE_CONFIG(GMBSPRenderConfigs, gm_config_guids::BSPConfigGUID, GMBSPRenderConfig);

class GMBSPPhysicsWorld;
GM_PRIVATE_OBJECT(GMBSPGameWorld)
{
	GMBSP bsp;
	GMGameObject* sky = nullptr;
	GMBSPPhysicsWorld* physics = nullptr;
	GMBSPRender render;
	GMBSPShaderLoader shaderLoader;
	Map<GMint, Set<GMBSPEntity*> > entities;
	Vector<GMGameObject*> renderBuffer;
	GMDebugConfig debugConfig;
	GMConfig bspRenderConfig;
	GMBSPRenderConfig bspRenderConfigWrapper;
};

class GMBSPGameWorld;
struct BSPGameWorldEntityReader
{
	static void import(const GMBSPEntity& entity, GMBSPGameWorld* world);
};

END_NS
#endif