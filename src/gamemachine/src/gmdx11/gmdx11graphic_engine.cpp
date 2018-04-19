#include "stdafx.h"
#include <gamemachine.h>
#include "gmdx11graphic_engine.h"
#include "gmdx11renderers.h"
#include <gmdx11framebuffer.h>
#include "foundation/utilities/utilities.h"
#include "gmengine/gameobjects/gmgameobject.h"

GMDx11GraphicEngine::~GMDx11GraphicEngine()
{
	D(d);
	GM_delete(d->filterFramebuffers);
	GM_delete(d->filterQuad);
}

void GMDx11GraphicEngine::init()
{
	D(d);
	if (d->ready)
		initShaders();
	else
		GM_ASSERT(false);

	d->renderConfig = GM.getConfigs().getConfig(GMConfigs::Render).asRenderConfig();

	const GMRect& renderRect = GM.getGameMachineRunningStates().windowRect;
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

void GMDx11GraphicEngine::drawObjects(GMGameObject *objects[], GMuint count, GMBufferMode bufferMode /*= GMBufferMode::Normal*/)
{
	GM_PROFILE("drawObjects");
	if (!count)
		return;

	D(d);
	GMFilterMode::Mode filterMode = getCurrentFilterMode();
	if (filterMode != GMFilterMode::None)
		createFilterFramebuffer();

	d->needActivateLight = true;
	if (bufferMode == GMBufferMode::NoFramebuffer)
	{
		directDraw(objects, count, filterMode);
	}
	else
	{
		//TODO 考虑延迟渲染
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
	if (filter != GMFilterMode::None)
	{
		d->filterFramebuffers->clear();
		d->filterFramebuffers->bind();
	}

	forwardRender(objects, count);

	if (filter != GMFilterMode::None)
	{
		d->filterFramebuffers->unbind();
		GM_ASSERT(d->filterQuad);
		d->filterQuad->draw();
	}
}

void GMDx11GraphicEngine::forwardRender(GMGameObject *objects[], GMuint count)
{
	for (GMuint i = 0; i < count; i++)
	{
		objects[i]->draw();
	}
}

void GMDx11GraphicEngine::directDraw(GMGameObject *objects[], GMuint count, GMFilterMode::Mode filter)
{
	D(d);
	forwardRender(objects, count);
}

void GMDx11GraphicEngine::createFilterFramebuffer()
{
	D(d);
	if (!d->filterFramebuffers)
	{
		IFactory* factory = GM.getFactory();
		const GMGameMachineRunningStates& states = GM.getGameMachineRunningStates();
		GMFramebufferDesc desc = { 0 };
		desc.rect = states.windowRect;
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
		quad->getShader().getTexture().getTextureFrames(GMTextureType::Ambient, 0).addFrame(d->filterFramebuffers->getTexture(0));
		d->filterQuadModel.reset(quad);
		GM.createModelPainterAndTransfer(quad);
		GMAsset asset = GMAssets::createIsolatedAsset(GMAssetType::Model, quad);
		d->filterQuad = new GMGameObject(asset);
	}
}

IRenderer* GMDx11GraphicEngine::getRenderer(GMModelType objectType)
{
	D(d);
	static GMDx11Renderer_3D s_renderer_3d;
	static GMDx11Renderer_2D s_renderer_2d;
	static GMDx11Renderer_Glyph s_renderer_glyph;
	static GMDx11Renderer_CubeMap s_renderer_cubemap;
	static GMDx11Renderer_Filter s_renderer_filter;
	switch (objectType)
	{
	case GMModelType::Model2D:
		return &s_renderer_2d;
	case GMModelType::Glyph:
		return &s_renderer_glyph;
	case GMModelType::Model3D:
		return &s_renderer_3d;
	case GMModelType::CubeMap:
		return &s_renderer_cubemap;
	case GMModelType::Filter:
		return &s_renderer_filter;
	default:
		GM_ASSERT(false);
		return nullptr;
	}
}