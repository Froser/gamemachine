#include "stdafx.h"
#include <gamemachine.h>
#include "gmdx11graphic_engine.h"
#include "gmdx11techniques.h"
#include <gmdx11framebuffer.h>
#include "foundation/utilities/utilities.h"
#include "gmengine/gameobjects/gmgameobject.h"
#include "gmdx11gbuffer.h"
#include "gmdx11glyphmanager.h"

GMDx11GraphicEngine::GMDx11GraphicEngine(const IRenderContext* context)
	: GMGraphicEngine(context)
{
}

void GMDx11GraphicEngine::init()
{
	D(d);
	D_BASE(db, Base);
	Base::init();

	if (d->ready)
	{
		initShaders(db->context);
		db->renderTechniqueManager->init();
	}
	else
	{
		GM_ASSERT(false);
	}
}

void GMDx11GraphicEngine::update(GMUpdateDataType type)
{
	D(d);
	switch (type)
	{
	case GMUpdateDataType::LightChanged:
		d->lightDirty = true;
		break;
	case GMUpdateDataType::TurnOffCubeMap:
	{
		GMDx11CubeMapState& state = getCubeMapState();
		state.hasCubeMap = false;
		state.cubeMapTechnique = nullptr;
		state.model = nullptr;
		break;
	}
	}
}

void GMDx11GraphicEngine::activateLights(ITechnique* technique)
{
	D(d);
	if (d->lightDirty)
	{
		IShaderProgram* shaderProgram = getShaderProgram();
		const Vector<ILight*>& lights = getLights();
		GMsize_t lightCount = lights.size();
		GM_ASSERT(lightCount <= getMaxLightCount());
		shaderProgram->setInt(GM_VariablesDesc.LightCount, gm_sizet_to_uint(lightCount));

		for (GMuint i = 0; i < (GMuint)lightCount; ++i)
		{
			lights[i]->activateLight(i, technique);
		}
		d->lightDirty = false;
	}
}

IShaderProgram* GMDx11GraphicEngine::getShaderProgram(GMShaderProgramType type)
{
	D(d);
	return d->shaderProgram.get();
}

IFramebuffers* GMDx11GraphicEngine::getDefaultFramebuffers()
{
	D_BASE(d, Base);
	if (!d->defaultFramebuffers)
		d->defaultFramebuffers = GMDx11Framebuffers::createDefaultFramebuffers(d->context);
	return d->defaultFramebuffers;
}

bool GMDx11GraphicEngine::setInterface(GameMachineInterfaceID id, void* in)
{
	D(d);
	switch (id)
	{
	case GameMachineInterfaceID::D3D11ShaderProgram:
		d->shaderProgram.reset(static_cast<IShaderProgram*>(in));
		break;
	default:
		return false;
	}
	return true;
}

bool GMDx11GraphicEngine::getInterface(GameMachineInterfaceID id, void** out)
{
	D(d);
	switch (id)
	{
	case GameMachineInterfaceID::D3D11Device:
		GM_ASSERT(d->device);
		d->device->AddRef();
		(*out) = d->device.get();
		break;
	case GameMachineInterfaceID::D3D11DeviceContext:
		GM_ASSERT(d->deviceContext);
		d->deviceContext->AddRef();
		(*out) = d->deviceContext.get();
		break;
	case GameMachineInterfaceID::D3D11SwapChain:
		GM_ASSERT(d->swapChain);
		d->swapChain->AddRef();
		(*out) = d->swapChain.get();
		break;
	case GameMachineInterfaceID::D3D11DepthStencilView:
		GM_ASSERT(d->depthStencilView);
		d->depthStencilView->AddRef();
		(*out) = d->depthStencilView.get();
		break;
	case GameMachineInterfaceID::D3D11RenderTargetView:
		GM_ASSERT(d->renderTargetView);
		d->renderTargetView->AddRef();
		(*out) = d->renderTargetView.get();
		break;
	case GameMachineInterfaceID::D3D11DepthStencilTexture:
		GM_ASSERT(d->depthStencilTexture);
		d->depthStencilTexture->AddRef();
		(*out) = d->depthStencilTexture.get();
		break;
	default:
		return false;
	}
	return true;
}

