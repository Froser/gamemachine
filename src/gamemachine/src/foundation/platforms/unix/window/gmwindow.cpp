#include "stdafx.h"
#include "gmengine/ui/gmwindow.h"
#include <X11/Xlib.h>
#include <GL/glx.h>
#include "gminput.h"
#include "gmxrendercontext.h"
#include "gmwindowhelper.h"
#include <gamemachine.h>

namespace
{
	GMLResult GM_SYSTEM_CALLBACK WndProc(GMWindowHandle hWnd, GMuint32 uMsg, GMWParam wParam, GMLParam lParam)
	{
		GMXEventContext* c = reinterpret_cast<GMXEventContext*>(lParam);
		GMWindow* window = gm_cast<GMWindow*>(c->window);
		GMSystemEvent* sysEvent = nullptr;
		GM.translateSystemEvent(uMsg, wParam, lParam, &sysEvent);
		GMScopedPtr<GMSystemEvent> guard(sysEvent);
		GMLResult lRes = 0;
		window->handleSystemEvent(sysEvent, lRes);
		return lRes;
	}
}

GMWindow::GMWindow()
{
}

GMWindow::~GMWindow()
{

}

void GMWindow::showWindow()
{
	const GMXRenderContext* context = gm_cast<const GMXRenderContext*>(getContext());
	XMapWindow(context->getDisplay(), getWindowHandle());
	XFlush(context->getDisplay());
}

GMWindowHandle GMWindow::create(const GMWindowDesc& desc)
{
	D(d);
	onWindowCreated(desc);
	d->windowStates.renderRect = getRenderRect();
	d->windowStates.framebufferRect = d->windowStates.renderRect;
	return getWindowHandle();
}

IInput* GMWindow::getInputManager()
{
	D(d);
	if (!d->input)
		d->input = gm_makeOwnedPtr<GMInput>(this);
	return d->input.get();
}

GMRect GMWindow::getWindowRect()
{
	// TODO WindowRect contains border
	return getRenderRect();
}

GMRect GMWindow::getRenderRect()
{
	const GMXRenderContext* context = gm_cast<const GMXRenderContext*>(getContext());
	GMRect rc = GMWindowHelper::getWindowRect(context->getDisplay(), getWindowHandle(), context->getRootWindow()); // Retrieve coords from root
	return rc;
}

void GMWindow::centerWindow()
{
	D(d);
	const auto& windowRect = getWindowRect();
	const GMXRenderContext* context = gm_cast<const GMXRenderContext*>(getContext());

	GMint32 x = (context->getScreenWidth() - windowRect.width) / 2;
	GMint32 y = (context->getScreenHeight() - windowRect.height) / 2;
	XMoveWindow(context->getDisplay(), getWindowHandle(), x, y);
}

bool GMWindow::isWindowActivate()
{
	return true;
}

void GMWindow::setWindowCapture(bool)
{
}

void GMWindow::changeCursor()
{
}

void GMWindow::onWindowDestroyed()
{
}

GMWindowProcHandler GMWindow::getProcHandler()
{
	return &WndProc;
}

void GMWindow::setMultithreadRenderingFlag(GMMultithreadRenderingFlag)
{
}