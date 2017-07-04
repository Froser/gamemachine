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

GMUIWindow* GameMachine::appendWindow(AUTORELEASE GMUIWindow* window, const GMUIWindowAttributes& attrs)
{
	D(d);
	d->windows.push_back(makePair(window, attrs) );
	return window;
}

void GameMachine::postMessage(GameMachineMessage msg)
{
	D(d);
	d->messageQueue.push(msg);
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

	// 创建Glyph管理器，它必须在OpenGL窗口创建以后才可以初始化
	GMGlyphManager* glyphManager;
	d->factory->createGlyphManager(&glyphManager);
	d->glyphManager.reset(glyphManager);

	// 开始渲染
	d->engine->start();

	// 初始化gameHandler
	d->gameHandler->start();

	// 开始计时器
	d->clock.begin();
	// 消息循环
	while (true)
	{
		if (!GMUIWindow::handleMessage())
			break;

		if (!handleMessages())
			break;
		
		d->gameHandler->event(GameMachineEvent::FrameStart);

		if (d->gameHandler->isWindowActivate())
			d->gameHandler->event(GameMachineEvent::Activate);
		else
			d->gameHandler->event(GameMachineEvent::Deactivate);

		d->gameHandler->event(GameMachineEvent::Simulate);
		d->gameHandler->event(GameMachineEvent::Render);
		d->mainWindow->swapBuffers();

		d->gameHandler->event(GameMachineEvent::FrameEnd);

		// 更新所有管理器
		updateWindows();
		d->inputManager->update();
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
		case gm::GM_MESSAGE_WINDOW_SIZE:
			d->engine->setViewport(d->mainWindow->getClientRect());
			break;
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
		WS_OVERLAPPEDWINDOW,
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

void GameMachine::updateWindows()
{
	D(d);
	for (auto& window : d->windows)
	{
		window.first->update();
	}
}

void GameMachine::initInner()
{
	D(d);
	GMUIWindowAttributes attrs =
	{
		NULL,
		L"GameMachineConsoleWindow",
		WS_OVERLAPPED | WS_CAPTION | WS_THICKFRAME | WS_SYSMENU,
		WS_EX_CLIENTEDGE,
		{ 0, 0, 700, 400 },
		NULL,
		d->instance,
	};
	GMUIConsole::newConsoleWindow(&d->consoleWindow);
	appendWindow(d->consoleWindow, attrs);
	GMDebugger::setDebugOutput(d->consoleWindow);
}