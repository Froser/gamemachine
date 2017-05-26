#ifndef __GAMEMACHINE_H__
#define __GAMEMACHINE_H__
#include "common.h"
#include "graphic_engine.h"
#include "foundation/utilities/utilities.h"
#include <queue>
#include "gmdatacore/soundreader/soundreader.h"
BEGIN_NS

enum GameMachineMessage
{
	GM_MESSAGE_EXIT,
};

struct IFactory;
struct IGameHandler;
class GlyphManager;

GM_ALIGNED_16(struct) GameMachinePrivate : public GMObjectPrivateBase<GameMachine>
{
	FPSCounter fpsCounter;
	GraphicSettings settings;
	AutoPtr<IWindow> window;
	AutoPtr<IFactory> factory;
	AutoPtr<IGraphicEngine> engine;
	AutoPtr<IGameHandler> gameHandler;
	AutoPtr<GlyphManager> glyphManager;
	std::queue<GameMachineMessage> messageQueue;
};

class GameMachine : public GMObject
{
	DECLARE_PRIVATE(GameMachine)

	enum
	{
		MAX_KEY_STATE_BITS = 512,
	};

public:
	GameMachine(
		GraphicSettings settings,
		AUTORELEASE IFactory* factory,
		AUTORELEASE IGameHandler* gameHandler
	);

public:
	IGraphicEngine* getGraphicEngine();
	IWindow* getWindow();
	IFactory* getFactory();
	GraphicSettings& getSettings();
	GlyphManager* getGlyphManager();
	GMfloat getFPS();
	GMfloat getElapsedSinceLastFrame();
	void startGameMachine();
	void postMessage(GameMachineMessage msg);

private:
	void init();
	void initDebugger();
	bool handleMessages();
};

END_NS
#endif
