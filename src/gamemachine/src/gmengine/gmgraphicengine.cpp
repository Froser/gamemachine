#include "stdafx.h"
#include "gmgraphicengine.h"
#include "foundation/utilities/utilities.h"
#include "gmassets.h"
#include "foundation/gamemachine.h"
#include "gmengine/gameobjects/gmgameobject.h"
#include "foundation/gmprofile.h"
#include "foundation/gmconfigs.h"

static GMShaderVariablesDesc s_defaultShaderVariablesDesc =
{
	"GM_WorldMatrix",
	"GM_ViewMatrix",
	"GM_ProjectionMatrix",
	"GM_InverseTransposeModelMatrix",
	"GM_InverseViewMatrix",

	"GM_ViewPosition",

	{ "OffsetX", "OffsetY", "ScaleX", "ScaleY", "Enabled", "Texture" },
	"GM_AmbientTextureAttribute",
	"GM_DiffuseTextureAttribute",
	"GM_SpecularTextureAttribute",
	"GM_NormalMapTextureAttribute",
	"GM_LightmapTextureAttribute",
	"GM_AlbedoTextureAttribute",
	"GM_MetallicRoughnessAOTextureAttribute",
	"GM_CubeMapTextureAttribute",

	"GM_LightCount",

	{ "Ka", "Kd", "Ks", "Shininess", "Refractivity", "F0" },
	"GM_Material",

	{
		"GM_Filter",
		"GM_KernelDeltaX",
		"GM_KernelDeltaY",
		{
			"GM_DefaultFilter",
			"GM_InversionFilter",
			"GM_SharpenFilter",
			"GM_BlurFilter",
			"GM_GrayscaleFilter",
			"GM_EdgeDetectFilter",
		}
	},

	{
		"GM_ScreenInfo",
		"ScreenWidth",
		"ScreenHeight",
		"Multisampling",
	},

	"GM_RasterizerState",
	"GM_BlendState",
	"GM_DepthStencilState",

	{
		"GM_ShadowInfo",
		"HasShadow",
		"ShadowMatrix",
		"Position",
		"GM_ShadowMap",
		"GM_ShadowMapMSAA",
		"ShadowMapWidth",
		"ShadowMapHeight",
		"BiasMin",
		"BiasMax",
	},

	{
		"GM_GammaCorrection",
		"GM_Gamma",
		"GM_GammaInv",
	},

	{
		"GM_HDR",
		"GM_ToneMapping",
	},

	"GM_IlluminationModel",
};

GMint64 GMShadowSourceDesc::version = 0;

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

GMGraphicEngine::GMGraphicEngine(const IRenderContext* context)
{
	D(d);
	d->context = context;
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
	GM_delete(d->defaultFramebuffers);
	GM_delete(d->glyphManager);
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
	if (filterMode != GMFilterMode::None || needHDR())
	{
		createFilterFramebuffer();
		getFilterFramebuffers()->clear();
	}

	if (!deferredRenderingObjects.empty())
	{
		IGBuffer* gBuffer = getGBuffer();
		gBuffer->geometryPass(deferredRenderingObjects);

		if (needHDR() || filterMode != GMFilterMode::None)
		{
			IFramebuffers* filterFramebuffers = getFilterFramebuffers();
			GM_ASSERT(filterFramebuffers);
			filterFramebuffers->bind();
			filterFramebuffers->clear();
		}
		gBuffer->lightPass();
		if (needHDR() || filterMode != GMFilterMode::None)
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
		if (needHDR() || filterMode != GMFilterMode::None)
		{
			filterFramebuffers->clear();
			filterFramebuffers->bind();
		}

		draw(forwardRenderingObjects);

		if (needHDR() || filterMode != GMFilterMode::None)
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

bool GMGraphicEngine::needGammaCorrection()
{
	D(d);
	return d->renderConfig.get(GMRenderConfigs::GammaCorrection_Bool).toBool();
}

GMfloat GMGraphicEngine::getGammaValue()
{
	D(d);
	return d->renderConfig.get(GMRenderConfigs::Gamma_Float).toFloat();
}

bool GMGraphicEngine::needHDR()
{
	D(d);
	return d->renderConfig.get(GMRenderConfigs::HDR_Bool).toBool();
}

GMToneMapping::Mode GMGraphicEngine::getToneMapping()
{
	D(d);
	return d->renderConfig.get(GMRenderConfigs::ToneMapping).toInt();
}

void GMGraphicEngine::createFilterFramebuffer()
{
	D(d);
	if (!d->filterFramebuffers)
	{
		IFactory* factory = GM.getFactory();
		const GMWindowStates& windowStates = d->context->getWindow()->getWindowStates();
		GMFramebufferDesc desc = { 0 };
		desc.rect = windowStates.renderRect;
		desc.framebufferFormat = GMFramebufferFormat::R32G32B32A32_FLOAT;
		factory->createFramebuffers(d->context, &d->filterFramebuffers);
		GM_ASSERT(d->filterFramebuffers);
		GMFramebuffersDesc fbDesc;
		fbDesc.rect = windowStates.renderRect;
		d->filterFramebuffers->init(fbDesc);
		IFramebuffer* framebuffer = nullptr;
		factory->createFramebuffer(d->context, &framebuffer);
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
		quad->getShader().getTextureList().getTextureSampler(GMTextureType::Ambient).addFrame(d->filterFramebuffers->getFramebuffer(0)->getTexture());
		d->filterQuadModel.reset(quad);
		GM.createModelDataProxyAndTransfer(d->context, quad);
		GMAsset asset = GMAssets::createIsolatedAsset(GMAssetType::Model, quad);
		d->filterQuad = new GMGameObject(asset);
		d->filterQuad->setContext(d->context);
	}
}

IGBuffer* GMGraphicEngine::createGBuffer()
{
	D(d);
	IGBuffer* gBuffer = nullptr;
	GM.getFactory()->createGBuffer(d->context, &gBuffer);
	GM_ASSERT(gBuffer);
	return gBuffer;
}

void GMGraphicEngine::setShadowSource(const GMShadowSourceDesc& desc)
{
	D(d);
	d->shadow = desc;
	d->shadow.version = ++GMShadowSourceDesc::version;
}

GMCamera& GMGraphicEngine::getCamera()
{
	D(d);
	return d->camera;
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

const GMShaderVariablesDesc& GMGraphicEngine::getDefaultShaderVariablesDesc()
{
	return s_defaultShaderVariablesDesc;
}