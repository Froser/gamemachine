#include "stdafx.h"
#include "gamemachine.h"
#include "gmdatacore/glyph/gmglyphmanager.h"
#include "gmengine/gmgameobject.h"
#include "gmconfig.h"
#include "gmui/gmui.h"

#if _WINDOWS
#	include "os/gmdirectsound_sounddevice.h"
#endif

void GameMachine::init(
	GMUIInstance instance,
	AUTORELEASE IFactory* factory,
	AUTORELEASE IGameHandler* gameHandler
)
{
	D(d);
	d->instance = instance;
	GMUIPainter::SetInstance(d->instance);

	defaultMainWindowAttributes();

	d->factory.reset(factory);
	d->gameHandler.reset(gameHandler);

	GMUIWindow* window;
	d->factory->createWindow(&window);
	d->mainWindow.reset(window);

	IGraphicEngine* engine;
	d->factory->createGraphicEngine(&engine);
	d->engine.reset(engine);

	d->gamePackageManager.reset(new GMGamePackage(factory));
	d->inputManager.reset(new GMInput());
	d->configManager.reset(new GMConfig());

	initInner();
}

void GameMachine::setMainWindowAttributes(const GMUIWindowAttributes& attrs)
{
	D(d);
	d->mainWindowAttributes = attrs;
}

IGraphicEngine* GameMachine::getGraphicEngine()
{
	D(d);
	return d->engine;
}

GMUIWindow* GameMachine::getMainWindow()
{
	D(d);
	return d->mainWindow;
}

IFactory* GameMachine::getFactory()
{
	D(d);
	return d->factory;
}

GMUIWindow* GameMachine::appendWindow(AUTORELEASE GMUIWindow* window, const GMUIWindowAttributes& attrs)
{
	D(d);
	d->windows.push_back(makePair(window, attrs) );
	return window;
}

const GameMachineWindows& GameMachine::getWindows()
{
	D(d);
	return d->windows;
}

void GameMachine::postMessage(GameMachineMessage msg)
{
	D(d);
	d->messageQueue.push(msg);
}

GMConfig* GameMachine::getConfigManager()
{
	D(d);
	return d->configManager;
}

GMGlyphManager* GameMachine::getGlyphManager()
{
	D(d);
	return d->glyphManager;
}

GMGamePackage* GameMachine::getGamePackageManager()
{
	D(d);
	return d->gamePackageManager;
}

GMfloat GameMachine::getFPS()
{
	D(d);
	return d->clock.getFps();
}

GMfloat GameMachine::evaluateDeltaTime()
{
	D(d);
	return d->clock.evaluateDeltaTime();
}

GMfloat GameMachine::getGameTimeSeconds()
{
	D(d);
	return d->clock.getTime();
}

void GameMachine::initObjectPainter(GMGameObject* obj)
{
	D(d);
	ObjectPainter* painter;
	getFactory()->createPainter(getGraphicEngine(), obj->getObject(), &painter);
	ASSERT(!obj->getObject()->getPainter());
	obj->getObject()->setPainter(painter);
	painter->transfer();
}

GMInput* GameMachine::getInputManager()
{
	D(d);
	return d->inputManager;
}

GameMachine::EndiannessMode GameMachine::getMachineEndianness()
{
	static EndiannessMode ret = UNKNOWN_YET;
	if (ret == UNKNOWN_YET)
	{
		long int i = 1;
		const char *p = (const char *)&i;
		if (p[0] == 1)
			ret = LITTLE_ENDIAN;
		else
			ret = BIG_ENDIAN;
	}
	return ret;
}

void GameMachine::startGameMachine()
{
	D(d);
	// 创建主窗口
	d->mainWindow->create(d->mainWindowAttributes);
	d->mainWindow->centerWindow();
	d->mainWindow->showWindow();

	// 创建其他窗口
	createWindows();

#if _WINDOWS
	// 创建声音设备
	GMSoundPlayerDevice::createInstance(d->mainWindow);
#endif

	// 创建Glyph管理器
	GMGlyphManager* glyphManager;
	d->factory->createGlyphManager(&glyphManager);
	d->glyphManager.reset(glyphManager);

	// 初始化gameHandler
	d->gameHandler->init();

	// 开始多线程工作
	d->simulateJob.setHandler(d->gameHandler);
	d->simulateJob.start();

	d->clock.begin();
	// 消息循环
	while (true)
	{
		if (!GMUIWindow::handleMessage())
			break;

		if (!handleMessages())
			break;
		
		if (d->gameHandler->isWindowActivate())
			d->gameHandler->event(GM_EVENT_ACTIVATE);

		{
			gmRunSustainedThread (simulateJob, &d->simulateJob);
			d->gameHandler->event(GM_EVENT_RENDER);
			d->mainWindow->swapBuffers();
		}
		d->clock.update();
	}
}

bool GameMachine::handleMessages()
{
	D(d);
	GameMachineMessage msg;
	while (d->messageQueue.size() > 0)
	{
		msg = d->messageQueue.back();

		switch (msg.msgType)
		{
		case gm::GM_MESSAGE_EXIT:
			return false;
		case gm::GM_MESSAGE_CONSOLE:
			{
				d->consoleWindow->centerWindow();
				d->consoleWindow->showWindow(true, true);
			}
		default:
			break;
		}

		d->messageQueue.pop();
	}
	return true;
}

void GameMachine::defaultMainWindowAttributes()
{
	D(d);
	GMUIWindowAttributes attrs =
	{
		NULL,
		L"DefaultGameMachineWindow",
		0,
		0,
		{ 0, 0, 700, 400 },
		NULL,
		d->instance,
	};
	setMainWindowAttributes(attrs);
}

void GameMachine::createWindows()
{
	D(d);
	for (auto& window : d->windows)
	{
		ASSERT(window.first);
		GMUIWindowAttributes attrs = window.second;
		attrs.hwndParent = d->mainWindow->getWindowHandle();
		window.first->create(attrs);
	}
}

void GameMachine::initInner()
{
	D(d);
	GMUIWindowAttributes attrs =
	{
		NULL,
		L"GameMachineConsoleWindow",
		WS_OVERLAPPEDWINDOW,
		WS_EX_CLIENTEDGE,
		{ 0, 0, 700, 400 },
		NULL,
		d->instance,
	};
	GMUIConsole::newConsoleWindow(&d->consoleWindow);
	appendWindow(d->consoleWindow, attrs);
	GMDebugger::setDebugOuput(d->consoleWindow);
}