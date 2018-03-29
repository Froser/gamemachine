#ifndef __GMUI_H__
#define __GMUI_H__
#include <gmuicommon.h>

#if GM_WINDOWS
#include "../windows/gmuiguiwindow.h"
#include "../windows/gmuiglwindow.h"
#include "../windows/gmuiwindow.h"

#if GM_USE_DX11
#include "../windows/gmuidx11window.h"
#endif

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
	static void createMainWindow(gm::GMInstance instance, OUT gm::IWindow** window, gm::GMRenderEnvironment env);
	static void createConsoleWindow(gm::GMInstance instance, REF gm::GMConsoleHandle& handle);
	static void createCursor(OUT gm::ICursor** cursor);

private:
	static void initEnvironment(gm::GMInstance instance);
};

END_UI_NS

#endif