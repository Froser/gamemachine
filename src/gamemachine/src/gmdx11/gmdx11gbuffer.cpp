#include "stdafx.h"
#include "gmdx11gbuffer.h"
#include "foundation/gamemachine.h"
#include "gmdx11graphic_engine.h"
#include "foundation/utilities/utilities.h"
#include "gmengine/gameobjects/gmgameobject.h"
#include "gmdata/gmmodel.h"
#include "gmdx11texture.h"

namespace
{
	constexpr const char* GeometryFramebufferNames[] = {
		"GM_DeferredPosition",
		"GM_DeferredNormal_World",
		"GM_DeferredNormal_Eye",
		"GM_DeferredTextureAmbient",
		"GM_DeferredTextureDiffuse",
		"GM_DeferredTangent_Eye",
		"GM_DeferredBitangent_Eye",
		"GM_DeferredNormalMap",
	};

	constexpr const char* MaterialFramebufferNames[] = {
		"GM_DeferredKs",
		"GM_DeferredShininess_bNormalMap_Refractivity",
	};
}

GMDx11GBuffer::GMDx11GBuffer(GMDx11GraphicEngine* engine)
{
	D(d);
	d->engine = engine;
}

void GMDx11GBuffer::geometryPass(GMGameObject *objects[], GMuint count)
{
	D(d);
	setGeometryPassingState(GMGeometryPassingState::PassingGeometry); // Dx11的Passes会在一个technique中完成，所以设置为非Done就可以了
	getGeometryFramebuffers()->clear();
	getMaterialFramebuffers()->clear();
	d->engine->draw(objects, count);
	setGeometryPassingState(GMGeometryPassingState::Done);
}

void GMDx11GBuffer::lightPass()
{
	D(d);
	GM_ASSERT(getQuad());
	getQuad()->draw();
}

void GMDx11GBuffer::useGeometryTextures(ID3DX11Effect* effect)
{
	D(d);
	IFramebuffers* geometryFramebuffers = getGeometryFramebuffers();
	GMint cnt = geometryFramebuffers->count();
	for (GMint i = 0; i < cnt; ++i)
	{
		GM_ASSERT(i < GM_array_size(GeometryFramebufferNames));
		GMDx11Texture* tex = gm_cast<GMDx11Texture*>(geometryFramebuffers->getFramebuffer(i)->getTexture());
		ID3DX11EffectShaderResourceVariable* shaderResource = effect->GetVariableByName(GeometryFramebufferNames[i])->AsShaderResource();
		GM_ASSERT(shaderResource->IsValid());
		GM_DX_HR(shaderResource->SetResource(tex->getResourceView()));
		if (i == 0)
			setSampler(effect, tex);
	}
}

void GMDx11GBuffer::useMaterialTextures(ID3DX11Effect* effect)
{
	D(d);
	IFramebuffers* materialFramebuffers = getMaterialFramebuffers();
	GMint cnt = materialFramebuffers->count();
	for (GMint i = 0; i < cnt; ++i)
	{
		GM_ASSERT(i < GM_array_size(MaterialFramebufferNames));
		GMDx11Texture* tex = gm_cast<GMDx11Texture*>(materialFramebuffers->getFramebuffer(i)->getTexture());
		ID3DX11EffectShaderResourceVariable* shaderResource = effect->GetVariableByName(MaterialFramebufferNames[i])->AsShaderResource();
		GM_ASSERT(shaderResource->IsValid());
		GM_DX_HR(shaderResource->SetResource(tex->getResourceView()));
		if (i == 0)
			setSampler(effect, tex);
	}
}

void GMDx11GBuffer::setSampler(ID3DX11Effect* effect, GMDx11Texture* texture)
{
	ID3DX11EffectSamplerVariable* sampler = effect->GetVariableByName("DeferredSampler")->AsSampler();
	GM_ASSERT(sampler->IsValid());
	GM_DX_HR(sampler->SetSampler(0, texture->getSamplerState()));
}

IFramebuffers* GMDx11GBuffer::createGeometryFramebuffers()
{
	IFramebuffers* framebuffers = nullptr;
	const GMGameMachineRunningStates& states = GM.getGameMachineRunningStates();
	GMFramebufferDesc desc = { 0 };
	desc.rect = states.renderRect;

	// Geometry Pass的纹理格式为R8G8B8A8_UNORM，意味着所有的输出在着色器中范围是[0,1]，对应着UNORM的[0x00, 0xFF]
	// 对于[-1, 1]范围的数据，需要在着色器中进行一次转换。
	desc.framebufferFormat = GMFramebufferFormat::R8G8B8A8_UNORM;

	GM.getFactory()->createFramebuffers(&framebuffers);
	framebuffers->init(desc);

	constexpr GMint framebufferCount = GM_array_size(GeometryFramebufferNames); //一共有8个SV_TARGET
	GM_STATIC_ASSERT(framebufferCount <= 8, "Too many targets.");
	for (GMint i = 0; i < framebufferCount; ++i)
	{
		IFramebuffer* framebuffer = nullptr;
		GM.getFactory()->createFramebuffer(&framebuffer);
		framebuffer->setName(GeometryFramebufferNames[i]);
		framebuffer->init(desc);
		framebuffers->addFramebuffer(framebuffer);
	}
	return framebuffers;
}

IFramebuffers* GMDx11GBuffer::createMaterialFramebuffers()
{
	IFramebuffers* framebuffers = nullptr;
	const GMGameMachineRunningStates& states = GM.getGameMachineRunningStates();
	GMFramebufferDesc desc = { 0 };
	desc.rect = states.renderRect;
	desc.framebufferFormat = GMFramebufferFormat::R32G32B32A32_FLOAT;

	GM.getFactory()->createFramebuffers(&framebuffers);
	framebuffers->init(desc);

	constexpr GMint framebufferCount = GM_array_size(MaterialFramebufferNames);
	GM_STATIC_ASSERT(framebufferCount <= 8, "Too many targets.");
	for (GMint i = 0; i < framebufferCount; ++i)
	{
		IFramebuffer* framebuffer = nullptr;
		GM.getFactory()->createFramebuffer(&framebuffer);
		framebuffer->setName(MaterialFramebufferNames[i]);
		framebuffer->init(desc);
		framebuffers->addFramebuffer(framebuffer);
	}
	return framebuffers;
}
