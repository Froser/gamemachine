#include "stdafx.h"
#include <gmui.h>
#include "gmuidef.h"

#if GM_USE_DUILIB
#include "duilib/gmuiconsole.h"
#endif

// factory
void GMUIFactory::createMainWindow(gm::GMInstance instance, OUT gm::IWindow** window, gm::GMRenderEnvironment env)
{
	initEnvironment(instance);
	if (env == gm::GMRenderEnvironment::DirectX11)
	{
#if GM_USE_DX11
		if (gm::GMQueryCapability(gm::GMCapability::SupportDirectX11))
		{
			(*window) = new GMUIDx11Window();
			return;
		}
		GM_ASSERT(!"DirectX11 not support, use opengl instead");
#else
		GM_ASSERT(!"DirectX11 not support, use opengl instead");
#endif
	}
	(*window) = new GMUIGLWindow();
}

void GMUIFactory::createConsoleWindow(gm::GMInstance instance, REF gm::GMConsoleHandle& handle)
{
#if GM_USE_DUILIB
	initEnvironment(instance);
	GMUIConsole* console = new GMUIConsole();
	handle.window = console;
	handle.dbgoutput = console;
#else
	handle.window = nullptr;
	handle.dbgoutput = nullptr;
#endif
}

void GMUIFactory::initEnvironment(gm::GMInstance instance)
{
#if GM_USE_DUILIB
	DuiLib::CPaintManagerUI::SetInstance(instance);
#endif
}