#include <iostream>
#include <windows.h>
#include <fstream>
#include <gamemachine.h>
#include <gmgl.h>
#include <gmui.h>
#include <gmm.h>
#include "demostration_world.h"


gm::GMGLFactory factory;

int WINAPI WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	char * lpCmdLine,
	int nCmdShow
)
{
	gm::GMWindowAttributes mainAttrs;
	mainAttrs.instance = hInstance;

	gm::IWindow* mainWindow = nullptr;
	gmui::GMUIFactory::createMainWindow(hInstance, &mainWindow);
	mainWindow->create(mainAttrs);

	gm::GMWindowAttributes consoleAttrs;
	consoleAttrs.windowName = L"GameMachineConsoleWindow";
	consoleAttrs.dwStyle = WS_OVERLAPPED | WS_CAPTION | WS_THICKFRAME | WS_SYSMENU;
	consoleAttrs.dwExStyle = WS_EX_CLIENTEDGE;
	consoleAttrs.rc.right = 700;
	consoleAttrs.rc.bottom = 400;

	gm::GMConsoleHandle consoleHandle;
	gmui::GMUIFactory::createConsoleWindow(hInstance, consoleHandle);
	if (consoleHandle.window)
		consoleHandle.window->create(consoleAttrs);

	GM.init(
		mainWindow,
		consoleHandle,
		new gm::GMGLFactory(),
		new DemostrationEntrance()
	);

	GM.startGameMachine();
	return 0;
}