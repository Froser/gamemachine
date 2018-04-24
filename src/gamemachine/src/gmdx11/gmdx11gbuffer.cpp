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

GMDx11GBuffer::~GMDx11GBuffer()
{
	D(d);
	GM_delete(d->geometryFramebuffers);
	GM_delete(d->materialFramebuffers);
	GM_delete(d->quad);
	GM_delete(d->quadModel);
}

void GMDx11GBuffer::init()
{
	D(d);
	const GMGameMachineRunningStates& states = GM.getGameMachineRunningStates();
	GMFramebufferDesc desc = { 0 };
	desc.rect = states.renderRect;

	if (!d->geometryFramebuffers)
	{
		GM.getFactory()->createFramebuffers(&d->geometryFramebuffers);
		d->geometryFramebuffers->init(desc);

		constexpr GMint framebufferCount = GM_array_size(GeometryFramebufferNames); //一共有8个SV_TARGET
		GM_STATIC_ASSERT(framebufferCount <= 8, "Too many targets.");
		for (GMint i = 0; i < framebufferCount; ++i)
		{
			IFramebuffer* framebuffer = nullptr;
			GM.getFactory()->createFramebuffer(&framebuffer);
			framebuffer->setName(GeometryFramebufferNames[i]);
			framebuffer->init(desc);
			d->geometryFramebuffers->addFramebuffer(framebuffer);
		}
	}

	if (!d->materialFramebuffers)
	{
		GM.getFactory()->createFramebuffers(&d->materialFramebuffers);
		d->materialFramebuffers->init(desc);

		constexpr GMint framebufferCount = GM_array_size(MaterialFramebufferNames);
		GM_STATIC_ASSERT(framebufferCount <= 8, "Too many targets.");
		for (GMint i = 0; i < framebufferCount; ++i)
		{
			IFramebuffer* framebuffer = nullptr;
			GM.getFactory()->createFramebuffer(&framebuffer);
			framebuffer->setName(MaterialFramebufferNames[i]);
			framebuffer->init(desc);
			d->materialFramebuffers->addFramebuffer(framebuffer);
		}
	}
}

void GMDx11GBuffer::geometryPass(GMGameObject *objects[], GMuint count)
{
	D(d);
	d->engine->setIsDeferredRendering(true);
	d->geometryFramebuffers->clear();
	d->materialFramebuffers->clear();
	d->engine->draw(objects, count);
	d->engine->setIsDeferredRendering(false);
}

void GMDx11GBuffer::lightPass()
{
	D(d);
	if (!d->quad)
	{
		GM_ASSERT(!d->quadModel);
		GMPrimitiveCreator::createQuadrangle(GMPrimitiveCreator::one2(), 0, &d->quadModel);
		GM_ASSERT(d->quadModel);
		d->quadModel->setType(GMModelType::LightPassQuad);
		GM.createModelPainterAndTransfer(d->quadModel);
		d->quad = new GMGameObject(GMAssets::createIsolatedAsset(GMAssetType::Model, d->quadModel));
	}
	d->quad->draw();
}

void GMDx11GBuffer::useGeometryTextures(ID3DX11Effect* effect)
{
	D(d);
	GMint cnt = d->geometryFramebuffers->count();
	for (GMint i = 0; i < cnt; ++i)
	{
		GM_ASSERT(i < GM_array_size(GeometryFramebufferNames));
		GMDx11Texture* tex = gm_cast<GMDx11Texture*>(d->geometryFramebuffers->getFramebuffer(i)->getTexture());
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
	GMint cnt = d->materialFramebuffers->count();
	for (GMint i = 0; i < cnt; ++i)
	{
		GM_ASSERT(i < GM_array_size(MaterialFramebufferNames));
		GMDx11Texture* tex = gm_cast<GMDx11Texture*>(d->materialFramebuffers->getFramebuffer(i)->getTexture());
		ID3DX11EffectShaderResourceVariable* shaderResource = effect->GetVariableByName(MaterialFramebufferNames[i])->AsShaderResource();
		GM_ASSERT(shaderResource->IsValid());
		GM_DX_HR(shaderResource->SetResource(tex->getResourceView()));
		if (i == 0)
			setSampler(effect, tex);
	}
}

IFramebuffers* GMDx11GBuffer::getGeometryFramebuffers()
{
	D(d);
	GM_ASSERT(d->geometryFramebuffers);
	return d->geometryFramebuffers;
}

IFramebuffers* GMDx11GBuffer::getMaterialFramebuffers()
{
	D(d);
	GM_ASSERT(d->materialFramebuffers);
	return d->materialFramebuffers;
}

void GMDx11GBuffer::setSampler(ID3DX11Effect* effect, GMDx11Texture* texture)
{
	ID3DX11EffectSamplerVariable* sampler = effect->GetVariableByName("DeferredSampler")->AsSampler();
	GM_ASSERT(sampler->IsValid());
	GM_DX_HR(sampler->SetSampler(0, texture->getSamplerState()));
}