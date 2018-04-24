#include "stdafx.h"
#include "gmdx11gbuffer.h"
#include "foundation/gamemachine.h"
#include "gmdx11graphic_engine.h"
#include "foundation/utilities/utilities.h"
#include "gmengine/gameobjects/gmgameobject.h"
#include "gmdata/gmmodel.h"

GMDx11GBuffer::GMDx11GBuffer(GMDx11GraphicEngine* engine)
{
	D(d);
	d->engine = engine;
}

GMDx11GBuffer::~GMDx11GBuffer()
{
	D(d);
	GM_delete(d->geometryFramebuffer);
	GM_delete(d->materialFramebuffer);
	GM_delete(d->quad);
	GM_delete(d->quadModel);
}

void GMDx11GBuffer::init()
{
	D(d);
	constexpr const GMwchar* GeometryFramebufferNames[] = {
		L"GM_Position",
		L"GM_Normal_World",
		L"GM_Normal_Eye",
		L"GM_TextureAmbient",
		L"GM_TextureDiffuse",
		L"GM_Tangent_Eye",
		L"GM_Bitangent_Eye",
		L"GM_NormalMap",
	};

	constexpr const GMwchar* MaterialFramebufferNames[] = {
		L"GM_Ka",
		L"GM_Ks",
		L"GM_Kd",
		L"GM_Shininess_bNormalMap_Refractivity",
	};

	const GMGameMachineRunningStates& states = GM.getGameMachineRunningStates();
	GMFramebufferDesc desc = { 0 };
	desc.rect = states.renderRect;

	if (!d->geometryFramebuffer)
	{
		GM.getFactory()->createFramebuffers(&d->geometryFramebuffer);
		d->geometryFramebuffer->init(desc);

		constexpr GMint framebufferCount = GM_array_size(GeometryFramebufferNames); //一共有8个SV_TARGET
		GM_STATIC_ASSERT(framebufferCount <= 8, "Too many targets.");
		for (GMint i = 0; i < framebufferCount; ++i)
		{
			IFramebuffer* framebuffer = nullptr;
			GM.getFactory()->createFramebuffer(&framebuffer);
			framebuffer->setName(GeometryFramebufferNames[i]);
			framebuffer->init(desc);
			d->geometryFramebuffer->addFramebuffer(framebuffer);
		}
	}

	if (!d->materialFramebuffer)
	{
		GM.getFactory()->createFramebuffers(&d->materialFramebuffer);
		d->materialFramebuffer->init(desc);

		constexpr GMint framebufferCount = GM_array_size(MaterialFramebufferNames);
		GM_STATIC_ASSERT(framebufferCount <= 8, "Too many targets.");
		for (GMint i = 0; i < framebufferCount; ++i)
		{
			IFramebuffer* framebuffer = nullptr;
			GM.getFactory()->createFramebuffer(&framebuffer);
			framebuffer->setName(MaterialFramebufferNames[i]);
			framebuffer->init(desc);
			d->materialFramebuffer->addFramebuffer(framebuffer);
		}
	}
}

void GMDx11GBuffer::geometryPass(GMGameObject *objects[], GMuint count)
{
	D(d);
	d->engine->setIsDeferredRendering(true);
	d->geometryFramebuffer->clear();
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

IFramebuffers* GMDx11GBuffer::getGeometryFramebuffers()
{
	D(d);
	GM_ASSERT(d->geometryFramebuffer);
	return d->geometryFramebuffer;
}

IFramebuffers* GMDx11GBuffer::getMaterialFramebuffers()
{
	D(d);
	GM_ASSERT(d->materialFramebuffer);
	return d->materialFramebuffer;
}