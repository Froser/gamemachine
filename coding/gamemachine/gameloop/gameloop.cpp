#include "stdafx.h"
#include <cstdio>
#include "gameloop.h"
#include "glut.h"

GameLoop::GameLoop(const GameLoopSettings& settings, IGameHandler* handler)
	: m_settings(settings)
	, m_running(false)
	, m_handler(handler)
	, m_terminate(false)
{
	updateSettings();
}

void GameLoop::drawFrame()
{
	GMint elapse = 0;
	if (!m_drawStopwatch.isStarted())
	{
		m_drawStopwatch.start();
		return;
	}
	m_drawStopwatch.stop();
	elapse = m_drawStopwatch.getMillisecond();
	m_drawStopwatch.start();

	m_handler->render();

	GMfloat nextFrameInterval = 0;
	GMfloat skipFrameNum = 0;
	if (elapse < m_eachFrameElapse)
	{
		nextFrameInterval = m_eachFrameElapse - elapse;
		m_currentFps = m_settings.fps;
	}
	else
	{
		skipFrameNum = (GMint)(elapse / m_eachFrameElapse);
		nextFrameInterval = elapse - skipFrameNum * m_eachFrameElapse;
		m_currentFps = 1000 / (elapse + nextFrameInterval);
		if (m_currentFps < 0)
			m_currentFps = 0;
	}

#ifdef _WINDOWS
	Sleep(nextFrameInterval);
#endif
}

void GameLoop::messageLoop()
{
	GMint elapse = 0;
	if (!m_drawStopwatch.isStarted())
	{
		m_drawStopwatch.start();
		return;
	}
	m_drawStopwatch.stop();
	elapse = m_drawStopwatch.getMillisecond();
	m_drawStopwatch.start();

	m_handler->mouse();
	m_handler->keyboard();

	GMfloat nextFrameInterval = 0;
	GMfloat skipFrameNum = 0;
	if (elapse < m_eachFrameElapse)
	{
		nextFrameInterval = m_eachFrameElapse - elapse;
	}
	else
	{
		skipFrameNum = (GMint)(elapse / m_eachFrameElapse);
		nextFrameInterval = elapse - skipFrameNum * m_eachFrameElapse;
	}
#ifdef _WINDOWS
	Sleep(nextFrameInterval);
#endif
}

void GameLoop::terminate()
{
	m_terminate = true;
}

bool GameLoop::isTerminated()
{
	return m_terminate;
}

void GameLoop::updateSettings()
{
	m_eachFrameElapse = 1000.0f / m_settings.fps;
}

//////////////////////////////////////////////////////////////////////////

GameLoop* GameLoopUtilities::s_gameLoop = nullptr;
static void gameLoop(int v)
{
	GameLoop* gl = GameLoopUtilities::getRegisteredGameLoop();
	if (v == 1)
	{
		gl->drawFrame();

		char title[100];
		sprintf_s(title, "FPS: %d", gl->getCurrentFPS());
		glutSetIconTitle(title);

		glutTimerFunc(1, gameLoop, 1);
	}
	else if (v == 2)
	{
		gl->messageLoop();
		if (gl->isTerminated())
			exit(0);
		else
			glutTimerFunc(1, gameLoop, 2);
	}
}

void GameLoopUtilities::fglextlib_gl_registerGameLoop(GameLoop& gl)
{
	s_gameLoop = &gl;
	glutTimerFunc(1, gameLoop, 1);
	glutTimerFunc(1, gameLoop, 2);
}