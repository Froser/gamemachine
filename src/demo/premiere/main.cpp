#include <iostream>
#include <gamemachine.h>
#include <wrapper.h>
#include "handler.h"

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
#elif GM_UNIX
int main(int argc, char* argv[])
{	
#endif

#if GM_WINDOWS
	gm::GMsize_t sz = gm::GMString::countOfCharacters(cmdLine);
	gm::GMRenderEnvironment env = gm::GMRenderEnvironment::DirectX11;
	if (sz > 7 && gm::GMString(cmdLine + sz - 7) == "-opengl")
		env = gm::GMRenderEnvironment::OpenGL;
#elif GM_UNIX
	gm::GMRenderEnvironment env = gm::GMRenderEnvironment::OpenGL;
	gm::GMInstance hInstance = 0; // Useless in unix
#endif

	gm::IFactory* factory = nullptr;
	env = GMCreateFactory(env, gm::GMRenderEnvironment::OpenGL, &factory);
	GM_ASSERT(env != gm::GMRenderEnvironment::Invalid && factory);
	SetRenderEnv(env);

	gm::GMGameMachineDesc desc;
	desc.factory = factory;
	desc.renderEnvironment = GetRenderEnv();
	GM.init(desc);

	gm::GMWindowDesc windowDesc;
	windowDesc.instance = hInstance;
	windowDesc.windowName = L"GameMachine Premiere";

#if GM_WINDOWS
	windowDesc.dwStyle = WS_OVERLAPPEDWINDOW | WS_SIZEBOX;
#endif
	// windowDesc.samples = 1;

	gm::IWindow* mainWindow = nullptr;
	factory->createWindow(hInstance, nullptr, &mainWindow);
	mainWindow->create(windowDesc);
	mainWindow->centerWindow();
	mainWindow->showWindow();
	mainWindow->setHandler(new Handler(mainWindow));
	GM.addWindow(mainWindow);

	GM.startGameMachine();
	return 0;
}
