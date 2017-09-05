#include "stdafx.h"
#include "gamemachine.h"
#include "gmdatacore/glyph/gmglyphmanager.h"
#include "gmengine/gmgameobject.h"
#include "gmstates.h"

#if _WINDOWS
#	include "os/gmdirectsound_sounddevice.h"
#endif

void GameMachine::init(
	AUTORELEASE IWindow* mainWindow,
	AUTORELEASE IWindow* consoleWindow,
	AUTORELEASE IFactory* factory,
	AUTORELEASE IGameHandler* gameHandler
)
{
	D(d);
	registerManager(factory, &d->factory);
	registerManager(gameHandler, &d->gameHandler);
	registerManager(mainWindow, &d->mainWindow);

	IGraphicEngine* engine;
	d->factory->createGraphicEngine(&engine);
	registerManager(engine, &d->engine);

	registerManager(new GMGamePackage(factory), &d->gamePackageManager);
	registerManager(new GMInput(), &d->inputManager);
	registerManager(new GMStates(), &d->statesManager);
	registerManager(consoleWindow, &d->consoleWindow);

	initInner();
	d->gameHandler->init();
}

void GameMachine::postMessage(GameMachineMessage msg)
{
	D(d);
	d->messageQueue.push(msg);
}

void GameMachine::initObjectPainter(GMModel* obj)
{
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
	// 显示主窗口
	d->mainWindow->centerWindow();
	d->mainWindow->showWindow();

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
		GMint bNeedControlFrameRate = GMGetDebugState(FRAMERATE_CONTROL);
		if (bNeedControlFrameRate)
			frameCounter.begin();

		if (!d->mainWindow->handleMessage())
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
		d->consoleWindow->update();
		d->inputManager->update();
		d->clock.update();

		// 控制帧率
		if (bNeedControlFrameRate)
		{
			static const GMfloat ms = 1 / 60.f;
			GMfloat elapsedFromStart = frameCounter.elapsedFromStart();
			diff = (ms - elapsedFromStart) * 1000;
			if (diff > 0)
			{
				GMThread::sleep(diff);
				d->lastFrameElpased = ms;
			}
			else
			{
				d->lastFrameElpased = frameCounter.elapsedFromStart();
			}
		}
		else
		{
			d->lastFrameElpased = frameCounter.elapsedFromStart();
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
		case GameMachineMessageType::Quit:
			return false;
		case GameMachineMessageType::Console:
			{
				if (d->consoleWindow)
					d->consoleWindow->event(msg);
			}
		default:
			break;
		}

		d->engine->event(msg);
		d->messageQueue.pop();
	}
	return true;
}

void GameMachine::initInner()
{
	D(d);
	GMDebugger::setDebugOutput(gminterface_cast<IDebugOutput*>(d->consoleWindow));
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