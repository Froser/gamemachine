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
#include "gmui/gmui_console.h"
#include "gmengine/gmcamera.h"

BEGIN_NS

enum GameMachineMessageType
{
	GM_MESSAGE_EXIT,
	GM_MESSAGE_CONSOLE,
	GM_MESSAGE_WINDOW_SIZE,
};

struct GameMachineMessage
{
	enum ParamType
	{
		Type_GMObject,
		Type_Bool,
		Type_Int,
	};

	union Param
	{
		GMObject* obj;
		bool b;
		GMint i;
	};

	GameMachineMessageType msgType;
	ParamType paramType;
	Param param;
};

typedef Vector<Pair<GMUIWindow*, GMUIWindowAttributes> > GameMachineWindows;

GM_PRIVATE_OBJECT(GameMachine)
{
	GMUIInstance instance;
	GMUIWindowAttributes mainWindowAttributes;
	GMClock clock;
	AutoPtr<GMUIWindow> mainWindow; // 主窗口
	AutoPtr<IFactory> factory;
	AutoPtr<IGraphicEngine> engine;
	AutoPtr<IGameHandler> gameHandler;
	AutoPtr<GMGlyphManager> glyphManager;
	AutoPtr<GMGamePackage> gamePackageManager;
	AutoPtr<IInput> inputManager;
	AutoPtr<GMConfig> configManager;
	GMCamera camera;
	std::queue<GameMachineMessage> messageQueue;
	GameMachineWindows windows; // 除主窗口以外的窗口列表
	AUTORELEASE GMUIConsole* consoleWindow; // 内置调试窗口
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

	// 绘制引擎
	inline IGraphicEngine* getGraphicEngine() { D(d); return d->engine; }

	// 主窗口
	inline GMUIWindow* getMainWindow() { D(d); return d->mainWindow; }

	// 工厂管理
	inline IFactory* getFactory() { D(d); return d->factory; }

	// 窗口管理
	GMUIWindow* appendWindow(AUTORELEASE GMUIWindow* window, const GMUIWindowAttributes& attrs);
	inline const GameMachineWindows& getWindows() { D(d); return d->windows; }

	// 配置管理
	GMConfig* getConfigManager() { D(d); return d->configManager; }

	// 字体管理
	GMGlyphManager* getGlyphManager() { D(d); return d->glyphManager; }

	// 资源管理
	GMGamePackage* getGamePackageManager() { D(d); return d->gamePackageManager; }

	// HID
	IInput* getInputManager() { D(d); return d->inputManager; }

	// 相机管理
	GMCamera& getCamera() { D(d); return d->camera; }

	// 时间管理
	GMfloat getFPS();
	GMfloat evaluateDeltaTime();
	GMfloat getGameTimeSeconds();

	// 绘制管理
	void initObjectPainter(GMGameObject* obj);

	// 大小端模式
	EndiannessMode getMachineEndianness();

	// 发送事件
	void postMessage(GameMachineMessage msg);

	void startGameMachine();

private:
	bool handleMessages();
	void defaultMainWindowAttributes();
	void createWindows();
	void updateWindows();
	void initInner();
};

END_NS
#endif
