#include "stdafx.h"
#include "main.h"
#include "gmdx11/gmdx11factory.h"
#include "gmdx11helper.h"
#include "gmengine/gameobjects/gmgameobject.h"
#include "foundation/gamemachine.h"
#include "gmengine/particle/cocos2d/gmparticleeffects_cocos2d.h"
#include "gmengine/particle/cocos2d/gmparticlemodel_cocos2d.h"
#include "extensions/objects/gmwavegameobject.h"
#include "extensions/objects/gmwavegameobject_p.h"
#include "gmdx11techniques.h"

namespace
{
	gm::GMString getFileContent(const gm::GMString& path)
	{
		gm::GMBuffer buffer;
		gm::GMString filename;
		GM.getGamePackageManager()->readFile(gm::GMPackageIndex::Shaders, path, &buffer, &filename);
		buffer.convertToStringBuffer();
		return gm::GMString((const char*)buffer.getData());
	}
}

gm::IFactory* gmdx11_createDirectX11Factory()
{
	return new gm::GMDx11Factory();
}

void gmdx11_loadShader(const gm::IRenderContext* context, const gm::GMString& fileName)
{
	gm::GMDx11Helper::loadShader(context, fileName, L"main.gfx");
	gm::GMGameObject::setDefaultCullShaderCode(getFileContent(L"dx11/compute/frustumcull.hlsl"));
}

void gmdx11_loadExtensionShaders(const gm::IRenderContext*)
{
	gm::GMGravityParticleEffect_Cocos2D::setDefaultCodeAndEntry(getFileContent(L"dx11/compute/particle_cocos2d.hlsl"), L"gravity_main");
	gm::GMRadialParticleEffect_Cocos2D::setDefaultCodeAndEntry(getFileContent(L"dx11/compute/particle_cocos2d.hlsl"), L"radial_main");
	gm::GMParticleModel_Cocos2D::setDefaultCode(getFileContent(L"dx11/compute/particle_transfer_cocos2d.hlsl"));
}

void gmdx11_ext_renderWaveObjectShader(gm::GMWaveGameObject* waveObject, gm::IShaderProgram* shaderProgram)
{
	auto& d = waveObject->dataRef();
	enum
	{
		WAVE_COUNT_ID,
		WAVE_DURATION_ID,
		LAST_ID,
	};
	static std::once_flag s_flag;
	std::call_once(s_flag, [&d, shaderProgram]() {
		d.globalIndices.resize(1);
		d.globalIndices[0].waveCount = shaderProgram->getIndex(WAVE_COUNT);
		d.globalIndices[0].duration = shaderProgram->getIndex(WAVE_DURATION);
	});
	gm::GMint32 waveCount = gm::gm_sizet_to_int(d.waveDescriptions.size());
	shaderProgram->setInt(d.globalIndices[0].waveCount, waveCount);
	shaderProgram->setFloat(d.globalIndices[0].duration, d.duration);

	gm::GMComPtr<ID3DX11Effect> effect;
	shaderProgram->getInterface(gm::GameMachineInterfaceID::D3D11Effect, (void**)&effect);
	for (gm::GMint32 i = 0; i < waveCount; ++i)
	{
		GM_ASSERT(effect);
		auto descriptions = effect->GetVariableByName(WAVE_DESCRIPTION);
		GM_ASSERT(descriptions->IsValid());
		auto description = descriptions->GetElement(i);
		GM_ASSERT(description->IsValid());
		GM_DX_HR(description->GetMemberByName(STEEPNESS)->AsScalar()->SetFloat(d.waveDescriptions[i].steepness));
		GM_DX_HR(description->GetMemberByName(AMPLITUDE)->AsScalar()->SetFloat(d.waveDescriptions[i].amplitude));
		GM_DX_HR(description->GetMemberByName(DIRECTION)->AsVector()->SetFloatVector(d.waveDescriptions[i].direction));
		GM_DX_HR(description->GetMemberByName(SPEED)->AsScalar()->SetFloat(d.waveDescriptions[i].speed));
		GM_DX_HR(description->GetMemberByName(WAVELENGTH)->AsScalar()->SetFloat(d.waveDescriptions[i].waveLength));
	}
}

ID3DX11Effect* gmdx11_getEffectFromTechnique(gm::ITechnique* technique)
{
	gm::GMDx11Technique* dxTechnique = gm::gm_cast<gm::GMDx11Technique*>(technique);
	ID3DX11Effect* effect = dxTechnique->getEffect();
	return effect;
}
