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
		GMBuffer buffer;
		GM.getGamePackageManager()->readFile(GMPackageIndex::Shaders, "gl/manifest.xml", &buffer, nullptr);
		buffer.convertToStringBuffer();
		GMGLHelper::loadShader(context, GMString((const char*)buffer.getData()));

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
