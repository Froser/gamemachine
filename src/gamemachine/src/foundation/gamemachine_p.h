#ifndef __GAMEMACHINE_P_H__
#define __GAMEMACHINE_P_H__
#include <gmcommon.h>

BEGIN_NS

GM_PRIVATE_OBJECT_UNALIGNED(GameMachine)
{
	GM_DECLARE_PUBLIC(GameMachine)

	GMClock clock;
	bool inited = false;
	bool gamemachinestarted = false;
	Set<IWindow*> windows;
	const IRenderContext* computeContext = nullptr;
	IFactory* factory = nullptr;
	GMGamePackage* gamePackageManager = nullptr;
	GMMessage lastMessage;
	Queue<GMMessage> messageQueue;
	Vector<IDestroyObject*> managerQueue;
	Queue<GMCallable> callableQueue;
	GMGameMachineRunningStates states;
	GMGameMachineRunningMode runningMode;

	void initSystemInfo();
	void runEventLoop();
	IDestroyObject* registerManager(IDestroyObject* object);
	bool handleMessage(const GMMessage& msg);
	void updateGameMachine();
	void setRenderEnvironment(GMRenderEnvironment renv);
	bool checkCrashDown();
	void beginHandlerEvents(IWindow* window);
	void endHandlerEvents(IWindow* window);
	void eachHandler(std::function<void(IWindow*, IGameHandler*)> action);
	void beforeStartGameMachine();
	void initHandlers();
	void invokeCallables();
};

END_NS
#endif
