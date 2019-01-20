#include "stdafx.h"
#include "gamemachine.h"
#include "gmdata/glyph/gmglyphmanager.h"
#include "gmengine/gameobjects/gmgameobject.h"
#include "gmconfigs.h"
#include "gmengine/ui/gmwidget.h"
#include "gmmessage.h"

extern "C"
{
	GM_EXPORT bool GMQueryCapability(GMCapability cp)
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

GameMachine& GameMachine::instance()
{
	static GameMachine s_instance;
	return s_instance;
}

GameMachine::GameMachine()
{
	GMDebugger::instance();
	updateGameMachineRunningStates();
}

void GameMachine::init(
	const GMGameMachineDesc& desc
)
{
	D(d);
	initSystemInfo();

	d->runningMode = desc.runningMode;
	setRenderEnvironment(desc.renderEnvironment);
	registerManager(desc.factory, &d->factory);
	registerManager(new GMGamePackage(), &d->gamePackageManager);
	registerManager(new GMConfigs(), &d->statesManager);

	d->clock.begin();
	handleMessages();
	updateGameMachineRunningStates();

	eachHandler([=](auto window, auto handler) {
		if (handler)
			handler->init(window->getContext());
	});

	d->inited = true;
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

void GameMachine::startGameMachine()
{
	D(d);
	updateGameMachineRunningStates();
	handleMessages();
	eachHandler([](auto window, auto)
	{
		window->getContext()->getEngine()->init();
	});

	// 初始化gameHandler
	if (!d->states.crashDown)
	{
		eachHandler([](auto window, auto handler)
		{
			if (handler)
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

void GameMachine::addWindow(AUTORELEASE IWindow* window)
{
	D(d);
	d->windows.insert(window);
	if (d->inited)
	{
		handleMessages();
		auto handler = window->getHandler();
		if (handler)
			handler->init(window->getContext());
	}
}

void GameMachine::removeWindow(IWindow* window)
{
	D(d);
	postMessage({ GameMachineMessageType::DeleteWindowLater, 0, window });
}

bool GameMachine::renderFrame(IWindow* window)
{
	D(d);
	GMClock frameCounter;
	// 记录帧率
	frameCounter.begin();

	// 检查是否崩溃
	if (checkCrashDown())
		return true;

	// 调用Handler
	beginHandlerEvents(window);

	// 更新时钟
	d->clock.update();

	// 更新状态
	updateGameMachineRunningStates();

	// 本帧结束
	endHandlerEvents(window);

	d->states.lastFrameElpased = frameCounter.elapsedFromStart();
	return true;
}

bool GameMachine::sendMessage(const GMMessage& msg)
{
	return handleMessage(msg);
}

void GameMachine::exit()
{
	postMessage({ gm::GameMachineMessageType::QuitGameMachine });
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

void GameMachine::beginHandlerEvents(IWindow* window)
{
	D(d);
	auto action = [](auto window, auto handler) {
		window->getContext()->switchToContext();
		if (handler)
		{
			handler->event(GameMachineHandlerEvent::FrameStart);
			if (window->isWindowActivate())
				handler->event(GameMachineHandlerEvent::Activate);
			else
				handler->event(GameMachineHandlerEvent::Deactivate);

			handler->event(GameMachineHandlerEvent::Update);
			handler->event(GameMachineHandlerEvent::Render);
		}
		window->msgProc(s_frameUpdateMsg);
	};

	if (window)
		action(window, window->getHandler());
	else
		eachHandler(action);
}

void GameMachine::endHandlerEvents(IWindow* window)
{
	auto action = [](auto, auto handler) {
		if (handler)
			handler->event(GameMachineHandlerEvent::FrameEnd);
	};

	if (window)
		action(window, window->getHandler());
	else
		eachHandler(action);
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
	case GameMachineMessageType::DeleteLater:
	{
		GMObject* obj = static_cast<GMObject*>(msg.object);
		GM_delete(obj);
		break;
	}
	case GameMachineMessageType::DeleteWindowLater:
	{
		IWindow* window = static_cast<IWindow*>(msg.object);
		if (window)
		{
			d->windows.erase(window);
			GM_delete(window);
			if (d->windows.empty())
				exit();
		}
		break;
	}
	default:
		break;
	}

	for (decltype(auto) window : d->windows)
	{
		window->msgProc(msg);
		window->getContext()->getEngine()->msgProc(msg);
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

	for (auto window : d->windows)
	{
		GM_delete(window);
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
	for (decltype(auto) window : d->windows)
	{
		action(window, window->getHandler());
	}
}