#include "stdafx.h"
#include "src/shell/application.h"
#include "src/shell/mainwindow.h"

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
	mainWindow.show();

	app.exec();
	return 0;
}
