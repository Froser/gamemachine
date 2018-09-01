#include <iostream>
#if GM_WINDOWS
#include <windows.h>
#endif
#include <fstream>
#include <gamemachine.h>
#include <gmm.h>
#include "demonstration_world.h"
#include <gmdx11.h>
#include <gmgl.h>

namespace
{
	gm::GMRenderEnvironment s_env;

	void SetRenderEnv(gm::GMRenderEnvironment env)
	{
		s_env = env;
	}
}

gm::GMRenderEnvironment GetRenderEnv()
{
	return s_env;
}

#if GM_WINDOWS
int WINAPI wWinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPWSTR lpCmdLine,
	int nCmdShow
)
{
	LPWSTR cmdLine = GetCommandLineW();
	gm::GMsize_t sz = gm::GMString::countOfCharacters(cmdLine);
	gm::GMRenderEnvironment env = gm::GMRenderEnvironment::DirectX11;
	if (sz > 7 && gm::GMString(cmdLine + sz - 7) == "-opengl")
		env = gm::GMRenderEnvironment::OpenGL;
	SetRenderEnv(env);

	gm::IFactory* factory = nullptr;
	if (GetRenderEnv() == gm::GMRenderEnvironment::OpenGL)
	{
		factory = new gm::GMGLFactory();
	}
	else
	{
#if GM_USE_DX11
		if (gm::GMQueryCapability(gm::GMCapability::SupportDirectX11))
			factory = new gm::GMDx11Factory();
		else
			factory = new gm::GMGLFactory();
#else
		SetRenderEnv(gm::GMRenderEnvironment::OpenGL);
		factory = new gm::GMGLFactory();
#endif
	}

	gm::GMWindowAttributes mainAttrs;
	mainAttrs.instance = hInstance;
	//mainAttrs.samples = 1;

	gm::IWindow* mainWindow = nullptr;
	factory->createWindow(hInstance, nullptr, &mainWindow);
	mainWindow->create(mainAttrs);
	mainWindow->centerWindow();
	mainWindow->showWindow();
	mainWindow->setHandler(new DemostrationEntrance(mainWindow));
	GM.addWindow(mainWindow);

	/*
	gm::IWindow* mainWindow2 = nullptr;
	factory->createWindow(hInstance, mainWindow, &mainWindow2);
	mainWindow2->create(mainAttrs);
	mainWindow2->centerWindow();
	mainWindow2->showWindow();
	mainWindow2->setHandler(new DemostrationEntrance(mainWindow2));
	GM.addWindow(mainWindow2);
	*/

	GM.init(
		factory,
		GetRenderEnv()
	);

	GM.startGameMachine();
	return 0;
}
#endif