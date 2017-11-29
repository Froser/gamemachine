#ifndef __GAMEMACHINE_H__
#define __GAMEMACHINE_H__
#include <gmcommon.h>
#include <utilities.h>
#include <input.h>
#include <gmassets.h>
#include "../gmdata/glyph/gmglyphmanager.h"
#include "../gmdata/gamepackage/gmgamepackage.h"
#include "../gmengine/gmcamera.h"
#include "gmthreads.h"

BEGIN_NS

#define GM gm::GameMachine::instance()

class GMCursorGameObject;
struct GMGameMachineRunningStates
{
	GMRect clientRect;
	GMRect windowRect;
	GMfloat lastFrameElpased = 0;
};

GM_PRIVATE_OBJECT(GameMachine)
{
	GMClock clock;

	IWindow* mainWindow = nullptr;
	IFactory* factory = nullptr;
	IGraphicEngine* engine = nullptr;
	GMGlyphManager* glyphManager = nullptr;
	GMGamePackage* gamePackageManager = nullptr;
	GMStates* statesManager = nullptr;
	IGameHandler* gameHandler = nullptr;

	GameMachineMessage lastMessage;

	// 内置调试窗口，他们本质是同一个对象，所以不能重复释放
	IWindow* consoleWindow = nullptr; 
	IDebugOutput* consoleOutput = nullptr;

	GMCamera camera;
	Queue<GameMachineMessage> messageQueue;
	Vector<IDispose*> manangerQueue;

	GMGameMachineRunningStates states;
	GMCursorGameObject* cursor;
};

class GameMachine : public GMSingleton<GameMachine>
{
	DECLARE_PRIVATE(GameMachine)
	DECLARE_SINGLETON(GameMachine)

	enum
	{
		MAX_KEY_STATE_BITS = 512,
	};

public:
	enum EndiannessMode
	{
		// Never returns:
		UNKNOWN_YET = -1,

		// Modes:
		LITTLE_ENDIAN = 0,
		BIG_ENDIAN = 1,
	};

public:
	GameMachine() = default;

public:
	void init(
		AUTORELEASE IWindow* mainWindow,
		const GMConsoleHandle& consoleHandle,
		AUTORELEASE IFactory* factory,
		AUTORELEASE IGameHandler* gameHandler
	);

	// 绘制引擎
	inline IGraphicEngine* getGraphicEngine() { D(d); return d->engine; }

	// 窗口
	inline IWindow* getMainWindow() { D(d); return d->mainWindow; }

	// 工厂管理
	inline IFactory* getFactory() { D(d); return d->factory; }

	// 配置管理
	GMStates* getStatesManager() { D(d); return d->statesManager; }

	// 字体管理
	GMGlyphManager* getGlyphManager() { D(d); return d->glyphManager; }

	// 资源管理
	GMGamePackage* getGamePackageManager() { D(d); return d->gamePackageManager; }

	// 相机管理
	GMCamera& getCamera() { D(d); return d->camera; }

	// 时间管理
	inline GMfloat getFPS() { D(d); return d->clock.getFps(); }
	inline GMfloat getGameTimeSeconds() { D(d); return d->clock.getTime(); }

	// 状态管理
	inline const GMGameMachineRunningStates& getGameMachineRunningStates() const { D(d); return d->states; }

	// 绘制管理
	void initObjectPainter(GMModel* model);

	// 大小端模式
	EndiannessMode getMachineEndianness();

	// 发送事件
	void postMessage(GameMachineMessage msg);
	GameMachineMessage peekMessage();

	inline void setCursor(GMCursorGameObject* cursor) { D(d); d->cursor = cursor; }
	inline GMCursorGameObject* getCursor() { D(d); return d->cursor; }

	void startGameMachine();

private:
	template <typename T, typename U> void registerManager(T* newObject, OUT U** manager);
	void terminate();
	bool handleMessages();
	void initInner();
	void updateGameMachineRunningStates();
};

END_NS
#endif
