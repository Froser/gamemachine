#ifndef __GAMEMACHINE_H__
#define __GAMEMACHINE_H__
#include "common.h"
#include "foundation/utilities/utilities.h"
#include <queue>
#include "gmdatacore/glyph/gmglyphmanager.h"
#include "gmdatacore/gamepackage/gmgamepackage.h"
#include "os/gminput.h"
#include "gmthreads.h"
#include "gmui/gmui.h"
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
	GMUIInstance instance;
	GMUIWindowAttributes mainWindowAttributes;
	GMClock clock;
	AutoPtr<GMUIWindow> mainWindow;
	AutoPtr<IFactory> factory;
	AutoPtr<IGraphicEngine> engine;
	AutoPtr<IGameHandler> gameHandler;
	AutoPtr<GMGlyphManager> glyphManager;
	AutoPtr<GMGamePackage> gamePackageManager;
	AutoPtr<GMInput> inputManager;
	AutoPtr<GMConfig> configManager;
	std::queue<GameMachineMessage> messageQueue;
	Vector<Pair<GMUIWindow*, GMUIWindowAttributes> > childWindows;
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
		GMUIInstance instance,
		AUTORELEASE IFactory* factory,
		AUTORELEASE IGameHandler* gameHandler
	);

	void setMainWindowAttributes(const GMUIWindowAttributes& attrs);

	IGraphicEngine* getGraphicEngine();
	GMUIWindow* getWindow();
	IFactory* getFactory();

	// 窗口管理
	GMUIWindow* appendWindow(AUTORELEASE GMUIWindow* window, const GMUIWindowAttributes& attrs);

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
	void defaultMainWindowAttributes();
	void createChildWindows();
};

END_NS
#endif
