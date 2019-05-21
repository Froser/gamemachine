#include "stdafx.h"
#include "main.h"
#include "gmdx11/gmdx11factory.h"
#include "gmdx11helper.h"
#include "gmengine/gameobjects/gmgameobject.h"
#include "foundation/gamemachine.h"
#include "extensions/objects/particle/gmparticleeffects.h"
#include "extensions/objects/particle/gmparticlemodel.h"

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

IFactory* gmdx11_CreateDirectX11Factory()
{
	return new GMDx11Factory();
}

void gmdx11_loadShader(const IRenderContext* context, const GMString& fileName)
{
	GMDx11Helper::loadShader(context, fileName, L"main.gfx");
	GMGameObject::setDefaultCullShaderCode(getFileContent(L"dx11/compute/frustumcull.hlsl"));
}

void gmdx11_loadExtensionShaders(const IRenderContext*)
{
	GMGravityParticleEffect::setDefaultCodeAndEntry(getFileContent(L"dx11/compute/particle.hlsl"), L"gravity_main");
	GMRadialParticleEffect::setDefaultCodeAndEntry(getFileContent(L"dx11/compute/particle.hlsl"), L"radial_main");
	GMParticleModel::setDefaultCode(getFileContent(L"dx11/compute/particle_transfer.hlsl"));
}