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

	registerManager(factory, &d->factory);
	registerManager(gameHandler, &d->gameHandler);

	GMUIWindow* window;
	d->factory->createWindow(&window);
	registerManager(window, &d->mainWindow);

	IGraphicEngine* engine;
	d->factory->createGraphicEngine(&engine);
	registerManager(engine, &d->engine);

	registerManager(new GMGamePackage(factory), &d->gamePackageManager);
	registerManager(new GMInput(), &d->inputManager);
	registerManager(new GMConfig(), &d->configManager);

	initInner();
	d->gameHandler->init();
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

void GameMachine::initObjectPainter(GMModel* obj)
{
	D(d);
	GMModelPainter* painter = obj->getPainter();
	if (!painter)
	{
		getFactory()->createPainter(getGraphicEngine(), obj, &painter);
		obj->setPainter(painter);
	}
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
	registerManager(glyphManager, &d->glyphManager);

	// 开始渲染
	d->engine->start();

	// 初始化gameHandler
	d->gameHandler->start();

	// 开始计时器
	d->clock.begin();
	// 消息循环

	GMfloat diff = 0;
	GMClock frameCounter;
	while (true)
	{
		GMint bNeedControlFrameRate = GMGetBuiltIn(FRAMERATE_CONTROL);
		if (bNeedControlFrameRate)
			frameCounter.begin();

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

		// 控制帧率
		if (bNeedControlFrameRate)
		{
			frameCounter.update();
			diff = (1 / 60.f - frameCounter.evaluateDeltaTime()) * 1000;
			if (diff > 0)
				GMThread::sleep(diff);
		}
	}

	terminate();
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

template <typename T, typename U>
void GameMachine::registerManager(T* newObject, OUT U** manager)
{
	D(d);
	*manager = newObject;
	d->manangerQueue.push_back(*manager);
}

void GameMachine::terminate()
{
	D(d);
	d->gameHandler->event(GameMachineEvent::Terminate);
	for (auto manager : d->manangerQueue)
	{
		delete manager;
	}
	GMSoundPlayerDevice::terminate();
}