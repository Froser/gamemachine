#ifndef __GMMESSAGE_H__
#define __GMMESSAGE_H__
#include <gmcommon.h>
BEGIN_NS

#define GMM_CONSOLE_SELECT_FILTER	0x1000
#define GMM_CONSOLE_INFO			0x0000
#define GMM_CONSOLE_WARNING			0x0001
#define GMM_CONSOLE_ERROR			0x0002
#define GMM_CONSOLE_DEBUG			0x0003

enum class GameMachineMessageType
{
	None,
	Quit,
	CrashDown,
	Console,
	WindowSizeChanged,
	Dx11Ready,
	SystemMessage,
};

struct GMMessage
{
	GMMessage(GameMachineMessageType t = GameMachineMessageType::None, GMint tp = 0, GMint v = 0, void* objPtr = nullptr)
		: msgType(t)
		, type(tp)
		, value(v)
	{}

	GameMachineMessageType msgType = GameMachineMessageType::None;
	GMint type = 0;
	GMint value = 0;
	void* objPtr = 0;
};

enum class GMSystemEventType
{

};

GM_PRIVATE_OBJECT(GMSystemEvent)
{
	GMMessage message = { GameMachineMessageType::SystemMessage };
	GMSystemEventType type;
};

class GMSystemEvent : public GMObject
{
	DECLARE_PRIVATE(GMSystemEvent)
	GM_DECLARE_PROPERTY(Type, type, GMSystemEventType);

public:
	GMSystemEvent();
};

class GMSystemKeyEvent : public GMSystemEvent
{

};

END_NS
#endif