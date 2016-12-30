#ifndef __GAMELOOP_H__
#define __GAMELOOP_H__
#include "common.h"
#include "utilities/stopwatch.h"
BEGIN_NS

struct GameLoopSettings
{
	GMint fps;
};

struct IGameHandler
{
	virtual void mouse() = 0;
	virtual void keyboard() = 0;
	virtual void render() = 0;
	virtual void logicalFrame(GMfloat elapsed) = 0;
};

class GameLoop
{
public:
	GameLoop(const GameLoopSettings& settings, IGameHandler* handler);

public:
	void drawFrame();

	void start();
	void terminate();
	bool isTerminated();

	GMint getCurrentFPS() { return m_currentFps; }
	const GameLoopSettings& getSettings() const { return m_settings; }

	void exit();

private:
	void updateSettings();

private:
	bool m_running;
	GameLoopSettings m_settings;
	IGameHandler* m_handler;
	GMfloat m_eachFrameElapse;
	GMint m_currentFps;
	Stopwatch m_drawStopwatch;
	Stopwatch m_logicStopwatch;
	bool m_terminate;
	GMfloat m_timeElapsed;
};

class GameLoopUtilities
{
public:
	static void gm_gl_registerGameLoop(GameLoop& gl);
	static GameLoop* getRegisteredGameLoop() { return s_gameLoop; }

private:
	static GameLoop* s_gameLoop;
};
END_NS
#endif