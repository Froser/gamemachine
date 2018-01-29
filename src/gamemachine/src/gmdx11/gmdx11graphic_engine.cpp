#include "stdafx.h"
#include "gmdx11graphic_engine.h"

void GMDx11GraphicEngine::init()
{
}

void GMDx11GraphicEngine::newFrame()
{
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

bool GMDx11GraphicEngine::event(const GameMachineMessage& e)
{
	D(d);
	switch (e.msgType)
	{
	{
	case GameMachineMessageType::Dx11_DeviceReady:
		GM_ASSERT(!d->device);
		d->device = static_cast<ID3D11Device*>(e.objPtr);
		break;
	}
	{
	case GameMachineMessageType::Dx11_DeviceContextReady:
		GM_ASSERT(!d->context);
		d->context = static_cast<ID3D11DeviceContext*>(e.objPtr);
		break;
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