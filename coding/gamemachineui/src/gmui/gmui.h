#ifndef __GMUI_H__
#define __GMUI_H__
#include <gmuicommon.h>

#if _WINDOWS
#include "../windows/gmuiwindow.h"
#include "../windows/gmuiguiwindow.h"
#include "../windows/gmuiconsole.h"
#include "../windows/gmuiglwindow.h"
#endif

namespace gm
{
	struct IWindow;
	struct IDebugOutput;
}

BEGIN_UI_NS
//Factory
class GMUIFactory
{
public:
	static void createMainWindow(gm::GMInstance instance, OUT gm::IWindow** window);
	static void createConsoleWindow(gm::GMInstance instance, OUT gm::IWindow** window);

private:
	static void initEnvironment(gm::GMInstance instance);
};

END_UI_NS

#endif