#include "stdafx.h"
#include <gamemachine.h>
#include "gmdx11graphic_engine.h"
#include "gmdx11renderers.h"
#include <gmdx11framebuffer.h>
#include "foundation/utilities/utilities.h"
#include "gmengine/gameobjects/gmgameobject.h"
#include "gmdx11gbuffer.h"

void GMDx11GraphicEngine::init()
{
	D(d);
	D_BASE(db, Base);
	if (d->ready)
		initShaders();
	else
		GM_ASSERT(false);
}

void GMDx11GraphicEngine::newFrame()
{
	D(d);
	static const GMfloat clear[4] = { 0, 0, 0, 1 };
	d->deviceContext->RSSetState(NULL);
	d->deviceContext->ClearRenderTargetView(d->renderTargetView, clear);
	d->deviceContext->ClearDepthStencilView(d->depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0U);
}

void GMDx11GraphicEngine::update(GMUpdateDataType type)
{
	D(d);
	switch (type)
	{
	case GMUpdateDataType::TurnOffCubeMap:
	{
		GMDx11CubeMapState& state = GMDx11Renderer::getCubeMapState();
		state.hasCubeMap = false;
		state.cubeMapRenderer = nullptr;
		state.model = nullptr;
		break;
	}
	}
}

void GMDx11GraphicEngine::clearStencil()
{
	D(d);
	d->deviceContext->ClearDepthStencilView(d->depthStencilView, D3D11_CLEAR_STENCIL, 0, 0U);
}

void GMDx11GraphicEngine::beginBlend(GMS_BlendFunc sfactor /*= GMS_BlendFunc::ONE*/, GMS_BlendFunc dfactor /*= GMS_BlendFunc::ONE*/)
{
	D(d);
	++d->blendState.blendRefCount;
	d->blendState.enabled = true;
	d->blendState.source = sfactor;
	d->blendState.dest = dfactor;
}

void GMDx11GraphicEngine::endBlend()
{
	D(d);
	if (--d->blendState.blendRefCount == 0)
	{
		d->blendState.enabled = false;
	}
}

IShaderProgram* GMDx11GraphicEngine::getShaderProgram(GMShaderProgramType type)
{
	D(d);
	return d->shaderProgram;
}

IFramebuffers* GMDx11GraphicEngine::getDefaultFramebuffers()
{
	return GMDx11Framebuffers::getDefaultFramebuffers();
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

bool GMDx11GraphicEngine::event(const GameMachineMessage& e)
{
	D(d);
	switch (e.msgType)
	{
	case GameMachineMessageType::Dx11Ready:
		IQueriable* queriable = static_cast<IQueriable*>(e.objPtr);
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

	return false;
}

void GMDx11GraphicEngine::initShaders()
{
	D(d);
	// 读取着色器
	if (!getShaderLoadCallback())
	{
		gm_error("You must specify a IShaderLoadCallback");
		GM_ASSERT(false);
		return;
	}

	getShaderLoadCallback()->onLoadShaders(this);
}

IRenderer* GMDx11GraphicEngine::getRenderer(GMModelType objectType)
{
	D(d);
	static GMDx11Renderer_3D s_renderer_3d;
	static GMDx11Renderer_2D s_renderer_2d;
	static GMDx11Renderer_Glyph s_renderer_glyph;
	static GMDx11Renderer_CubeMap s_renderer_cubemap;
	static GMDx11Renderer_Filter s_renderer_filter;
	static GMDx11Renderer_Deferred_3D s_renderer_deferred_3d;
	static GMDx11Renderer_Deferred_3D_LightPass s_renderer_deferred_3d_lightpass;
	switch (objectType)
	{
	case GMModelType::Model2D:
		return &s_renderer_2d;
	case GMModelType::Glyph:
		return &s_renderer_glyph;
	case GMModelType::Model3D:
		if (getGBuffer()->getGeometryPassingState() != GMGeometryPassingState::Done)
			return &s_renderer_deferred_3d;
		return &s_renderer_3d;
	case GMModelType::CubeMap:
		return &s_renderer_cubemap;
	case GMModelType::Filter:
		return &s_renderer_filter;
	case GMModelType::LightPassQuad:
		return &s_renderer_deferred_3d_lightpass;
	default:
		GM_ASSERT(false);
		return nullptr;
	}
}