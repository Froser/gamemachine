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
	if (d->ready)
		initShaders();
	else
		GM_ASSERT(false);

	d->renderConfig = GM.getConfigs().getConfig(GMConfigs::Render).asRenderConfig();

	const GMRect& renderRect = GM.getGameMachineRunningStates().renderRect;
	d->renderConfig.set(GMRenderConfigs::FilterKernelOffset_Vec2, GMVec2(1.f / renderRect.width, 1.f / renderRect.height));
}

void GMDx11GraphicEngine::newFrame()
{
	D(d);
	static const GMfloat clear[4] = { 0, 0, 0, 1 };
	d->deviceContext->RSSetState(NULL);
	d->deviceContext->ClearRenderTargetView(d->renderTargetView, clear);
	d->deviceContext->ClearDepthStencilView(d->depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0U);
}

void GMDx11GraphicEngine::drawObjects(GMGameObject *objects[], GMuint count)
{
	GM_PROFILE("drawObjects");
	if (!count)
		return;

	D(d);
	D_BASE(db, Base);
	GMFilterMode::Mode filterMode = getCurrentFilterMode();
	if (filterMode != GMFilterMode::None)
		createFilterFramebuffer();

	d->needActivateLight = true;

	// 假设现在是延迟渲染
	// 把渲染图形分为两组，可延迟渲染组和不可延迟渲染组，先渲染可延迟渲染的图形
	groupGameObjects(objects, count);
	if (true)
	{
		IGBuffer* gBuffer = getGBuffer();
		gBuffer->geometryPass(d->deferredRenderingGameObjects.data(), d->deferredRenderingGameObjects.size());
		gBuffer->lightPass();
		forwardDraw(d->forwardRenderingGameObjects.data(), d->forwardRenderingGameObjects.size(), filterMode);
	}
	else
	{
		forwardDraw(objects, count, filterMode);
	}

	d->needActivateLight = false;
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

void GMDx11GraphicEngine::addLight(const GMLight& light)
{
	D(d);
	d->lights.push_back(light);
}

void GMDx11GraphicEngine::removeLights()
{
	D(d);
	d->lights.clear();
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

IShaderProgram* GMDx11GraphicEngine::getShaderProgram(GMShaderProgramType type /*= GMShaderProgramType::CurrentShaderProgram*/)
{
	D(d);
	return d->shaderProgram;
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
	if (!d->shaderLoadCallback)
	{
		gm_error("You must specify a IShaderLoadCallback");
		GM_ASSERT(false);
		return;
	}

	d->shaderLoadCallback->onLoadShaders(this);
}

void GMDx11GraphicEngine::forwardDraw(GMGameObject *objects[], GMuint count, GMFilterMode::Mode filter)
{
	D(d);
	IFramebuffers* filterFramebuffers = getFilterFramebuffers();
	if (filter != GMFilterMode::None)
	{
		filterFramebuffers->clear();
		filterFramebuffers->bind();
	}

	draw(objects, count);

	if (filter != GMFilterMode::None)
	{
		filterFramebuffers->unbind();
		getFilterQuad()->draw();
	}
}

void GMDx11GraphicEngine::draw(GMGameObject *objects[], GMuint count)
{
	for (GMuint i = 0; i < count; i++)
	{
		objects[i]->draw();
	}
}

void GMDx11GraphicEngine::directDraw(GMGameObject *objects[], GMuint count, GMFilterMode::Mode filter)
{
	D(d);
	draw(objects, count);
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

void GMDx11GraphicEngine::groupGameObjects(GMGameObject *objects[], GMuint count)
{
	D(d);
	d->deferredRenderingGameObjects.clear();
	d->deferredRenderingGameObjects.reserve(count);
	d->forwardRenderingGameObjects.clear();
	d->forwardRenderingGameObjects.reserve(count);

	for (GMuint i = 0; i < count; i++)
	{
		if (objects[i]->canDeferredRendering())
			d->deferredRenderingGameObjects.push_back(objects[i]);
		else
			d->forwardRenderingGameObjects.push_back(objects[i]);
	}
}
