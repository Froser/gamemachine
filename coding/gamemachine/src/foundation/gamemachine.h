#ifndef __GAMEMACHINE_H__
#define __GAMEMACHINE_H__
#include "common.h"
#include "gmengine/controllers/graphic_engine.h"
#include "foundation/utilities/utilities.h"
#include <queue>
#include "gmdatacore/glyph/glyphmanager.h"
BEGIN_NS

enum GameMachineMessage
{
	GM_MESSAGE_EXIT,
};

GM_PRIVATE_OBJECT(GameMachine)
{
	GMClock clock;
	GraphicSettings settings;
	AutoPtr<IWindow> window;
	AutoPtr<IFactory> factory;
	AutoPtr<IGraphicEngine> engine;
	AutoPtr<IGameHandler> gameHandler;
	AutoPtr<GlyphManager> glyphManager;
	std::queue<GameMachineMessage> messageQueue;
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
	void init(
		GraphicSettings settings,
		AUTORELEASE IFactory* factory,
		AUTORELEASE IGameHandler* gameHandler
	);

	IGraphicEngine* getGraphicEngine();
	IWindow* getWindow();
	IFactory* getFactory();
	GraphicSettings& getSettings();

	// 字体管理
	GlyphManager* getGlyphManager();

	// 时间管理
	GMfloat getFPS();
	GMfloat evaluateDeltaTime();
	GMfloat getGameTimeSeconds();

	// 绘制管理
	void initObjectPainter(GameObject* obj);

	void startGameMachine();
	void postMessage(GameMachineMessage msg);

private:
	void init();
	void initDebugger();
	bool handleMessages();
};

END_NS
#endif
