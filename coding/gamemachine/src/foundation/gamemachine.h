#ifndef __GAMEMACHINE_H__
#define __GAMEMACHINE_H__
#include "common.h"
#include "gmengine/controllers/graphic_engine.h"
#include "foundation/utilities/utilities.h"
#include <queue>
#include "gmdatacore/glyph/gmglyphmanager.h"
#include "gmdatacore/gamepackage/gmgamepackage.h"
#include "os/gminput.h"
#include "gmthreads.h"
BEGIN_NS

enum GameMachineMessage
{
	GM_MESSAGE_EXIT,
};

// Multi-threads
template <GameMachineEvent e>
class GameLoopJob : public GMSustainedThread
{
public:
	void setHandler(IGameHandler* h)
	{
		handler = h;
	}

	virtual void sustainedRun() override
	{
		handler->event(e);
	}

private:
	IGameHandler* handler;
};

GM_PRIVATE_OBJECT(GameMachine)
{
	GMClock clock;
	GraphicSettings settings;
	AutoPtr<IWindow> window;
	AutoPtr<IFactory> factory;
	AutoPtr<IGraphicEngine> engine;
	AutoPtr<IGameHandler> gameHandler;
	AutoPtr<GMGlyphManager> glyphManager;
	AutoPtr<GMGamePackage> gamePackageManager;
	AutoPtr<GMInput> inputManager;
	AutoPtr<GMConfig> configManager;
	std::queue<GameMachineMessage> messageQueue;
	GameLoopJob<GM_EVENT_SIMULATE> simulateJob;
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
	void init(
		GraphicSettings settings,
		AUTORELEASE IFactory* factory,
		AUTORELEASE IGameHandler* gameHandler
	);

	IGraphicEngine* getGraphicEngine();
	IWindow* getWindow();
	IFactory* getFactory();
	GraphicSettings& getSettings();

	// 配置管理
	GMConfig* getConfigManager();

	// 字体管理
	GMGlyphManager* getGlyphManager();

	// 资源管理
	GMGamePackage* getGamePackageManager();

	// 时间管理
	GMfloat getFPS();
	GMfloat evaluateDeltaTime();
	GMfloat getGameTimeSeconds();

	// 绘制管理
	void initObjectPainter(GMGameObject* obj);

	// HID
	GMInput* getInputManager();

	// 大小端模式
	EndiannessMode getMachineEndianness();

	// 发送事件
	void postMessage(GameMachineMessage msg);

	void startGameMachine();

private:
	bool handleMessages();
};

END_NS
#endif
