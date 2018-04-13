#include "stdafx.h"
#include "gamemachine.h"
#include "gmdata/glyph/gmglyphmanager.h"
#include "gmengine/gameobjects/gmgameobject.h"
#include "gmconfigs.h"

extern "C"
{
	bool GMQueryCapability(GMCapability cp)
	{
		if (cp == GMCapability::SupportOpenGL)
			return true;

		if (cp == GMCapability::SupportDirectX11)
#if GM_USE_DX11
			return true;
#else
			return false;
#endif
		GM_ASSERT(false);
		return false;
	}
}

GameMachine::GameMachine()
{
	getMachineEndianness();
	updateGameMachineRunningStates();
}

void GameMachine::init(
	AUTORELEASE IWindow* mainWindow,
	const GMConsoleHandle& consoleHandle,
	AUTORELEASE IFactory* factory,
	AUTORELEASE IGameHandler* gameHandler,
	GMRenderEnvironment renderEnv
)
{
	D(d);
	setRenderEnvironment(renderEnv);
	d->camera.reset(new GMCamera());
	registerManager(factory, &d->factory);
	registerManager(mainWindow, &d->mainWindow);
	IGraphicEngine* engine;
	d->factory->createGraphicEngine(&engine);
	registerManager(engine, &d->engine);
	registerManager(gameHandler, &d->gameHandler);
	registerManager(new GMGamePackage(), &d->gamePackageManager);
	registerManager(new GMConfigs(), &d->statesManager);
	registerManager(consoleHandle.window, &d->consoleWindow);

	d->consoleOutput = consoleHandle.dbgoutput;
	GMDebugger::setDebugOutput(d->consoleOutput);

	handleMessages();
	updateGameMachineRunningStates();
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

GMEndiannessMode GameMachine::getMachineEndianness()
{
	D(d);
	if (d->states.endiannessMode == GMEndiannessMode::Unknown)
	{
		long int i = 1;
		const char *p = (const char *)&i;
		if (p[0] == 1)
			d->states.endiannessMode = GMEndiannessMode::LittleEndian;
		else
			d->states.endiannessMode = GMEndiannessMode::BigEndian;
	}
	return d->states.endiannessMode;
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

	// 初始化gameHandler
	if (!d->states.crashDown)
		d->gameHandler->start();
	else
		terminate();

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
	GMClock frameCounter;
	while (true)
	{
		// 记录帧率
		frameCounter.begin();

		// 接收窗口消息
		if (!d->mainWindow->handleMessage())
			break;

		// 处理GameMachine消息
		if (!handleMessages())
			break;

		// 检查是否崩溃
		if (checkCrashDown())
			continue;

		// 调用Handler
		handlerEvents();

		// 更新所有管理器
		updateManagers();

		// 更新状态
		updateGameMachineRunningStates();

		// 本帧结束
		d->gameHandler->event(GameMachineEvent::FrameEnd);
		d->states.lastFrameElpased = frameCounter.elapsedFromStart();
	}
}

void GameMachine::setRenderEnvironment(GMRenderEnvironment renv)
{
	D(d);
	switch (renv)
	{
	case GMRenderEnvironment::OpenGL:
		if (GMQueryCapability(GMCapability::SupportOpenGL))
		{
			d->states.renderEnvironment = renv;
			return;
		}
	case GMRenderEnvironment::DirectX11:
		if (GMQueryCapability(GMCapability::SupportDirectX11))
		{
			d->states.renderEnvironment = renv;
			return;
		}
		break;
	default:
		break;
	}
	GM_ASSERT(!"Wrong render environment");
	d->states.renderEnvironment = GMRenderEnvironment::OpenGL;
}

bool GameMachine::checkCrashDown()
{
	D(d);
	if (d->states.crashDown)
	{
		// 宕机的情况下，只更新下面的状态
		d->mainWindow->update();
		d->consoleWindow->update();
		d->clock.update();
		return true;
	}
	return false;
}

void GameMachine::handlerEvents()
{
	D(d);
	d->gameHandler->event(GameMachineEvent::FrameStart);
	if (d->mainWindow->isWindowActivate())
		d->gameHandler->event(GameMachineEvent::Activate);
	else
		d->gameHandler->event(GameMachineEvent::Deactivate);
	d->gameHandler->event(GameMachineEvent::Simulate);
	d->gameHandler->event(GameMachineEvent::Render);
}

void GameMachine::updateManagers()
{
	D(d);
	d->mainWindow->update();
	if (d->consoleWindow)
		d->consoleWindow->update();
	d->clock.update();
}

bool GameMachine::handleMessages()
{
	D(d);
	GameMachineMessage msg;
	while (d->messageQueue.size() > 0)
	{
		msg = d->messageQueue.front();

		if (!handleMessage(msg))
			return false;

		d->messageQueue.pop();
	}
	d->lastMessage = msg;
	return true;
}

bool GameMachine::handleMessage(const GameMachineMessage& msg)
{
	D(d);
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
	return true;
}

template <typename T, typename U>
void GameMachine::registerManager(T* newObject, OUT U** manager)
{
	D(d);
	*manager = newObject;
	d->managerQueue.push_back(*manager);
}

void GameMachine::terminate()
{
	D(d);
	d->gameHandler->event(GameMachineEvent::Terminate);
	for (auto iter = d->managerQueue.rbegin(); iter != d->managerQueue.rend(); ++iter)
	{
		GM_delete(*iter);
	}
}

void GameMachine::updateGameMachineRunningStates()
{
	D(d);
	gm::IWindow* mainWindow = getMainWindow();
	if (mainWindow)
	{
		d->states.clientRect = mainWindow->getClientRect();
		d->states.windowRect = mainWindow->getWindowRect();
	}
}