#include "stdafx.h"
#include "src/shell/application.h"
#include "src/shell/mainwindow.h"

#include <gmgl.h>
#include <wrapper.h>

#if GM_WINDOWS
int WINAPI wWinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPWSTR lpCmdLine,
	int nCmdShow
)
{
	shell::Application app(__argc, __argv);
#elif GM_UNIX
int main(int argc, char* argv[])
{	
	shell::Application app(argc, argv);
#endif
	shell::MainWindow mainWindow;

	GMRenderEnvironment re = GMRenderEnvironment::DirectX11;
	IFactory* factory = nullptr;
	if (wcscmp(lpCmdLine, L"-opengl") == 0)
	{
		re = GMRenderEnvironment::OpenGL;
	}
	else
	{
		re = GMRenderEnvironment::DirectX11;
	}

	re = GMCreateFactory(re, GMRenderEnvironment::OpenGL, &factory);

	GMGameMachineDesc desc;
	desc.factory = factory;
	desc.renderEnvironment = re;
	desc.runningMode = GMGameMachineRunningMode::ApplicationMode;
	mainWindow.initGameMachine(desc);
	mainWindow.show();

	app.exec();
	return 0;
}
