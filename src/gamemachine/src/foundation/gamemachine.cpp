#include "stdafx.h"
#include "gamemachine.h"
#include "gmdata/glyph/gmglyphmanager.h"
#include "gmengine/gameobjects/gmgameobject.h"
#include "gmconfigs.h"
#include "gmengine/ui/gmwidget.h"
#include "gmmessage.h"
#include "wrapper/dx11wrapper.h"
#include "gamemachine_p.h"

extern "C"
{
	GM_EXPORT bool GMQueryCapability(GMCapability cp)
	{
		if (cp == GMCapability::SupportOpenGL)
			return true;

		if (cp == GMCapability::SupportDirectX11)
		{
#if GM_USE_DX11
			return true;
#else
			enum { Unknown, Yes, No };
			static bool s_canCreateDx11;
			static std::once_flag s_flag;
			std::call_once(s_flag, [](bool& canCreate) {
				canCreate = CreateDirectX11Factory(nullptr);
			}, s_canCreateDx11);
			return s_canCreateDx11;
#endif
		}

		GM_ASSERT(false); // wrong capability type
		return false;
	}
}

BEGIN_NS

namespace
{
	static GMMessage s_frameUpdateMsg(GameMachineMessageType::FrameUpdate);
	GMMutex s_callableLock;
}


void GameMachinePrivate::setRenderEnvironment(GMRenderEnvironment renv)
{
	switch (renv)
	{
	case GMRenderEnvironment::OpenGL:
		if (GMQueryCapability(GMCapability::SupportOpenGL))
		{
			states.renderEnvironment = renv;
			return;
		}
	case GMRenderEnvironment::DirectX11:
		if (GMQueryCapability(GMCapability::SupportDirectX11))
		{
			states.renderEnvironment = renv;
			return;
		}
		break;
	default:
		break;
	}
	GM_ASSERT(!"Wrong render environment");
	states.renderEnvironment = GMRenderEnvironment::OpenGL;
}

bool GameMachinePrivate::checkCrashDown()
{
	if (states.crashDown)
	{
		// 宕机的情况下，只更新下面的状态
		handleMessage(s_frameUpdateMsg);
		clock.update();
		return true;
	}
	return false;
}

void GameMachinePrivate::beginHandlerEvents(IWindow* window)
{
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

void GameMachinePrivate::endHandlerEvents(IWindow* window)
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

bool GameMachinePrivate::handleMessage(const GMMessage& msg)
{
	P_D(pd);
	switch (msg.msgType)
	{
	case GameMachineMessageType::QuitGameMachine:
		return false;
	case GameMachineMessageType::CrashDown:
	{
		states.crashDown = true;
		break;
	}
	case GameMachineMessageType::DeleteLater:
	{
		GMObject* obj = static_cast<GMObject*>(msg.object);
		obj->destroy();
		break;
	}
	case GameMachineMessageType::DeleteWindowLater:
	{
		IWindow* window = static_cast<IWindow*>(msg.object);
		if (window)
		{
			windows.erase(window);
			window->destroy();
			if (windows.empty())
				pd->exit();
		}
		break;
	}
	default:
		break;
	}

	for (decltype(auto) window : windows)
	{
		window->msgProc(msg);
		window->getContext()->getEngine()->msgProc(msg);
	}
	return true;
}

void GameMachinePrivate::updateGameMachine()
{
	states.elapsedTime = clock.getTime();
	states.fps = clock.getFps();
	invokeCallables();
}

void GameMachinePrivate::eachHandler(std::function<void(IWindow*, IGameHandler*)> action)
{
	for (decltype(auto) window : windows)
	{
		action(window, window->getHandler());
	}
}

void GameMachinePrivate::beforeStartGameMachine()
{
	P_D(pd);
	updateGameMachine();
	pd->handleMessages();
	initHandlers();
	gamemachinestarted = true;
}

void GameMachinePrivate::initHandlers()
{
	P_D(pd);
	eachHandler([](auto window, auto)
	{
		window->getContext()->getEngine()->init();
	});

	// 初始化gameHandler
	if (!states.crashDown)
	{
		eachHandler([](auto window, auto handler)
		{
			if (handler)
				handler->start();
		});
	}
	else
	{
		if (!gamemachinestarted)
			pd->finalize();
	}
}

void GameMachinePrivate::invokeCallables()
{
	GMMutexLock lock(&s_callableLock);
	lock->lock();

	while (!callableQueue.empty())
	{
		GMCallable callable = callableQueue.front();
		if (callable)
			callable();
		callableQueue.pop();
	}
}

IDestroyObject* GameMachinePrivate::registerManager(IDestroyObject* object)
{
	managerQueue.push_back(object);
	return object;
}

GameMachine& GameMachine::instance()
{
	static GameMachine s_instance;
	return s_instance;
}

GameMachine::~GameMachine()
{

}

GameMachine::GameMachine()
{
	GM_CREATE_DATA();
	GM_SET_PD();
	D(d);
	GMDebugger::instance();
	d->updateGameMachine();
}

void GameMachine::init(
	const GMGameMachineDesc& desc
)
{
	D(d);
	d->initSystemInfo();

	d->runningMode = desc.runningMode;
	d->setRenderEnvironment(desc.renderEnvironment);
	d->factory = gm_cast<IFactory*>(d->registerManager(desc.factory));
	d->gamePackageManager = gm_cast<GMGamePackage*>(d->registerManager(new GMGamePackage()));

	if (desc.runningMode != GMGameMachineRunningMode::ComputeOnly)
	{
		d->clock.begin();
		handleMessages();
		d->updateGameMachine();

		d->eachHandler([=](auto window, auto handler) {
			if (handler)
				handler->init(window->getContext());
		});
	}
	else
	{
		d->factory->createComputeContext(&d->computeContext);
	}

	d->inited = true;
}

GMGamePackage* GameMachine::getGamePackageManager()
{
	D(d); return d->gamePackageManager;
}

IFactory* GameMachine::getFactory()
{
	D(d); return d->factory;
}

const GMGameMachineRunningStates& GameMachine::getRunningStates() const
{
	D(d); return d->states;
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
	// 做一些开始之前的工作
	d->beforeStartGameMachine();

	// 消息循环
	d->runEventLoop();
}

void GameMachine::startGameMachineWithoutMessageLoop()
{
	D(d);
	// 做一些开始之前的工作
	d->beforeStartGameMachine();

	// 不进行消息循环
	return;
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

		if (d->gamemachinestarted)
		{
			// 如果gamemachine已经开始，则表示错过了window初始化
			auto engine = window->getContext()->getEngine();
			engine->init();
			handler->start();
		}
	}
}

