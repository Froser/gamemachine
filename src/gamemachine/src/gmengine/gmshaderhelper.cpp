#include "stdafx.h"
#include "gmshaderhelper.h"
#include <gamemachine.h>

#if GM_USE_DX11
#include <gmdx11helper.h>
#endif

#include <gmglhelper.h>
#include <extensions/objects/gmwavegameobject.h>
#include <extensions/objects/particle/gmparticlemodel.h>

namespace
{
	IComputeShaderProgram* createComputeShader(
		const IRenderContext* context,
		const GMString& path,
		const GMString& entryPoint
	)
	{
		IComputeShaderProgram* shaderProgram = nullptr;
		GMBuffer buffer;
		GMString filename;
		GM.getGamePackageManager()->readFile(GMPackageIndex::Shaders, path, &buffer, &filename);
		buffer.convertToStringBuffer();

		if (GM.getFactory()->createComputeShaderProgram(context, &shaderProgram))
		{
			shaderProgram->load(filename, GMString((char*)buffer.buffer), entryPoint);
		}
		return shaderProgram;
	}
}

void GMShaderHelper::loadShader(const IRenderContext* context, ShaderHelperResult* result)
{
	GM_ASSERT(result);
	IComputeShaderProgram* cullShaderProgram = nullptr;
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

		cullShaderProgram = createComputeShader(context,
			L"gl/compute/frustumcull.glsl",
			L"main"
		);

		if (cullShaderProgram)
			GMGameObject::setDefaultCullShaderProgram(cullShaderProgram);
	}
	else
	{
#if GM_USE_DX11
		GMDx11Helper::loadShader(context, L"dx11/effect.fx");

		cullShaderProgram = createComputeShader(context,
			L"dx11/frustumcull.hlsl",
			L"Main"
		);

		if (cullShaderProgram)
			GMGameObject::setDefaultCullShaderProgram(cullShaderProgram);
#else
		GM_ASSERT(false);
#endif
	}

	if (result)
		result->cullShaderProgram = cullShaderProgram;
}

void GMShaderHelper::loadExtensionShaders(const IRenderContext* context)
{
	GMWaveGameObject::initShader(context);
}
