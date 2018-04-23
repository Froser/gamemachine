#include "stdafx.h"
#include "gmgraphicengine.h"
#include "foundation/utilities/utilities.h"
#include "gmassets.h"
#include "foundation/gamemachine.h"
#include "gmengine/gameobjects/gmgameobject.h"

void GMFramebuffersStack::push(IFramebuffers* framebuffers)
{
	D(d);
	d->framebuffers.push(framebuffers);
}

IFramebuffers* GMFramebuffersStack::pop()
{
	D(d);
	if (d->framebuffers.empty())
		return nullptr;

	IFramebuffers* framebuffers = d->framebuffers.top();
	d->framebuffers.pop();
	return framebuffers;
}

IFramebuffers* GMFramebuffersStack::peek()
{
	D(d);
	if (d->framebuffers.empty())
		return nullptr;
	return d->framebuffers.top();
}

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
		desc.rect = states.renderRect;
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
		quad->getShader().getTexture().getTextureFrames(GMTextureType::Ambient, 0).addFrame(d->filterFramebuffers->getFramebuffer(0)->getTexture());
		d->filterQuadModel.reset(quad);
		GM.createModelPainterAndTransfer(quad);
		GMAsset asset = GMAssets::createIsolatedAsset(GMAssetType::Model, quad);
		d->filterQuad = new GMGameObject(asset);
	}
}
