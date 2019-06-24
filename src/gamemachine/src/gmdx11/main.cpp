#include "stdafx.h"
#include "main.h"
#include "gmdx11/gmdx11factory.h"
#include "gmdx11helper.h"
#include "gmengine/gameobjects/gmgameobject.h"
#include "foundation/gamemachine.h"
#include "extensions/objects/particle/cocos2d/gmparticleeffects_cocos2d.h"
#include "extensions/objects/particle/cocos2d/gmparticlemodel_cocos2d.h"

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

IFactory* gmdx11_createDirectX11Factory()
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
	GMGravityParticleEffect_Cocos2D::setDefaultCodeAndEntry(getFileContent(L"dx11/compute/particle_cocos2d.hlsl"), L"gravity_main");
	GMRadialParticleEffect_Cocos2D::setDefaultCodeAndEntry(getFileContent(L"dx11/compute/particle_cocos2d.hlsl"), L"radial_main");
	GMParticleModel_Cocos2D::setDefaultCode(getFileContent(L"dx11/compute/particle_transfer_cocos2d.hlsl"));
}

void gmdx11_ext_renderWaveObjectShader(const GMWaveGameObject* waveObject, IShaderProgram* shaderProgram)
{
	auto d = waveObject->data();
	enum
	{
		WAVE_COUNT_ID,
		WAVE_DURATION_ID,
		LAST_ID,
	};
	static std::once_flag s_flag;
	std::call_once(s_flag, [d, shaderProgram]() {
		d->globalIndices.resize(1);
		d->globalIndices[0].waveCount = shaderProgram->getIndex(WAVE_COUNT);
		d->globalIndices[0].duration = shaderProgram->getIndex(WAVE_DURATION);
	});
	GMint32 waveCount = gm_sizet_to_int(d->waveDescriptions.size());
	shaderProgram->setInt(d->globalIndices[0].waveCount, waveCount);
	shaderProgram->setFloat(d->globalIndices[0].duration, d->duration);

	GMComPtr<ID3DX11Effect> effect;
	shaderProgram->getInterface(GameMachineInterfaceID::D3D11Effect, (void**)&effect);
	for (GMint32 i = 0; i < waveCount; ++i)
	{
		GM_ASSERT(effect);
		auto descriptions = effect->GetVariableByName(WAVE_DESCRIPTION);
		GM_ASSERT(descriptions->IsValid());
		auto description = descriptions->GetElement(i);
		GM_ASSERT(description->IsValid());
		GM_DX_HR(description->GetMemberByName(STEEPNESS)->AsScalar()->SetFloat(d->waveDescriptions[i].steepness));
		GM_DX_HR(description->GetMemberByName(AMPLITUDE)->AsScalar()->SetFloat(d->waveDescriptions[i].amplitude));
		GM_DX_HR(description->GetMemberByName(DIRECTION)->AsVector()->SetFloatVector(d->waveDescriptions[i].direction));
		GM_DX_HR(description->GetMemberByName(SPEED)->AsScalar()->SetFloat(d->waveDescriptions[i].speed));
		GM_DX_HR(description->GetMemberByName(WAVELENGTH)->AsScalar()->SetFloat(d->waveDescriptions[i].waveLength));
	}
}
