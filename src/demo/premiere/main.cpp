#include <iostream>
#include <gamemachine.h>
#include <wrapper.h>
#include "handler.h"

namespace
{
	void loadPackage()
	{
		GMGamePackage* pk = GM.getGamePackageManager();
#if GM_WINDOWS
#	ifdef GM_DEBUG
		pk->loadPackage("D:/gmpk");
#	else
		pk->loadPackage((GMPath::getCurrentPath() + L"gm.pk0"));
#	endif
#else
#	ifdef GM_DEBUG
		pk->loadPackage("/home/froser/Documents/gmpk");
#	else
		pk->loadPackage((GMPath::getCurrentPath() + L"gm.pk0"));
#	endif
#endif
	}

	Config setup()
	{
		gm::GMXMLDocument doc;
		gm::GMGamePackage* pk = GM.getGamePackageManager();
		gm::GMBuffer configBuffer;
		pk->readFile(GMPackageIndex::Scripts, "config", &configBuffer);

		Config config;
		if (configBuffer.getSize() > 0)
		{
			configBuffer.convertToStringBuffer();
			GMString contentStr = (char*)configBuffer.getData();
			std::string content = contentStr.toStdString();
			if (gm::GMXMLError::XML_SUCCESS == doc.Parse(content.c_str()))
			{
				auto root = doc.RootElement();
				if (GMString::stringEquals(root->Name(), "config"))
				{
					auto e = root->FirstChildElement();
					while (e)
					{
						GMString tag = e->Name();
						GMString text = e->GetText();
						if (tag == "windowName")
							config.windowName = text;
						else if (tag == "sample")
							config.samples = GMString::parseInt(text);
						else if (tag == "windowWidth")
							config.windowWidth = GMString::parseInt(text);
						else if (tag == "windowHeight")
							config.windowHeight = GMString::parseInt(text);
						else if (tag == "font")
						{
							GMString lang = e->Attribute("lang");
							if (lang == "CN")
								config.fontCN = text;
							else
								config.fontEN = text;
						}

						e = e->NextSiblingElement();
					}
				}
				else
				{
					gm_warning(gm_dbg_wrap("Wrong config root tag: {0}"), GMString(root->Name()));
				}
			}
		}
		return config;
	}
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

	gm::GMGameMachineDesc desc;
	desc.factory = factory;
	desc.renderEnvironment = env;
	GM.init(desc);

	loadPackage();
	Config config = setup();

	gm::GMWindowDesc windowDesc;
	windowDesc.instance = hInstance;
	windowDesc.windowName = config.windowName;;
	if (config.samples > 0)
		windowDesc.samples = config.samples;
	windowDesc.rc = { 0, 0, (gm::GMfloat) config.windowWidth, (gm::GMfloat) config.windowHeight };

#if GM_WINDOWS
	windowDesc.dwStyle = WS_OVERLAPPEDWINDOW | WS_SIZEBOX;
#endif

	gm::IWindow* mainWindow = nullptr;
	factory->createWindow(hInstance, nullptr, &mainWindow);
	mainWindow->create(windowDesc);
	mainWindow->centerWindow();
	mainWindow->showWindow();
	mainWindow->setHandler(new Handler(mainWindow, std::move(config)));
	GM.addWindow(mainWindow);

	GM.startGameMachine();
	return 0;
}
