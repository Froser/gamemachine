#include "stdafx.h"
#include "gmgraphicengine.h"
#include "foundation/utilities/utilities.h"
#include "gmassets.h"
#include "foundation/gamemachine.h"
#include "gmengine/gameobjects/gmgameobject.h"
#include "foundation/gmprofile.h"
#include "foundation/gmconfigs.h"

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

GMGraphicEngine::GMGraphicEngine()
{
	D(d);
	d->renderConfig = GM.getConfigs().getConfig(GMConfigs::Render).asRenderConfig();
	d->debugConfig = GM.getConfigs().getConfig(GMConfigs::Debug).asDebugConfig();
}

GMGraphicEngine::~GMGraphicEngine()
{
	D(d);
	GM_delete(d->filterFramebuffers);
	GM_delete(d->filterQuad);
	GM_delete(d->gBuffer);
}

IGBuffer* GMGraphicEngine::getGBuffer()
{
	D(d);
	if (!d->gBuffer)
	{
		d->gBuffer = createGBuffer();
		d->gBuffer->init();
	}
	return d->gBuffer;
}

IFramebuffers* GMGraphicEngine::getFilterFramebuffers()
{
	D(d);
	return d->filterFramebuffers;
}

void GMGraphicEngine::draw(
	GMGameObject *forwardRenderingObjects[],
	GMuint forwardRenderingCount,
	GMGameObject *deferredRenderingObjects[],
	GMuint deferredRenderingCount)
{
	GM_PROFILE("draw");
	GMFilterMode::Mode filterMode = getCurrentFilterMode();
	if (filterMode != GMFilterMode::None)
	{
		createFilterFramebuffer();
		getFilterFramebuffers()->clear();
	}

	if (deferredRenderingObjects && deferredRenderingCount > 0)
	{
		IGBuffer* gBuffer = getGBuffer();
		gBuffer->geometryPass(deferredRenderingObjects, deferredRenderingCount);

		if (filterMode != GMFilterMode::None)
		{
			IFramebuffers* filterFramebuffers = getFilterFramebuffers();
			GM_ASSERT(filterFramebuffers);
			filterFramebuffers->bind();
			filterFramebuffers->clear();
		}
		gBuffer->lightPass();
		if (filterMode != GMFilterMode::None)
		{
			IFramebuffers* filterFramebuffers = getFilterFramebuffers();
			filterFramebuffers->unbind();
			getFilterQuad()->draw();
			gBuffer->getGeometryFramebuffers()->copyDepthStencilFramebuffer(filterFramebuffers);
		}
		gBuffer->getGeometryFramebuffers()->copyDepthStencilFramebuffer(getDefaultFramebuffers());
	}

	if (forwardRenderingObjects && forwardRenderingCount > 0)
	{
		IFramebuffers* filterFramebuffers = getFilterFramebuffers();
		if (filterMode != GMFilterMode::None)
		{
			filterFramebuffers->clear();
			filterFramebuffers->bind();
		}

		draw(forwardRenderingObjects, forwardRenderingCount);

		if (filterMode != GMFilterMode::None)
		{
			filterFramebuffers->unbind();
			getFilterQuad()->draw();
		}
	}
}

void GMGraphicEngine::draw(GMGameObject *objects[], GMuint count)
{
	D(d);
	for (GMuint i = 0; i < count; i++)
	{
		objects[i]->draw();
	}
}

const GMFilterMode::Mode GMGraphicEngine::getCurrentFilterMode()
{
	D(d);
	return d->renderConfig.get(GMRenderConfigs::FilterMode).toEnum<GMFilterMode::Mode>();
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
		GMFramebuffersDesc fbDesc = { 0 };
		fbDesc.rect = states.renderRect;
		d->filterFramebuffers->init(fbDesc);
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

IGBuffer* GMGraphicEngine::createGBuffer()
{
	D(d);
	IGBuffer* gBuffer = nullptr;
	GM.getFactory()->createGBuffer(this, &gBuffer);
	GM_ASSERT(gBuffer);
	return gBuffer;
}

void GMGraphicEngine::addLight(const GMLight& light)
{
	D(d);
	d->lights.push_back(light);
}

void GMGraphicEngine::removeLights()
{
	D(d);
	d->lights.clear();
}


void GMGraphicEngine::setStencilOptions(const GMStencilOptions& options)
{
	D(d);
	d->stencilOptions = options;
}

const GMStencilOptions& GMGraphicEngine::getStencilOptions()
{
	D(d);
	return d->stencilOptions;
}

void GMGraphicEngine::setShaderLoadCallback(IShaderLoadCallback* cb)
{
	D(d);
	d->shaderLoadCallback = cb;
}