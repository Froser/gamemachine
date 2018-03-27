#include "stdafx.h"
#include <gamemachine.h>
#include "gmdx11graphic_engine.h"
#include "gmdx11renderers.h"

void GMDx11GraphicEngine::init()
{
	D(d);
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

void GMDx11GraphicEngine::drawObjects(GMGameObject *objects[], GMuint count, GMBufferMode bufferMode /*= GMBufferMode::Normal*/)
{
	GM_PROFILE("drawObjects");
	if (!count)
		return;

	if (bufferMode == GMBufferMode::NoFramebuffer)
	{
		directDraw(objects, count);
	}
	else
	{
		//TODO 考虑延迟渲染
		forwardDraw(objects, count);
	}
}

void GMDx11GraphicEngine::update(GMUpdateDataType type)
{
	// Nothing to update
}

void GMDx11GraphicEngine::addLight(const GMLight& light)
{
}

void GMDx11GraphicEngine::removeLights()
{
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

void GMDx11GraphicEngine::forwardDraw(GMGameObject *objects[], GMuint count)
{
	// 先不考虑效果
	forwardRender(objects, count);
}

void GMDx11GraphicEngine::forwardRender(GMGameObject *objects[], GMuint count)
{
	D(d);
	for (GMuint i = 0; i < count; i++)
	{
		objects[i]->draw();
	}
}

void GMDx11GraphicEngine::directDraw(GMGameObject *objects[], GMuint count)
{
	D(d);
	//setCurrentRenderMode(GMStates_RenderOptions::FORWARD);
	//d->framebuffer.releaseBind();
	forwardRender(objects, count);
}

IRenderer* GMDx11GraphicEngine::getRenderer(GMModelType objectType)
{
	D(d);
	static GMDx11Renderer_3D s_renderer_3d;
	static GMDx11Renderer_2D s_renderer_2d;
	static GMDx11Renderer_Glyph s_renderer_glyph;
	switch (objectType)
	{
	case GMModelType::Model2D:
		return &s_renderer_2d;
	case GMModelType::Glyph:
		return &s_renderer_glyph;
	case GMModelType::Model3D:
	case GMModelType::Particles:
	case GMModelType::CubeMap:
		return &s_renderer_3d;
	default:
		GM_ASSERT(false);
		return nullptr;
	}
}