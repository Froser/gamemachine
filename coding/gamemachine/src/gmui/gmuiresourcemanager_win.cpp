#include "stdafx.h"
#include "gmuiresourcemanager_win.h"
#include <windowsx.h>
#include "foundation/vector.h"

static Vector<GMUIResourceManager*> s_messages;

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

void GMUIResourceManager::setResourceInstance(GMUIInstance hInstance)
{
	D(d);
	d->instance = hInstance;
}

GMUIInstance GMUIResourceManager::getResourceInstance()
{
	D(d);
	return d->instance;
}

void GMUIResourceManager::initWindow(GMUIWindowHandle handle)
{
	D(d);
	ASSERT(::IsWindow(handle));
	d->wndPaint = handle;
	s_messages.push_back(this);
}

bool GMUIResourceManager::translateMessage(const LPMSG pMsg)
{
	// Pretranslate Message takes care of system-wide messages, such as
	// tabbing and shortcut key-combos. We'll look for all messages for
	// each window and any child control attached.

	HWND hwndParent = ::GetParent(pMsg->hwnd);
	UINT uStyle = GetWindowStyle(pMsg->hwnd);
	LRESULT lRes = 0;

	for (auto iter = s_messages.begin(); iter != s_messages.end(); iter++)
	{
		GMUIResourceManager* p = (*iter);
		if (pMsg->hwnd == p->getPaintWindow()
			|| (hwndParent == p->getPaintWindow() && ((uStyle & WS_CHILD) != 0)))
		{
			if (p->preMessageHandler(pMsg->message, pMsg->wParam, pMsg->lParam, lRes))
				return true;
		}
	}

	return false;
}

GMUIWindowHandle GMUIResourceManager::getPaintWindow() const
{
	D(d);
	return d->wndPaint;
}

bool GMUIResourceManager::preMessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& /*lRes*/)
{
	return false;
}

bool GMUIResourceManager::handleMessage()
{
	MSG msg;
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		if (msg.message == WM_QUIT)
			return false;
		if (!translateMessage(&msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return true;
}