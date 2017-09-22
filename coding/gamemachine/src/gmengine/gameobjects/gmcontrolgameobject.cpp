#include "stdafx.h"
#include "gmcontrolgameobject.h"
#include "foundation/utilities/gmprimitivecreator.h"

//////////////////////////////////////////////////////////////////////////
GMControlEvent::GMControlEvent(GMControlEventType type)
	: m_type(type)
{
}

bool GM2DMouseDownEvent::buttonDown(Button button)
{
	if (button == GM2DMouseDownEvent::Left)
		return !!(m_state.button & GMMouseButton_Left);
	if (button == GM2DMouseDownEvent::Right)
		return !!(m_state.button & GMMouseButton_Right);
	if (button == GM2DMouseDownEvent::Middle)
		return !!(m_state.button & GMMouseButton_Middle);
	return false;
}
//////////////////////////////////////////////////////////////////////////
GMControlGameObject::GMControlGameObject()
{
	D(d);
	GMRect client = GM.getMainWindow()->getClientRect();
	d->clientSize = client;
}

void GMControlGameObject::setGeometry(const GMRect& rect)
{
	D(d);
	d->geometry = rect;
}

void GMControlGameObject::notifyControl()
{
	D(d);
	updateUI();

	IInput* input = GM.getMainWindow()->getInputMananger();
	IMouseState& mouseState = input->getMouseState();
	GMMouseState ms = mouseState.mouseState();

	if (insideGeometry(ms.posX, ms.posY))
	{
		GM2DMouseHoverEvent e(ms);
		event(&e);

		if (ms.button != GMMouseButton_None)
		{
			GM2DMouseDownEvent e(ms);
			event(&e);
		}
	}
}

bool GMControlGameObject::insideGeometry(GMint x, GMint y)
{
	D(d);
	return GM_in_rect(d->geometry, x, y);
}

void GMControlGameObject::updateUI()
{
	D(d);
	switch (GM.peekMessage().msgType)
	{
	case GameMachineMessageType::WindowSizeChanged:
		GMRect nowClient = GM.getMainWindow()->getClientRect();
		GMfloat scaleX = (GMfloat)nowClient.width / d->clientSize.width,
			scaleY = (GMfloat)nowClient.height / d->clientSize.height;

		if (getStretch())
		{
			d->geometry.x *= scaleX;
			d->geometry.y *= scaleY;
			d->geometry.width *= scaleX;
			d->geometry.height *= scaleY;
		}
		else
		{
			// 调整大小，防止拉伸
			GMfloat scaling[] = { 1.f / scaleX, 1.f / scaleY, 1 };
			GMPrimitiveUtil::scaleModel(*getModel(), scaling);

			// 相对于左上角位置也不能变
			GMRectF rect = toViewportCoord(d->geometry);
			GMfloat trans[] = { rect.x, rect.y, 0 };
			GMPrimitiveUtil::translateModelTo(*getModel(), trans);
		}

		d->clientSize = nowClient;
		break;
	}
}

GMRectF GMControlGameObject::toViewportCoord(const GMRect& in)
{
	GMRect client = GM.getMainWindow()->getClientRect();
	GMRectF out = {
		in.x * 2.f / client.width - 1.f,
		1.f - in.y * 2.f / client.height,
		(GMfloat)in.width / client.width,
		(GMfloat)in.height / client.height
	};
	return out;
}
