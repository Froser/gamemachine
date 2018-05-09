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
	d->shadow.type = GMShadowSourceDesc::NoShadow;
}

GMGraphicEngine::~GMGraphicEngine()
{
	D(d);
	GM_delete(d->filterFramebuffers);
	GM_delete(d->filterQuad);
	GM_delete(d->gBuffer);
	GM_delete(d->shadowDepthFramebuffers);
}

void GMGraphicEngine::init()
{
	getDefaultFramebuffers()->bind();
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

void GMGraphicEngine::draw(const List<GMGameObject*>& forwardRenderingObjects, const List<GMGameObject*>& deferredRenderingObjects)
{
	GM_PROFILE("draw");
	D(d);
	if (d->shadow.type != GMShadowSourceDesc::NoShadow)
	{
		if (!d->shadowDepthFramebuffers)
			createShadowFramebuffers(&d->shadowDepthFramebuffers);
		GM_ASSERT(d->shadowDepthFramebuffers);
		d->shadowDepthFramebuffers->clear(GMFramebuffersClearType::Depth);
		d->shadowDepthFramebuffers->bind();
		d->isDrawingShadow = true;
		draw(forwardRenderingObjects);
		draw(deferredRenderingObjects);
		d->shadowDepthFramebuffers->unbind();
		d->isDrawingShadow = false;
	}

	GMFilterMode::Mode filterMode = getCurrentFilterMode();
	if (filterMode != GMFilterMode::None)
	{
		createFilterFramebuffer();
		getFilterFramebuffers()->clear();
	}

	if (!deferredRenderingObjects.empty())
	{
		IGBuffer* gBuffer = getGBuffer();
		gBuffer->geometryPass(deferredRenderingObjects);

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

	if (!forwardRenderingObjects.empty())
	{
		IFramebuffers* filterFramebuffers = getFilterFramebuffers();
		if (filterMode != GMFilterMode::None)
		{
			filterFramebuffers->clear();
			filterFramebuffers->bind();
		}

		draw(forwardRenderingObjects);

		if (filterMode != GMFilterMode::None)
		{
			filterFramebuffers->unbind();
			getFilterQuad()->draw();
		}
	}
}

void GMGraphicEngine::draw(const List<GMGameObject*>& objects)
{
	D(d);
	for (auto object : objects)
	{
		object->draw();
	}
}

const GMFilterMode::Mode GMGraphicEngine::getCurrentFilterMode()
{
	D(d);
	return d->renderConfig.get(GMRenderConfigs::FilterMode).toEnum<GMFilterMode::Mode>();
}

IFramebuffers* GMGraphicEngine::getShadowMapFramebuffers()
{
	D(d);
	return d->shadowDepthFramebuffers;
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
		GMFramebuffersDesc fbDesc;
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

void GMGraphicEngine::setShadowSource(const GMShadowSourceDesc& desc)
{
	D(d);
	d->shadow = desc;
}

void GMGraphicEngine::addLight(AUTORELEASE ILight* light)
{
	D(d);
	d->lights.push_back(light);
	update(GMUpdateDataType::LightChanged);
}

void GMGraphicEngine::removeLights()
{
	D(d);
	for (auto light : d->lights)
	{
		GM_delete(light);
	}

	d->lights.clear();
	update(GMUpdateDataType::LightChanged);
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