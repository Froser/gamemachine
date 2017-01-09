#ifndef __GAMEMACHINE_H__
#define __GAMEMACHINE_H__
#include "common.h"
#include "graphic_engine.h"
#include "utilities/autoptr.h"
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

class GameMachine
{
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
	void startGameMachine();

private:
	void init();

private:
	GraphicSettings m_settings;
	AutoPtr<IWindow> m_window;
	AutoPtr<IFactory> m_factory;
	AutoPtr<IGraphicEngine> m_engine;
	AutoPtr<IGameHandler> m_gameHandler;
};

END_NS
#endif