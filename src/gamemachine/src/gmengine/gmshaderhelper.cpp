#include "stdafx.h"
#include "gmshaderhelper.h"
#include <gamemachine.h>

#if GM_USE_DX11
#include <gmdx11helper.h>
#endif

#include <gmglhelper.h>
#include <extensions/objects/gmwavegameobject.h>
#include <extensions/objects/particle/gmparticlemodel.h>
#include <extensions/objects/particle/gmparticleeffects.h>

namespace
{
	GMString getFileContent(const GMString& path)
	{
		GMBuffer buffer;
		GMString filename;
		GM.getGamePackageManager()->readFile(GMPackageIndex::Shaders, path, &buffer, &filename);
		buffer.convertToStringBuffer();
		return GMString((const char*)buffer.buffer);
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

		GMGameObject::setDefaultCullShaderCode(getFileContent(L"gl/compute/frustumcull.glsl"));
	}
	else
	{
#if GM_USE_DX11
		GMDx11Helper::loadShader(context, L"dx11/effect.fx");
		GMGameObject::setDefaultCullShaderCode(getFileContent(L"dx11/compute/frustumcull.hlsl"));
#else
		GM_ASSERT(false);
#endif
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
		GMGravityParticleEffect::setDefaultCodeAndEntry(getFileContent(L"gl/compute/particle_gravity.glsl"), L"main");
		GMRadialParticleEffect::setDefaultCodeAndEntry(getFileContent(L"gl/compute/particle_radial.glsl"), L"main");
		GMParticleModel::setDefaultCode(getFileContent(L"gl/compute/particle_transfer.glsl"));
	}
	else
	{
#if GM_USE_DX11
		GMGravityParticleEffect::setDefaultCodeAndEntry(getFileContent(L"dx11/compute/particle.hlsl"), L"gravity_main");
		GMRadialParticleEffect::setDefaultCodeAndEntry(getFileContent(L"dx11/compute/particle.hlsl"), L"radial_main");
		GMParticleModel::setDefaultCode(getFileContent(L"dx11/compute/particle_transfer.hlsl"));
#else
		GM_ASSERT(false);
#endif
	}
}
