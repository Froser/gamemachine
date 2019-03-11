#include "stdafx.h"
#include "src/shell/application.h"
#include "src/shell/mainwindow.h"

#include <gmgl.h>
#if GM_USE_DX11
#include <gmdx11.h>
#include <gmdx11helper.h>
#endif

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

	const QStringList& args = app.arguments();
	GMRenderEnvironment re = GMRenderEnvironment::DirectX11;
	IFactory* factory = nullptr;
	if (args.contains("-opengl"))
	{
		factory = new GMGLFactory();
		re = GMRenderEnvironment::OpenGL;
	}
	else
	{
		factory = new GMDx11Factory();
		re = GMRenderEnvironment::DirectX11;
	}

	GMGameMachineDesc desc;
	desc.factory = factory;
	desc.renderEnvironment = re;
	desc.runningMode = GMGameMachineRunningMode::ApplicationMode;
	mainWindow.initGameMachine(desc);
	mainWindow.show();

	app.exec();
	return 0;
}
