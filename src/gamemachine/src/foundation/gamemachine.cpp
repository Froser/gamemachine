﻿#include "stdafx.h"
#include "gamemachine.h"
#include "gmdata/glyph/gmglyphmanager.h"
#include "gmengine/gameobjects/gmgameobject.h"
#include "gmstates.h"

void GameMachine::init(
	AUTORELEASE IWindow* mainWindow,
	const GMConsoleHandle& consoleHandle,
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
	registerManager(new GMGamePackage(), &d->gamePackageManager);
	registerManager(new GMStates(), &d->statesManager);
	registerManager(consoleHandle.window, &d->consoleWindow);
	d->consoleOutput = consoleHandle.dbgoutput;

	updateGameMachineRunningStates();
	initInner();
	d->gameHandler->init();
}

void GameMachine::postMessage(GameMachineMessage msg)
{
	D(d);
	d->messageQueue.push(msg);
}

GameMachineMessage GameMachine::peekMessage()
{
	D(d);
	return d->lastMessage;
}

void GameMachine::createModelPainterAndTransfer(GMModel* model)
{
	if (model)
	{
		GMModelPainter* painter = model->getPainter();
		if (!painter)
		{
			getFactory()->createPainter(getGraphicEngine(), model, &painter);
			model->setPainter(painter);
		}
		painter->transfer();
	}
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

	// 创建Glyph管理器，它必须在OpenGL窗口创建以后才可以初始化
	GMGlyphManager* glyphManager;
	d->factory->createGlyphManager(&glyphManager);
	registerManager(glyphManager, &d->glyphManager);

	// 更新一次状态
	updateGameMachineRunningStates();

	// 开始渲染
	d->engine->init();

	// 处理一次消息
	handleMessages();

	// 初始化gameHandler
	if (!d->states.crashDown)
		d->gameHandler->start();

	// 开始计时器
	d->clock.begin();

	// 消息循环
	runLoop();

	// 结束
	terminate();
}

void GameMachine::runLoop()
{
	D(d);
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

		if (d->states.crashDown)
		{
			// 宕机的情况下，只更新下面的状态
			d->mainWindow->update();
			d->consoleWindow->update();
			d->clock.update();
			continue;
		}

		d->gameHandler->event(GameMachineEvent::FrameStart);
		if (d->mainWindow->isWindowActivate())
			d->gameHandler->event(GameMachineEvent::Activate);
		else
			d->gameHandler->event(GameMachineEvent::Deactivate);
		d->gameHandler->event(GameMachineEvent::Simulate);
		d->gameHandler->event(GameMachineEvent::Render);

		// 更新所有管理器
		d->mainWindow->update();
		if (d->consoleWindow)
			d->consoleWindow->update();
		d->clock.update();

		// 更新状态
		updateGameMachineRunningStates();

		// 本帧结束
		d->gameHandler->event(GameMachineEvent::FrameEnd);

		// 控制帧率
		if (bNeedControlFrameRate)
		{
			static const GMfloat ms = 1 / 60.f;
			GMfloat elapsedFromStart = frameCounter.elapsedFromStart();
			diff = (ms - elapsedFromStart) * 1000;
			if (diff > 0)
			{
				GMThread::sleep(diff);
				d->states.lastFrameElpased = ms;
			}
			else
			{
				d->states.lastFrameElpased = frameCounter.elapsedFromStart();
			}
		}
		else
		{
			d->states.lastFrameElpased = frameCounter.elapsedFromStart();
		}
	}

}

bool GameMachine::handleMessages()
{
	D(d);
	GameMachineMessage msg;
	while (d->messageQueue.size() > 0)
	{
		msg = d->messageQueue.front();

		switch (msg.msgType)
		{
		case GameMachineMessageType::Quit:
			return false;
		case GameMachineMessageType::Console:
		{
			if (d->consoleWindow)
				d->consoleWindow->event(msg);
			break;
		}
		case GameMachineMessageType::CrashDown:
		{
			d->states.crashDown = true;
			break;
		}
		default:
			break;
		}

		d->engine->event(msg);
		d->messageQueue.pop();
	}
	d->lastMessage = msg;
	return true;
}

void GameMachine::initInner()
{
	D(d);
	GMDebugger::setDebugOutput(d->consoleOutput);
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
		GM_delete(manager);
	}
}

void GameMachine::updateGameMachineRunningStates()
{
	D(d);
	d->states.clientRect = getMainWindow()->getClientRect();
	d->states.windowRect = getMainWindow()->getWindowRect();
}