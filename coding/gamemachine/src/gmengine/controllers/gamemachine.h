#ifndef __GAMEMACHINE_H__
#define __GAMEMACHINE_H__
#include "common.h"
#include "graphic_engine.h"
#include "utilities/autoptr.h"
#include "utilities/stopwatch.h"
BEGIN_NS

struct IWindow
{
	virtual void initWindowSize(GMfloat width, GMfloat height) = 0;
	virtual void setWindowResolution(GMfloat width, GMfloat height) = 0;
	virtual void setFullscreen(bool fullscreen) = 0;
	virtual void setWindowPosition(GMuint x, GMuint y) = 0;
	virtual void createWindow() = 0;
	virtual void startWindowLoop() = 0;
	virtual GMRect getWindowRect() = 0;
};

struct IFactory;
struct IGameHandler;
class GameLoop;
class GlyphManager;

class GameMachine
{
	enum
	{
		MAX_KEY_STATE_BITS = 512,
	};

public:
	GameMachine(
		GraphicSettings settings,
		AUTORELEASE IWindow* window,
		AUTORELEASE IFactory* factory,
		AUTORELEASE IGameHandler* gameHandler
	);

public:
	IGraphicEngine* getGraphicEngine();
	IWindow* getWindow();
	IFactory* getFactory();
	GameLoop* getGameLoop();
	GraphicSettings& getSettings();
	GlyphManager* getGlyphManager();
	void startGameMachine();

private:
	void init();
	void initDebugger();

private:
	GraphicSettings m_settings;
	AutoPtr<IWindow> m_window;
	AutoPtr<IFactory> m_factory;
	AutoPtr<IGraphicEngine> m_engine;
	AutoPtr<IGameHandler> m_gameHandler;
	AutoPtr<GlyphManager> m_glyphManager;
};

END_NS
#endif