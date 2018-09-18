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
	GMLResult GM_SYSTEM_CALLBACK WndProc(GMWindowHandle hWnd, GMuint uMsg, GMWParam wParam, GMLParam lParam)
	{
		GMXEventContext* c = reinterpret_cast<GMXEventContext*>(lParam);
		GMWindow* window = gm_cast<GMWindow*>(c->window);
		GMSystemEvent* sysEvent = new GMSystemEvent();
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

GMWindowHandle GMWindow::create(const GMWindowAttributes& wndAttrs)
{
	D(d);
	onWindowCreated(wndAttrs);
	d->windowStates.renderRect = getRenderRect();
	return getWindowHandle();
}

IInput* GMWindow::getInputMananger()
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

	GMint x = (context->getScreenWidth() - windowRect.width) / 2;
	GMint y = (context->getScreenHeight() - windowRect.height) / 2;
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

GMWindowProcHandler GMWindow::getProcHandler()
{
	return &WndProc;
}
