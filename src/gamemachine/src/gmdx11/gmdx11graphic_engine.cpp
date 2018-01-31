#include "stdafx.h"
#include <gamemachine.h>
#include "gmdx11graphic_engine.h"
#include "shader_constants.h"

void GMDx11GraphicEngine::init()
{
}

void GMDx11GraphicEngine::newFrame()
{
	D(d);
	static const GMfloat clear[4] = { 0 };
	d->deviceContext->ClearRenderTargetView(d->renderTargetView, clear);
	d->deviceContext->ClearDepthStencilView(d->depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void GMDx11GraphicEngine::drawObjects(GMGameObject *objects[], GMuint count, GMBufferMode bufferMode /*= GMBufferMode::Normal*/)
{
	throw std::logic_error("The method or operation is not implemented.");
}

void GMDx11GraphicEngine::update(GMUpdateDataType type)
{
	D(d);
	switch (type)
	{
	case GMUpdateDataType::ProjectionMatrix:
	{
		updateProjection();
		break;
	}
	case GMUpdateDataType::ViewMatrix:
	{
		updateView();
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
	throw std::logic_error("The method or operation is not implemented.");
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

		// 一切准备就绪才开始初始化
		initShaders();
		return true;
	}
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

	// 定义统一MVP Matrix缓存
	D3D11_BUFFER_DESC vpBufferDesc;
	vpBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	vpBufferDesc.ByteWidth = sizeof(GMDxVPMatrix);
	vpBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	vpBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vpBufferDesc.MiscFlags = 0;
	vpBufferDesc.StructureByteStride = 0;

	GMComPtr<ID3D11Buffer> matrixBuf;
	hr = d->device->CreateBuffer(&vpBufferDesc, NULL, &matrixBuf);
	GM_COM_CHECK(hr);

	GM.getCamera().getFrustum().setDxMatrixBuffer(matrixBuf);
}

void GMDx11GraphicEngine::updateProjection()
{
	D(d);
	D3DXMATRIX proj = GM.getCamera().getFrustum().getDxProjectionMatrix();

	GMDxVPMatrix* mvpMatrix = nullptr;
	beginMapMVPMatrix(&mvpMatrix);
	GM_ASSERT(mvpMatrix);

	mvpMatrix->dxProjMatrix = proj;
	endMapMVPMatrix();
}

void GMDx11GraphicEngine::updateView()
{
	D(d);
	D3DXMATRIX view = GM.getCamera().getFrustum().getDxViewMatrix();

	GMDxVPMatrix* mvpMatrix = nullptr;
	beginMapMVPMatrix(&mvpMatrix);
	GM_ASSERT(mvpMatrix);

	mvpMatrix->dxViewMatrix = view;
	endMapMVPMatrix();

	// TODO 确定eye位置
}

void GMDx11GraphicEngine::beginMapMVPMatrix(GMDxVPMatrix** mvp)
{
	D(d);
	HRESULT hr;
	GMComPtr<ID3D11Buffer> mvpBuf = GM.getCamera().getFrustum().getDxMatrixBuffer();

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	hr = d->deviceContext->Map(mvpBuf, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	GM_COM_CHECK(hr);

	(*mvp) = static_cast<GMDxVPMatrix*>(mappedResource.pData);
}

void GMDx11GraphicEngine::endMapMVPMatrix()
{
	D(d);
	d->deviceContext->Unmap(GM.getCamera().getFrustum().getDxMatrixBuffer(), 0);
}