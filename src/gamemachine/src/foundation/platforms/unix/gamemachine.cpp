#include "stdafx.h"
#include "check.h"
#include "../../gamemachine.h"
#include <locale.h>
#include <X11/Xlib.h>
#include "window/gmxrendercontext.h"
#include "foundation/gamemachine_p.h"

BEGIN_NS
namespace
{
	void getSystemInfo(GMSystemInfo& si)
	{
		static char s_line[1024] = { 0 };
		FILE* fp = fopen ("/proc/cpuinfo", "r");

		int processors = 0;
		if (fp)
		{
			while (!feof(fp))
			{
				if ( !fgets (s_line, sizeof (s_line), fp))
					continue;

				GMString s(s_line);
				if (s.startsWith(L"processor"))
				{
					++processors;
				}
				else if (s.startsWith(L"vendor_id"))
				{
					if (s.endsWith(L"GenuineIntel\n"))
						si.processorArchitecture = GMProcessorArchitecture::Intel;
					else if (s.endsWith(L"AuthenticAMD\n"))
						si.processorArchitecture = GMProcessorArchitecture::AMD64;
				}

			}
			si.numberOfProcessors = processors;
		}
		else
		{
			si.numberOfProcessors = 1;
			si.processorArchitecture = GMProcessorArchitecture::Unknown;
		}
	}

	GMEndiannessMode getMachineEndianness()
	{
		long int i = 1;
		const char *p = (const char *)&i;
		if (p[0] == 1)
			return GMEndiannessMode::LittleEndian;
		return GMEndiannessMode::BigEndian;
	}

	IWindow* findWindow(const Set<IWindow*>& windows, Window window)
	{
		for (const auto& w : windows)
		{
			if (w->getWindowHandle() == window)
				return w;
		}
		return nullptr;
	}
}

void GameMachinePrivate::runEventLoop()
{
	P_D(pd);
	const GMXRenderContext* context = nullptr;
	if (windows.size() > 0)
	{
		IWindow* window = *windows.begin();
		context = gm_cast<const GMXRenderContext*>(window->getContext());
	}

	XEvent e;
	while (1)
	{
		if (context)
		{
			Display* display = context->getDisplay();
			while (XPending(display))
			{
				XNextEvent(display, &e);
				IWindow* window = findWindow(windows, e.xany.window);
				if (window)
				{
					GMXEventContext c = { &e, window };
					window->getProcHandler()(window->getWindowHandle(), 0, 0, reinterpret_cast<GMLParam>(&c));
				}
			}
		}

		if (runningMode == GMGameMachineRunningMode::GameMode)
		{
			if (!pd->renderFrame())
				break;
		}

		// Application模式下，虽然不进行渲染，但是还是有消息处理
		if (!pd->handleMessages())
			break;
	}
	pd->finalize();
}

void GameMachinePrivate::initSystemInfo()
{
	static bool inited = false;
	if (!inited)
	{
		states.systemInfo.endiannessMode = getMachineEndianness();
		getSystemInfo(states.systemInfo);
		inited = true;

		// set locale
		char* result = setlocale(LC_ALL, "zh_CN.utf8");
		if (!result)
		{
			gm_error(gm_dbg_wrap("Set locale failed. Type 'sudo apt-get install language-pack-zh-hans' to install ch_ZN locale pack."));
		}
	}
}
END_NS