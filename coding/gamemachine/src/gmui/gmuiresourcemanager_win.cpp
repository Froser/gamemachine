#include "stdafx.h"
#include "gmuiresourcemanager_win.h"

GMUIResourceManager::GMUIResourceManager()
{
	D(d);
	d->wndPaint = NULL;
	d->dcPaint = NULL;
	d->dcOffscreen = NULL;
	d->bmpOffscreen = NULL;
	d->wndTooltip = NULL;
	d->showUpdateRect = false;
	d->timerID = 0x1000;

	d->firstLayout = true;
	d->focusNeeded = false;
	d->updateNeeded = false;
	d->mouseTracking = false;
	d->mouseCapture = false;
	d->offscreenPaint = true;
}

void GMUIResourceManager::setResourceInstance(HINSTANCE hInstance)
{
	D_OF(d, &GMUIResourceManager::instance());
	d->instance = hInstance;
}

HINSTANCE GMUIResourceManager::getResourceInstance()
{
	D_OF(d, &GMUIResourceManager::instance());
	return d->instance;
}