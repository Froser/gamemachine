#include "stdafx.h"
#include <gamemachine.h>
#include "gmdx11graphic_engine.h"
#include "gmdx11renderers.h"
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
		initShaders(db->context);
	else
		GM_ASSERT(false);
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
		state.cubeMapRenderer = nullptr;
		state.model = nullptr;
		break;
	}
	}
}

void GMDx11GraphicEngine::activateLights(IRenderer* renderer)
{
	D(d);
	if (d->lightDirty)
	{
		IShaderProgram* shaderProgram = getShaderProgram();
		const Vector<ILight*>& lights = getLights();
		GMsize_t lightCount = lights.size();
		GM_ASSERT(lightCount <= getMaxLightCount());
		GM_ASSERT(lightCount <= std::numeric_limits<GMuint>::max());
		shaderProgram->setInt(GM_VariablesDesc.LightCount, (GMuint)lightCount);

		for (GMuint i = 0; i < (GMuint)lightCount; ++i)
		{
			lights[i]->activateLight(i, renderer);
		}
		d->lightDirty = false;
	}
}

IShaderProgram* GMDx11GraphicEngine::getShaderProgram(GMShaderProgramType type)
{
	D(d);
	return d->shaderProgram;
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
		gm_error("You must specify a IShaderLoadCallback");
		GM_ASSERT(false);
		return;
	}

	getShaderLoadCallback()->onLoadShaders(context);
}

IRenderer* GMDx11GraphicEngine::getRenderer(GMModelType objectType)
{
	D(d);
	D_BASE(db, Base);
	switch (objectType)
	{
	case GMModelType::Model2D:
		return newRenderer<GMDx11Renderer_2D>(d->renderer_2d, db->context);
	case GMModelType::Text:
		return newRenderer<GMDx11Renderer_Text>(d->renderer_text, db->context);
	case GMModelType::Model3D:
		if (isDrawingShadow())
			return newRenderer<GMDx11Renderer_3D_Shadow>(d->renderer_3d_shadow, db->context);
		if (getGBuffer()->getGeometryPassingState() != GMGeometryPassingState::Done)
			return newRenderer<GMDx11Renderer_Deferred_3D>(d->renderer_deferred_3d, db->context);
		return newRenderer<GMDx11Renderer_3D>(d->renderer_3d, db->context);
	case GMModelType::CubeMap:
		return newRenderer<GMDx11Renderer_CubeMap>(d->renderer_cubemap, db->context);
	case GMModelType::Filter:
		return newRenderer<GMDx11Renderer_Filter>(d->renderer_filter, db->context);
	case GMModelType::LightPassQuad:
		return newRenderer<GMDx11Renderer_Deferred_3D_LightPass>(d->renderer_deferred_3d_lightpass, db->context);
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