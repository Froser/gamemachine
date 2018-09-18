#include "stdafx.h"
#include "check.h"
#include "../../gamemachine.h"
#include <locale.h>
#include <X11/Xlib.h>
#include "window/gmxrendercontext.h"

namespace
{
	GMEndiannessMode getMachineEndianness()
	{
		long int i = 1;
		const char *p = (const char *)&i;
		if (p[0] == 1)
			return GMEndiannessMode::LittleEndian;
		return GMEndiannessMode::BigEndian;
	}

	IWindow* findWindow(const Set<GMOwnedPtr<IWindow>>& windows, Window window)
	{
		for (const auto& w : windows)
		{
			if (w->getWindowHandle() == window)
				return w.get();
		}
		return nullptr;
	}
}

void GameMachine::runEventLoop()
{
	D(d);
	const GMXRenderContext* context = nullptr;
	if (d->windows.size() > 0)
	{
		IWindow* window = d->windows.begin()->get();
		context = gm_cast<const GMXRenderContext*>(window->getContext());
	}

	XEvent e;
	while (1)
	{
		if (context)
		{
			XNextEvent(context->getDisplay(), &e);
			IWindow* window = findWindow(d->windows, e.xany.window);
			if (window)
			{
				GMXEventContext c = { &e, window };
				window->getProcHandler()(window->getWindowHandle(), 0, 0, reinterpret_cast<GMLParam>(&c));
			}
		}

		if (!renderFrame())
			break;
	}
	terminate();
}

void GameMachine::translateSystemEvent(GMuint uMsg, GMWParam wParam, GMLParam lParam, OUT GMSystemEvent** event)
{
}

void GameMachine::initSystemInfo()
{
	D(d);
	static bool inited = false;
	if (!inited)
	{
		d->states.systemInfo.endiannessMode = getMachineEndianness();
		inited = true;

		// set locale
		char* result = setlocale(LC_ALL, "zh_CN.utf8");
		if (!result)
		{
			gm_error(gm_dbg_wrap("Set locale failed. Type 'sudo apt-get install language-pack-zh-hans' to install ch_ZN locale pack."));
		}
	}
}