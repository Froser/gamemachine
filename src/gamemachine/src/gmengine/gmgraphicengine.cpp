#include "stdafx.h"
#include "gmgraphicengine.h"
#include "foundation/utilities/utilities.h"
#include "gmassets.h"
#include "foundation/gamemachine.h"
#include "gmengine/gameobjects/gmgameobject.h"
#include "foundation/gmprofile.h"
#include "foundation/gmconfigs.h"
#include "gmprimitivemanager.h"

static GMShaderVariablesDesc s_defaultShaderVariablesDesc =
{
	"GM_WorldMatrix",
	"GM_ViewMatrix",
	"GM_ProjectionMatrix",
	"GM_InverseTransposeModelMatrix",
	"GM_InverseViewMatrix",

	"GM_Bones",
	"GM_UseBoneAnimation",

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
	"GM_ColorVertexOp",

	{
		"GM_Debug_Normal",
	}
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
	d->renderTechniqueManager.reset(new GMRenderTechniqueManager(context));
	d->primitiveManager.reset(new GMPrimitiveManager(context));
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
	D(d);
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
	// 如果绘制阴影，先生成阴影缓存
	if (d->shadow.type != GMShadowSourceDesc::NoShadow)
		generateShadowBuffer(forwardRenderingObjects, deferredRenderingObjects);

	// 是否使用滤镜
	bool useFilterFramebuffer = needUseFilterFramebuffer();
	if (useFilterFramebuffer)
	{
		createFilterFramebuffer();
		getFilterFramebuffers()->clear();
	}

	// 绘制需要延迟渲染的对象
	if (!deferredRenderingObjects.empty())
	{
		IGBuffer* gBuffer = getGBuffer();
		gBuffer->geometryPass(deferredRenderingObjects);

		if (useFilterFramebuffer)
			bindFilterFramebufferAndClear();

		gBuffer->lightPass();
		if (useFilterFramebuffer)
		{
			unbindFilterFramebufferAndDraw();
			gBuffer->getGeometryFramebuffers()->copyDepthStencilFramebuffer(getFilterFramebuffers());
		}
		else
		{
			gBuffer->getGeometryFramebuffers()->copyDepthStencilFramebuffer(getDefaultFramebuffers());
		}
	}

	// 绘制不需要延迟渲染的对象
	if (!forwardRenderingObjects.empty())
	{
		if (useFilterFramebuffer)
			bindFilterFramebufferAndClear();

		draw(forwardRenderingObjects);

		if (useFilterFramebuffer)
			unbindFilterFramebufferAndDraw();
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

bool GMGraphicEngine::isWireFrameMode(GMModel* model)
{
	D(d);
	// 有几种类型的Model不会绘制线框图
	auto type = model->getType();
	if (type == GMModelType::Model2D ||
		type == GMModelType::Text)
	{
		return false;
	}

	return d->debugConfig.get(GMDebugConfigs::WireFrameMode_Bool).toBool();
}

bool GMGraphicEngine::isNeedDiscardTexture(GMModel* model, GMTextureType type)
{
	D(d);
	return type != GMTextureType::Lightmap &&
		d->debugConfig.get(GMDebugConfigs::DrawLightmapOnly_Bool).toBool() &&
		model->getType() != GMModelType::Model2D &&
		model->getType() != GMModelType::Text;
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
		GMModelAsset quad;
		GMPrimitiveCreator::createQuadrangle(GMPrimitiveCreator::one2(), 0, quad);
		GM_ASSERT(!quad.isEmpty());
		GMModel* quadModel = quad.getScene()->getModels()[0].getModel();
		quadModel->setType(GMModelType::Filter);

		GMTextureAsset texture;
		d->filterFramebuffers->getFramebuffer(0)->getTexture(texture);
		quadModel->getShader().getTextureList().getTextureSampler(GMTextureType::Ambient).addFrame(texture);
		createModelDataProxy(d->context, quadModel);
		d->filterQuad = new GMGameObject(quad);
		d->filterQuad->setContext(d->context);
	}
}

void GMGraphicEngine::generateShadowBuffer(const List<GMGameObject*>& forwardRenderingObjects, const List<GMGameObject*>& deferredRenderingObjects)
{
	D(d);
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

bool GMGraphicEngine::needUseFilterFramebuffer()
{
	GMFilterMode::Mode filterMode = getCurrentFilterMode();
	return (filterMode != GMFilterMode::None || needHDR());
}

void GMGraphicEngine::bindFilterFramebufferAndClear()
{
	IFramebuffers* filterFramebuffers = getFilterFramebuffers();
	GM_ASSERT(filterFramebuffers);
	filterFramebuffers->bind();
	filterFramebuffers->clear();
}

void GMGraphicEngine::unbindFilterFramebufferAndDraw()
{
	IFramebuffers* filterFramebuffers = getFilterFramebuffers();
	GM_ASSERT(filterFramebuffers);
	filterFramebuffers->unbind();
	getFilterQuad()->draw();
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
	GMint64 ver = d->shadow.version;
	d->shadow = desc;
	d->shadow.version = ++ver;
}

GMCamera& GMGraphicEngine::getCamera()
{
	D(d);
	return d->camera;
}

void GMGraphicEngine::setCamera(const GMCamera& camera)
{
	D(d);
	d->camera = camera;
	d->camera.updateViewMatrix();
}

void GMGraphicEngine::beginBlend(
	GMS_BlendFunc sfactorRGB,
	GMS_BlendFunc dfactorRGB,
	GMS_BlendOp opRGB,
	GMS_BlendFunc sfactorAlpha,
	GMS_BlendFunc dfactorAlpha,
	GMS_BlendOp opAlpha
)
{
	D(d);
	++d->blendState.blendRefCount;
	d->blendState.enabled = true;
	d->blendState.sourceRGB = sfactorRGB;
	d->blendState.destRGB = dfactorRGB;
	d->blendState.opRGB = opRGB;
	d->blendState.sourceAlpha = sfactorAlpha;
	d->blendState.destAlpha = dfactorAlpha;
	d->blendState.opAlpha = opAlpha;
}

void GMGraphicEngine::endBlend()
{
	D(d);
	if (--d->blendState.blendRefCount == 0)
	{
		d->blendState.enabled = false;
	}
}

GMRenderTechniqueManager* GMGraphicEngine::getRenderTechniqueManager()
{
	D(d);
	return d->renderTechniqueManager.get();
}

GMPrimitiveManager* GMGraphicEngine::getPrimitiveManager()
{
	D(d);
	return d->primitiveManager.get();
}

void GMGraphicEngine::createModelDataProxy(const IRenderContext* context, GMModel* model, bool transfer)
{
	if (model)
	{
		GMModelDataProxy* modelDataProxy = model->getModelDataProxy();
		if (!modelDataProxy)
		{
			GM.getFactory()->createModelDataProxy(context, model, &modelDataProxy);
			model->setModelDataProxy(modelDataProxy);
		}
		if (transfer)
			modelDataProxy->transfer();
	}
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