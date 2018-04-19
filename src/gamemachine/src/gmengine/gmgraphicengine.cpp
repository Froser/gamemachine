#include "stdafx.h"
#include "gmgraphicengine.h"
#include "foundation/utilities/utilities.h"
#include "gmassets.h"
#include "foundation/gamemachine.h"
#include "gmengine/gameobjects/gmgameobject.h"

GMGraphicEngine::~GMGraphicEngine()
{
	D(d);
	GM_delete(d->filterFramebuffers);
	GM_delete(d->filterQuad);
}

void GMGraphicEngine::createFilterFramebuffer()
{
	D(d);
	if (!d->filterFramebuffers)
	{
		IFactory* factory = GM.getFactory();
		const GMGameMachineRunningStates& states = GM.getGameMachineRunningStates();
		GMFramebufferDesc desc = { 0 };
		desc.rect = states.windowRect;
		factory->createFramebuffers(&d->filterFramebuffers);
		GM_ASSERT(d->filterFramebuffers);
		d->filterFramebuffers->init(desc);
		IFramebuffer* framebuffer = nullptr;
		factory->createFramebuffer(&framebuffer);
		GM_ASSERT(framebuffer);
		framebuffer->init(desc);
		d->filterFramebuffers->addFramebuffer(framebuffer);
	}

	if (!d->filterQuad)
	{
		GMModel* quad = nullptr;
		GMPrimitiveCreator::createQuadrangle(GMPrimitiveCreator::one2(), 0, &quad);
		GM_ASSERT(quad);
		quad->setType(GMModelType::Filter);
		quad->getShader().getTexture().getTextureFrames(GMTextureType::Ambient, 0).addFrame(d->filterFramebuffers->getTexture(0));
		d->filterQuadModel.reset(quad);
		GM.createModelPainterAndTransfer(quad);
		GMAsset asset = GMAssets::createIsolatedAsset(GMAssetType::Model, quad);
		d->filterQuad = new GMGameObject(asset);
	}
}
