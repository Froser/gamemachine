#include "stdafx.h"
#include "gmdx11graphic_engine.h"

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
	throw std::logic_error("The method or operation is not implemented.");
}

void GMDx11GraphicEngine::endBlend()
{
	throw std::logic_error("The method or operation is not implemented.");
}

IShaderProgram* GMDx11GraphicEngine::getShaderProgram(GMShaderProgramType type /*= GMShaderProgramType::CurrentShaderProgram*/)
{
	throw std::logic_error("The method or operation is not implemented.");
}

bool GMDx11GraphicEngine::setInterface(GameMachineInterfaceID, void*)
{
	return false;
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
		return true;
	}
	}

	return false;
}

void GMDx11GraphicEngine::updateProjection()
{
}

void GMDx11GraphicEngine::updateView()
{

}