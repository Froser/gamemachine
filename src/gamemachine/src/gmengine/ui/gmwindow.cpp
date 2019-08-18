#include "stdafx.h"
#include "gmwindow.h"
#include "gmengine/gmmessage.h"
#include "foundation/gamemachine.h"
#include "gmwidget.h"
#include "gmwindow_p.h"

BEGIN_NS

bool GMWindow::getInterface(GameMachineInterfaceID id, void** out)
{
	return false;
}

bool GMWindow::setInterface(GameMachineInterfaceID id, void* in)
{
	return false;
}

void GMWindow::setWindowHandle(GMWindowHandle handle, bool autoRelease)
{
	D(d);
	d->handle = handle;
	d->ownedHandle = autoRelease;
}

bool GMWindow::handleSystemEvent(GMSystemEvent* event, REF GMLResult& result)
{
	if (!event)
		return false;

	bool r = false;
	switch (event->getType())
	{
	case GMSystemEventType::WindowAboutToDestroy:
	{
		GM.postMessage({ GameMachineMessageType::WindowAboutToDestroy, 0, static_cast<IWindow*>(this) });
		break;
	}
	case GMSystemEventType::WindowAboutToClose:
		r = !canClose();
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
	return r;
}

const GMwchar* GMWindow::getWindowClassName()
{
	return L"GameMachine Window";
}

GMWindowHandle GMWindow::getWindowHandle() const
{
	D(d);
	return d->handle;
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

bool GMWindow::canClose()
{
	return true;
}

GMWindow::GMWindow()
{
	GM_CREATE_DATA();
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
		GMDuration elapsed = GM.getRunningStates().lastFrameElapsed;
		for (auto widget : d->widgets)
		{
			widget->render(elapsed);
		}
	}
	else if (message.msgType == GameMachineMessageType::WindowAboutToDestroy)
	{
		if (message.object == static_cast<IWindow*>(this))
		{
			// 移除内核Window
			onWindowDestroyed();

			// 内核窗口已经被Destory，因此Handle设置为0
			setWindowHandle(0, false);

			// 删除GM的IWindow对象
			GM.removeWindow(this);
		}
	}
}

GMRect GMWindow::getFramebufferRect()
{
	D(d);
	return d->windowStates.framebufferRect;
}

END_NS
