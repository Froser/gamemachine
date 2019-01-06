#include "stdafx.h"
#include "gmwindow.h"
#include "gmengine/gmmessage.h"
#include "foundation/gamemachine.h"
#include "gmwidget.h"

bool GMWindow::getInterface(GameMachineInterfaceID id, void** out)
{
	return false;
}

bool GMWindow::setInterface(GameMachineInterfaceID id, void* in)
{
	return false;
}

bool GMWindow::handleSystemEvent(GMSystemEvent* event, REF GMLResult& result)
{
	if (!event)
		return false;

	switch (event->getType())
	{
	case GMSystemEventType::WindowDestroyed:
		GM.postMessage({ GameMachineMessageType::WindowDestoryed, 0, static_cast<IWindow*>(this) });
		break;
	case GMSystemEventType::WindowSizeChanged:
		GM.postMessage({ GameMachineMessageType::WindowSizeChanged });
		break;
	case GMSystemEventType::MouseWheel:
	case GMSystemEventType::MouseMove:
	case GMSystemEventType::MouseDown:
	case GMSystemEventType::MouseUp:
		getInputManager()->handleSystemEvent(event);
		break;
	case GMSystemEventType::SetCursor:
		changeCursor();
		break;
	}

	D(d);
	for (auto& widget : d->widgets)
	{
		if (widget->handleSystemEvent(event))
			return true;
	}
	return false;
}

bool GMWindow::addWidget(GMWidget* widget)
{
	D(d);
	if (widget->getParentWindow() == this)
		return true;

	if (widget->getParentWindow())
	{
		// 已经有了parent window，则无法添加此widget
		GM_ASSERT(false);
		return false;
	}

	widget->setParentWindow(this);
	widget->onInit();
	d->widgets.push_back(widget);
	return true;
}

void GMWindow::setHandler(AUTORELEASE IGameHandler* handler)
{
	D(d);
	d->handler.reset(handler);
}

IGameHandler* GMWindow::getHandler()
{
	D(d);
	return d->handler.get();
}

const GMWindowStates& GMWindow::getWindowStates()
{
	D(d);
	return d->windowStates;
}

void GMWindow::setCursor(GMCursorType cursorType)
{
	D(d);
	d->cursor = cursorType;
}

void GMWindow::msgProc(const GMMessage& message)
{
	D(d);
	if (message.msgType == GameMachineMessageType::SystemMessage)
	{
		GMSystemEvent* event = static_cast<GMSystemEvent*>(message.object);
		for (auto widget : d->widgets)
		{
			widget->handleSystemEvent(event);
		}
	}
	else if (message.msgType == GameMachineMessageType::FrameUpdate)
	{
		if (d->input)
			d->input->update();

		d->windowStates.renderRect = getRenderRect();
		GMDuration elapsed = GM.getRunningStates().lastFrameElpased;
		for (auto widget : d->widgets)
		{
			widget->render(elapsed);
		}
	}
	else if (message.msgType == GameMachineMessageType::WindowDestoryed)
	{
		if (message.object == static_cast<IWindow*>(this))
		{
			onWindowDestroyed();
			// 窗口已经被Destory，因此HWND设置为0
			setWindowHandle(0, false);
			GM.removeWindow(this);
		}
	}
}

GMRect GMWindow::getFramebufferRect()
{
	D(d);
	return d->windowStates.framebufferRect;
}
