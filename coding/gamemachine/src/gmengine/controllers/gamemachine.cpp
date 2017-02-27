#include "stdafx.h"
#include "gamemachine.h"
#include "factory.h"
#include "gameloop.h"

GameMachine::GameMachine(
	GraphicSettings settings,
	AUTORELEASE IWindow* window,
	AUTORELEASE IFactory* factory,
	AUTORELEASE IGameHandler* gameHandler
)
	: m_settings(settings)
	, m_window(window)
	, m_factory(factory)
	, m_gameHandler(gameHandler)
{
	m_gameHandler->setGameMachine(this);
	init();
}

void GameMachine::init()
{
	IGraphicEngine* engine;
	m_factory->createGraphicEngine(&engine);
	engine->setGraphicSettings(&m_settings);
	m_engine.reset(engine);

	getGameLoop()->init(m_settings, m_gameHandler);
	initDebugger();
}

void GameMachine::initDebugger()
{
	DBG_SET_INT(CALCULATE_BSP_FACE, 1);
	DBG_SET_INT(POLYGON_LINE_MODE, 0);
}

IGraphicEngine* GameMachine::getGraphicEngine()
{
	return m_engine;
}

IWindow* GameMachine::getWindow()
{
	return m_window;
}

IFactory* GameMachine::getFactory()
{
	return m_factory;
}

GameLoop* GameMachine::getGameLoop()
{
	return GameLoop::getInstance();
}

GraphicSettings& GameMachine::getSettings()
{
	return m_settings;
}

void GameMachine::startGameMachine()
{
	m_window->initWindowSize(m_settings.windowSize[0], m_settings.windowSize[1]);
	m_window->setWindowResolution(m_settings.resolution[0], m_settings.resolution[1]);
	m_window->setWindowPosition(m_settings.startPosition[0], m_settings.startPosition[1]);
	m_window->createWindow();
	m_gameHandler->init();
	getGameLoop()->init(m_settings, m_gameHandler);
	getGameLoop()->start();
}