void GMDx11GraphicEngine::createShadowFramebuffers(OUT IFramebuffers** framebuffers)
{
	D_BASE(d, Base);
	GMDx11ShadowFramebuffers* sdframebuffers = new GMDx11ShadowFramebuffers(d->context);
	(*framebuffers) = sdframebuffers;

	GMFramebuffersDesc desc;
	GMRect rect;
	rect.width = d->shadow.width;
	rect.height = d->shadow.height;
	desc.rect = rect;
	bool succeed = sdframebuffers->init(desc);
	GM_ASSERT(succeed);
}

bool GMDx11GraphicEngine::event(const GMMessage& e)
{
	D(d);
	switch (e.msgType)
	{
	case GameMachineMessageType::Dx11Ready:
	{
		const IRenderContext* context = static_cast<IRenderContext*>(e.objPtr);
		if (context->getEngine() == this)
		{
			IWindow* queriable = context->getWindow();
			bool b = false;
			b = queriable->getInterface(GameMachineInterfaceID::D3D11Device, (void**)&d->device);
			GM_ASSERT(b);
			b = queriable->getInterface(GameMachineInterfaceID::D3D11DeviceContext, (void**)&d->deviceContext);
			GM_ASSERT(b);
			b = queriable->getInterface(GameMachineInterfaceID::D3D11SwapChain, (void**)&d->swapChain);
			GM_ASSERT(b);
			b = queriable->getInterface(GameMachineInterfaceID::D3D11DepthStencilView, (void**)&d->depthStencilView);
			GM_ASSERT(b);
			b = queriable->getInterface(GameMachineInterfaceID::D3D11DepthStencilTexture, (void**)&d->depthStencilTexture);
			GM_ASSERT(b);
			b = queriable->getInterface(GameMachineInterfaceID::D3D11RenderTargetView, (void**)&d->renderTargetView);
			GM_ASSERT(b);
			d->ready = true;
			return true;
		}
		break;
	}
	}

	return false;
}

void GMDx11GraphicEngine::initShaders(const IRenderContext* context)
{
	D(d);
	// 读取着色器
	if (!getShaderLoadCallback())
	{
		gm_error(gm_dbg_wrap("You must specify a IShaderLoadCallback"));
		GM_ASSERT(false);
		return;
	}

	getShaderLoadCallback()->onLoadShaders(context);
}

ITechnique* GMDx11GraphicEngine::getTechnique(GMModelType objectType)
{
	D(d);
	D_BASE(db, Base);
	switch (objectType)
	{
	case GMModelType::Model2D:
		return newTechnique<GMDx11Technique_2D>(d->technique_2d, db->context);
	case GMModelType::Text:
		return newTechnique<GMDx11Technique_Text>(d->technique_text, db->context);
	case GMModelType::Model3D:
		if (isDrawingShadow())
			return newTechnique<GMDx11Technique_3D_Shadow>(d->technique_3d_shadow, db->context);
		if (getGBuffer()->getGeometryPassingState() != GMGeometryPassingState::Done)
			return newTechnique<GMDx11Technique_Deferred_3D>(d->technique_deferred_3d, db->context);
		return newTechnique<GMDx11Technique_3D>(d->technique_3d, db->context);
	case GMModelType::CubeMap:
		return newTechnique<GMDx11Technique_CubeMap>(d->technique_cubemap, db->context);
	case GMModelType::Filter:
		return newTechnique<GMDx11Technique_Filter>(d->technique_filter, db->context);
	case GMModelType::LightPassQuad:
		return newTechnique<GMDx11Technique_Deferred_3D_LightPass>(d->technique_deferred_3d_lightpass, db->context);
	case GMModelType::Particle:
		return newTechnique<GMDx11Technique_Particle>(d->technique_particle, db->context);
	case GMModelType::Custom:
		return newTechnique<GMDx11Technique_Custom>(d->technique_custom, db->context);
	default:
		GM_ASSERT(false);
		return nullptr;
	}
}

GMGlyphManager* GMDx11GraphicEngine::getGlyphManager()
{
	D_BASE(d, Base);
	if (!d->glyphManager)
	{
		d->glyphManager = new GMDx11GlyphManager(d->context);
	}
	return d->glyphManager;
}