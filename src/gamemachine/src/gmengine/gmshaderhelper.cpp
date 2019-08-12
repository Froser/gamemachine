#include "stdafx.h"
#include "gmshaderhelper.h"
#include <gamemachine.h>
#include <gmglhelper.h>
#include <extensions/objects/gmwavegameobject.h>
#include <gmengine/particle/cocos2d/gmparticlemodel_cocos2d.h>
#include <gmengine/particle/cocos2d/gmparticleeffects_cocos2d.h>
#include "wrapper/dx11wrapper.h"

BEGIN_NS

namespace
{
	GMString getFileContent(const GMString& path)
	{
		GMBuffer buffer;
		GMString filename;
		GM.getGamePackageManager()->readFile(GMPackageIndex::Shaders, path, &buffer, &filename);
		buffer.convertToStringBuffer();
		return GMString((const char*)buffer.getData());
	}
}

void GMShaderHelper::loadShader(const IRenderContext* context)
{
	auto& env = GM.getRunningStates().renderEnvironment;
	if (env == GMRenderEnvironment::OpenGL)
	{
        GMGLHelper::loadShader(
            context,
            L"gl/main.vert",
            L"gl/main.frag",
            L"gl/deferred/geometry_pass_main.vert",
            L"gl/deferred/geometry_pass_main.frag",
            L"gl/deferred/light_pass_main.vert",
            L"gl/deferred/light_pass_main.frag",
            L"gl/filters/filters.vert",
            L"gl/filters/filters.frag"
        );

        if (GMQueryCapability(GMCapability::SupportCalculateShader))
		{
			GMGameObject::setDefaultCullShaderCode(getFileContent(L"gl/compute/frustumcull.glsl"));
		}
	}
	else
	{
		DirectX11LoadShader(context, L"dx11/effect.fx");
		if (GMQueryCapability(GMCapability::SupportCalculateShader))
		{
			GMGameObject::setDefaultCullShaderCode(getFileContent(L"dx11/compute/frustumcull.hlsl"));
		}
	}
}

void GMShaderHelper::loadExtensionShaders(const IRenderContext* context)
{
return;
	// Wave
	GMWaveGameObject::initShader(context);

	// Particles
	auto& env = GM.getRunningStates().renderEnvironment;
	if (env == GMRenderEnvironment::OpenGL)
	{
		if (GMQueryCapability(GMCapability::SupportCalculateShader))
		{
			GMGravityParticleEffect_Cocos2D::setDefaultCodeAndEntry(getFileContent(L"gl/compute/particle_gravity_cocos2d.glsl"), L"main");
			GMRadialParticleEffect_Cocos2D::setDefaultCodeAndEntry(getFileContent(L"gl/compute/particle_radial_cocos2d.glsl"), L"main");
			GMParticleModel_Cocos2D::setDefaultCode(getFileContent(L"gl/compute/particle_transfer_cocos2d.glsl"));
		}
	}
	else
	{
		DirectX11LoadExtensionShaders(context);
	}
}

END_NS
