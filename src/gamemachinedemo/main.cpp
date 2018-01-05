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
	gm::GMWindowAttributes mainAttrs =
	{
		NULL,
		L"Default",
		WS_OVERLAPPED | WS_CAPTION | WS_THICKFRAME,
		0,
		{ 0, 0, 800, 600 },
		NULL,
		hInstance,
	};

	gm::IWindow* mainWindow = nullptr;
	gmui::GMUIFactory::createMainWindow(hInstance, &mainWindow);
	mainWindow->create(mainAttrs);

	gm::GMWindowAttributes consoleAttrs =
	{
		NULL,
		L"GameMachineConsoleWindow",
		WS_OVERLAPPED | WS_CAPTION | WS_THICKFRAME | WS_SYSMENU,
		WS_EX_CLIENTEDGE,
		{ 0, 0, 700, 400 },
		NULL,
	};

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