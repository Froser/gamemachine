#include "stdafx.h"
#include <gmui.h>
#include "gmuidef.h"

// factory
void GMUIFactory::createMainWindow(gm::GMInstance instance, OUT gm::IWindow** window)
{
	initEnvironment(instance);
	(*window) = new GMUIGLWindow();
}

void GMUIFactory::createConsoleWindow(gm::GMInstance instance, OUT gm::IWindow** window)
{
	initEnvironment(instance);
	(*window) = new GMUIConsole();
}

void GMUIFactory::initEnvironment(gm::GMInstance instance)
{
	GMUIPainter::SetInstance(instance);
}
