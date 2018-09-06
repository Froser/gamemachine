#include "stdafx.h"
#include "gmengine/ui/gmwindow.h"
#include <X11/Xlib.h>
#include <GL/glx.h>
#include "gminput.h"
#include "gmxrendercontext.h"
#include "gmwindowhelper.h"

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
	GM_ASSERT(false);
}

bool GMWindow::isWindowActivate()
{
	GM_ASSERT(false);
	return true;
}

void GMWindow::setWindowCapture(bool)
{
	GM_ASSERT(false);
}

void GMWindow::msgProc(const GMMessage& message)
{
	GM_ASSERT(false);
}

void GMWindow::changeCursor()
{
	GM_ASSERT(false);
}

GMWindowProcHandler GMWindow::getProcHandler()
{
	GM_ASSERT(false);
	return 0;
}
