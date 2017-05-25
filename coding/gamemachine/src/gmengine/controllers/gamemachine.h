#ifndef __GAMEMACHINE_H__
#define __GAMEMACHINE_H__
#include "common.h"
#include "graphic_engine.h"
#include "foundation/utilities/utilities.h"
#include <queue>
#include "gmdatacore/soundreader/soundreader.h"
BEGIN_NS

enum GameMachineEvent
{
	GM_EVENT_SIMULATE,
	GM_EVENT_RENDER,
	GM_EVENT_ACTIVATE,
};

enum GameMachineMessage
{
	GM_MESSAGE_EXIT,
};

struct IWindow
{
	virtual ~IWindow() {}
	virtual bool createWindow() = 0;
	virtual GMRect getWindowRect() = 0;
	virtual bool handleMessages() = 0;
	virtual void swapBuffers() = 0;
#ifdef _WINDOWS
	virtual HWND hwnd() = 0;
#endif
};

class GameMachine;
struct IGameHandler
{
	virtual ~IGameHandler() {}
	virtual void setGameMachine(GameMachine* gm) = 0;
	virtual void init() = 0;
	virtual void event(GameMachineEvent evt) = 0;
	virtual bool isWindowActivate() = 0;
};

struct IFactory;
struct IGameHandler;
class GlyphManager;

GM_PRIVATE_OBJECT(GameMachine)
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
	DEFINE_PRIVATE(GameMachine)

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
