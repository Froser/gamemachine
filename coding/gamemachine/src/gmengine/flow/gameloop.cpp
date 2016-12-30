#include "stdafx.h"
#include <cstdio>
#include "gameloop.h"
#include "GL/freeglut.h"
#include "utilities/assert.h"

GameLoop::GameLoop(const GameLoopSettings& settings, IGameHandler* handler)
	: m_settings(settings)
	, m_running(false)
	, m_handler(handler)
	, m_terminate(false)
	, m_timeElapsed(1.f / 60.f)
{
	updateSettings();
}

void GameLoop::drawFrame()
{
	m_handler->logicalFrame(m_timeElapsed);

	m_drawStopwatch.start();
	m_handler->mouse();
	m_handler->keyboard();
	m_handler->render();
	m_drawStopwatch.stop();
	m_timeElapsed = (m_drawStopwatch.getMillisecond()) / 1000 + (1.f / 60.f);
}

void GameLoop::start()
{
	while (!isTerminated())
	{
		drawFrame();
	}
	exit();
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

void GameLoop::exit()
{
	::exit(0);
}