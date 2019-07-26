#ifndef __GAMEMACHINE_P_H__
#define __GAMEMACHINE_P_H__
#include <gmcommon.h>

BEGIN_NS

GM_PRIVATE_OBJECT_UNALIGNED(GameMachine)
{
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
};

END_NS
#endif
