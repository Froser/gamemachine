#include "stdafx.h"
#include <gamemachine.h>
#include "gmdx11graphic_engine.h"
#include "shader_constants.h"

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
	d->deviceContext->ClearRenderTargetView(d->renderTargetView, clear);
	d->deviceContext->ClearDepthStencilView(d->depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
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
	D(d);
	switch (type)
	{
	case GMUpdateDataType::ProjectionMatrix:
	case GMUpdateDataType::ModelMatrix:
	case GMUpdateDataType::ViewMatrix:
	{
		updateAllMatrices();
		break;
	}
	case GMUpdateDataType::TurnOffCubeMap:
	{
		GM_ASSERT(false);
	}
	default:
		GM_ASSERT(false);
		break;
	}
}

void GMDx11GraphicEngine::addLight(const GMLight& light)
{
	throw std::logic_error("The method or operation is not implemented.");
}

void GMDx11GraphicEngine::removeLights()
{
	throw std::logic_error("The method or operation is not implemented.");
}

void GMDx11GraphicEngine::clearStencil()
{
	D(d);
	d->deviceContext->ClearDepthStencilView(d->depthStencilView, D3D11_CLEAR_STENCIL, 0, 0);
}

void GMDx11GraphicEngine::beginCreateStencil()
{
	throw std::logic_error("The method or operation is not implemented.");
}

void GMDx11GraphicEngine::endCreateStencil()
{
	throw std::logic_error("The method or operation is not implemented.");
}

void GMDx11GraphicEngine::beginUseStencil(bool inverse)
{
	throw std::logic_error("The method or operation is not implemented.");
}

void GMDx11GraphicEngine::endUseStencil()
{
	throw std::logic_error("The method or operation is not implemented.");
}

void GMDx11GraphicEngine::beginBlend(GMS_BlendFunc sfactor /*= GMS_BlendFunc::ONE*/, GMS_BlendFunc dfactor /*= GMS_BlendFunc::ONE*/)
{
	// throw std::logic_error("The method or operation is not implemented.");
}

void GMDx11GraphicEngine::endBlend()
{
	// throw std::logic_error("The method or operation is not implemented.");
}

IShaderProgram* GMDx11GraphicEngine::getShaderProgram(GMShaderProgramType type /*= GMShaderProgramType::CurrentShaderProgram*/)
{
	throw std::logic_error("The method or operation is not implemented.");
}

bool GMDx11GraphicEngine::setInterface(GameMachineInterfaceID id, void* in)
{
	D(d);
	switch (id)
	{
	case GameMachineInterfaceID::D3D11VertexShader:
		d->vertexShader = static_cast<ID3D11VertexShader*>(in);
		break;
	case GameMachineInterfaceID::D3D11PixelShader:
		d->pixelShader = static_cast<ID3D11PixelShader*>(in);
		break;
	case GameMachineInterfaceID::D3D11VertexShaderBuffer:
		d->vertexShaderBuffer = static_cast<ID3D10Blob*>(in);
		break;
	case GameMachineInterfaceID::D3D11PixelShaderBuffer:
		d->pixelShaderBuffer = static_cast<ID3D10Blob*>(in);
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
		b = queriable->getInterface(GameMachineInterfaceID::DXGISwapChain, (void**)&d->swapChain);
		GM_ASSERT(b);
		b = queriable->getInterface(GameMachineInterfaceID::D3D11DepthStencilView, (void**)&d->depthStencilView);
		GM_ASSERT(b);
		b = queriable->getInterface(GameMachineInterfaceID::D3D11RenderTargetView, (void**)&d->renderTargetView);
		GM_ASSERT(b);
		b = queriable->getInterface(GameMachineInterfaceID::D3D11RasterState, (void**)&d->rasterState);
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

	// 定义顶点布局
	HRESULT hr;
	hr = d->device->CreateInputLayout(
		GMSHADER_ElementDescriptions,
		GM_array_size(GMSHADER_ElementDescriptions),
		d->vertexShaderBuffer->GetBufferPointer(),
		d->vertexShaderBuffer->GetBufferSize(),
		&d->inputLayout
	);
	GM_COM_CHECK(hr);

	bool suc = GM.getCamera().getFrustum().createDxMatrixBuffer();
	GM_ASSERT(suc);
}

void GMDx11GraphicEngine::updateProjectionMatrix()
{
	updateAllMatrices();
}

void GMDx11GraphicEngine::updateAllMatrices()
{
	D(d);
	GMMVPMatrix mvpMatrix;
	mvpMatrix.modelMatrix = Transpose(GM.getCamera().getFrustum().getModelMatrix());
	mvpMatrix.viewMatrix = Transpose(GM.getCamera().getFrustum().getViewMatrix());
	mvpMatrix.projMatrix = Transpose(GM.getCamera().getFrustum().getProjectionMatrix());

	GMComPtr<ID3D11Buffer> buffer = GM.getCamera().getFrustum().getDxMatrixBuffer();
	d->deviceContext->UpdateSubresource(buffer, 0, NULL, &mvpMatrix, 0, 0);
	d->deviceContext->VSSetConstantBuffers(0, 1, &buffer);
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