void GameMachine::removeWindow(IWindow* window)
{
	D(d);
	postMessage({ GameMachineMessageType::DeleteWindowLater, 0, window });
}

const IRenderContext* GameMachine::getComputeContext()
{
	D(d);
	return d->computeContext;
}

bool GameMachine::renderFrame(IWindow* window)
{
	D(d);
	GMClock frameCounter;

	// 记录帧率
	frameCounter.begin();

	// 检查是否崩溃
	if (d->checkCrashDown())
		return false;

	// 调用Handler
	d->beginHandlerEvents(window);

	// 更新时钟
	d->clock.update();

	// 更新状态
	d->updateGameMachine();

	// 本帧结束
	d->endHandlerEvents(window);

#if GM_DEBUG
	// Debug模式下超过一定时间，认为是在调试
	constexpr GMfloat DEBUG_ELAPSED = 10;
	GMfloat lastFrameElapsed = frameCounter.elapsedFromStart();
	if (lastFrameElapsed > DEBUG_ELAPSED)
	{
		lastFrameElapsed = 1.f / 60;
		gm_info(gm_dbg_wrap("Detecting GameMachine triggered the break point. Skip one frame."));
	}
	d->states.lastFrameElapsed = lastFrameElapsed;
#else
	d->states.lastFrameElapsed = frameCounter.elapsedFromStart();
#endif

	return true;
}

bool GameMachine::sendMessage(const GMMessage& msg)
{
	D(d);
	return d->handleMessage(msg);
}

void GameMachine::invokeInMainThread(GMCallable callable)
{
	D(d);
	GMMutexLock lock(&s_callableLock);
	lock->lock();
	d->callableQueue.push(callable);
}

void GameMachine::exit()
{
	postMessage({ GameMachineMessageType::QuitGameMachine });
}

bool GameMachine::handleMessages()
{
	D(d);
	GMMessage msg;
	while (d->messageQueue.size() > 0)
	{
		msg = d->messageQueue.front();

		if (!d->handleMessage(msg))
		{
			d->messageQueue.pop();
			return false;
		}
		d->messageQueue.pop();
	}
	d->lastMessage = msg;
	return true;
}

void GameMachine::setGameMachineRunningStates(const GMGameMachineRunningStates& states)
{
	D(d);
	d->states = states;
}

void GameMachine::finalize()
{
	D(d);
	for (auto iter = d->managerQueue.rbegin(); iter != d->managerQueue.rend(); ++iter)
	{
		GM_delete(*iter);
		*iter = nullptr;
	}

	d->factory = nullptr;
	d->gamePackageManager = nullptr;

	GM_ASSERT(d->runningMode != GMGameMachineRunningMode::ComputeOnly || d->computeContext);
	GM_delete(d->computeContext);

	for (auto window : d->windows)
	{
		GM_delete(window);
	}
}

END_NS