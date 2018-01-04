#include "stdafx.h"
#include <gmui.h>
#include "gmuidef.h"

// factory
void GMUIFactory::createMainWindow(gm::GMInstance instance, OUT gm::IWindow** window)
{
	initEnvironment(instance);
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
