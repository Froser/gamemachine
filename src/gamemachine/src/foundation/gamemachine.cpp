#include "stdafx.h"
#include "gamemachine.h"
#include "gmdata/glyph/gmglyphmanager.h"
#include "gmengine/gameobjects/gmgameobject.h"
#include "gmconfigs.h"
#include "gmengine/ui/gmwidget.h"
#include "gmmessage.h"

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

namespace
{
	static GMMessage s_frameUpdateMsg(GameMachineMessageType::FrameUpdate);
}

GameMachine::GameMachine()
{
	getMachineEndianness();
	updateGameMachineRunningStates();
}

GameMachine::~GameMachine()
{
	D(d);
	while (!d->windows.empty())
	{
		auto iter = d->windows.begin();
		auto window = *iter;
		GM_delete(window);
	}
}

void GameMachine::init(
	IWindow* mainWindow,
	AUTORELEASE IFactory* factory,
	GMRenderEnvironment renderEnv
)
{
	D(d);
	addWindow(mainWindow);
	d->mainWindow = mainWindow;

	setRenderEnvironment(renderEnv);
	d->camera.reset(new GMCamera());
	registerManager(factory, &d->factory);
	registerManager(new GMGamePackage(), &d->gamePackageManager);
	registerManager(new GMConfigs(), &d->statesManager);

	d->clock.begin();
	handleMessages();
	updateGameMachineRunningStates();

	eachHandler([=](auto, auto handler) {
		handler->init(mainWindow->getContext());
	});
}

void GameMachine::postMessage(GMMessage msg)
{
	D(d);
	d->messageQueue.push(msg);
}

GMMessage GameMachine::peekMessage()
{
	D(d);
	return d->lastMessage;
}

void GameMachine::createModelDataProxyAndTransfer(const GMContext* context, GMModel* model)
{
	if (model)
	{
		GMModelDataProxy* modelDataProxy = model->getModelDataProxy();
		if (!modelDataProxy)
		{
			getFactory()->createModelDataProxy(context, model, &modelDataProxy);
			model->setModelDataProxy(modelDataProxy);
		}
		modelDataProxy->transfer();
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
	updateGameMachineRunningStates();
	eachHandler([](auto window, auto)
	{
		window->getContext()->engine->init();
	});

	// 初始化gameHandler
	if (!d->states.crashDown)
	{
		eachHandler([](auto, auto handler)
		{
			handler->start();
		});
	}
	else
	{
		terminate();
	}

	// 消息循环
	runEventLoop();
}

void GameMachine::addWindow(IWindow* window)
{
	D(d);
	d->windows.insert(window);
}

bool GameMachine::removeWindow(IWindow* window)
{
	D(d);
	GMsize_t count = d->windows.erase(window);
	return count > 0;
}

bool GameMachine::renderFrame()
{
	D(d);
	GMClock frameCounter;
	// 记录帧率
	frameCounter.begin();

	// 处理GameMachine消息
	if (!handleMessages())
		return false;

	// 检查是否崩溃
	if (checkCrashDown())
		return true;

	// 调用Handler
	handlerEvents();

	// 更新时钟
	d->clock.update();

	// 更新状态
	updateGameMachineRunningStates();

	// 本帧结束
	eachHandler([](auto, auto handler) {
		handler->event(GameMachineHandlerEvent::FrameEnd);
	});
	d->states.lastFrameElpased = frameCounter.elapsedFromStart();
	return true;
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
		handleMessage(s_frameUpdateMsg);
		d->clock.update();
		return true;
	}
	return false;
}

void GameMachine::handlerEvents()
{
	D(d);
	eachHandler([](auto window, auto handler) {
		handler->event(GameMachineHandlerEvent::FrameStart);
		if (window->isWindowActivate())
			handler->event(GameMachineHandlerEvent::Activate);
		else
			handler->event(GameMachineHandlerEvent::Deactivate);

		handler->event(GameMachineHandlerEvent::Simulate);
		handler->event(GameMachineHandlerEvent::Render);
		window->msgProc(s_frameUpdateMsg);
	});
}

bool GameMachine::handleMessages()
{
	D(d);
	GMMessage msg;
	while (d->messageQueue.size() > 0)
	{
		msg = d->messageQueue.front();

		if (!handleMessage(msg))
		{
			d->messageQueue.pop();
			return false;
		}
		d->messageQueue.pop();
	}
	d->lastMessage = msg;
	return true;
}

bool GameMachine::handleMessage(const GMMessage& msg)
{
	D(d);
	switch (msg.msgType)
	{
	case GameMachineMessageType::QuitGameMachine:
		return false;
	case GameMachineMessageType::CrashDown:
	{
		d->states.crashDown = true;
		break;
	}
	default:
		break;
	}

	for (auto window : d->windows)
	{
		window->msgProc(msg);
		window->getContext()->engine->event(msg);
	}
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
	eachHandler([](auto, auto handler) {
		handler->event(GameMachineHandlerEvent::Terminate);
	});

	for (auto iter = d->managerQueue.rbegin(); iter != d->managerQueue.rend(); ++iter)
	{
		GM_delete(*iter);
	}
}

void GameMachine::updateGameMachineRunningStates()
{
	D(d);
	d->states.elapsedTime = d->clock.getTime();
	d->states.fps = d->clock.getFps();
}

void GameMachine::eachHandler(std::function<void(IWindow*, IGameHandler*)> action)
{
	D(d);
	for (auto window : d->windows)
	{
		action(window, window->getHandler());
	}
}