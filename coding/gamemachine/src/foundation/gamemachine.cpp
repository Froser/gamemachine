#include "stdafx.h"
#include "gamemachine.h"
#include "gmdatacore/glyph/gmglyphmanager.h"
#include "gmengine/gmgameobject.h"
#include "gmconfig.h"

#ifdef _WINDOWS
#	include "os/gmdirectsound_sounddevice.h"
#endif

void GameMachine::init(
	GraphicSettings settings,
	AUTORELEASE IFactory* factory,
	AUTORELEASE IGameHandler* gameHandler
)
{
	D(d);
	d->settings = settings;
	d->factory.reset(factory);
	d->gameHandler.reset(gameHandler);

	IWindow* window;
	d->factory->createWindow(&window);
	d->window.reset(window);

	IGraphicEngine* engine;
	d->factory->createGraphicEngine(&engine);
	engine->setGraphicSettings(&d->settings);
	d->engine.reset(engine);

	d->gamePackageManager.reset(new GMGamePackage(factory));
	d->inputManager.reset(new GMInput());

	d->configManager.reset(new GMConfig());

	initDebugger();
}

void GameMachine::initDebugger()
{
	DBG_SET_INT(CALCULATE_BSP_FACE, 1);
	DBG_SET_INT(POLYGON_LINE_MODE, 0);
	DBG_SET_INT(DRAW_ONLY_SKY, 0);
	DBG_SET_INT(DRAW_NORMAL, DRAW_NORMAL_OFF);
}

IGraphicEngine* GameMachine::getGraphicEngine()
{
	D(d);
	return d->engine;
}

IWindow* GameMachine::getWindow()
{
	D(d);
	return d->window;
}

IFactory* GameMachine::getFactory()
{
	D(d);
	return d->factory;
}

void GameMachine::postMessage(GameMachineMessage msg)
{
	D(d);
	d->messageQueue.push(msg);
}

GraphicSettings& GameMachine::getSettings()
{
	D(d);
	return d->settings;
}

GMGlyphManager* GameMachine::getGlyphManager()
{
	D(d);
	return d->glyphManager;
}

GMGamePackage* GameMachine::getGamePackageManager()
{
	D(d);
	return d->gamePackageManager;
}

GMfloat GameMachine::getFPS()
{
	D(d);
	return d->clock.getFps();
}

GMfloat GameMachine::evaluateDeltaTime()
{
	D(d);
	return d->clock.evaluateDeltaTime();
}

GMfloat GameMachine::getGameTimeSeconds()
{
	D(d);
	return d->clock.getTime();
}

void GameMachine::initObjectPainter(GMGameObject* obj)
{
	D(d);
	ObjectPainter* painter;
	getFactory()->createPainter(getGraphicEngine(), obj->getObject(), &painter);
	ASSERT(!obj->getObject()->getPainter());
	obj->getObject()->setPainter(painter);
	painter->transfer();
}

GMInput* GameMachine::getInputManager()
{
	D(d);
	return d->inputManager;
}

GameMachine::EndiannessMode GameMachine::getMachineEndianness()
{
	static EndiannessMode ret = UNKNOWN_YET;
	if (ret == UNKNOWN_YET)
	{
		long int i = 1;
		const char *p = (const char *)&i;
		if (p[0] == 1)
			ret = LITTLE_ENDIAN;
		else
			ret = BIG_ENDIAN;
	}
	return ret;
}

void GameMachine::startGameMachine()
{
	D(d);
	// 创建Window (createWindow会初始化glew，所有GL操作必须要放在create之后）
	/*
	d->window->initWindowSize(d->settings.windowSize[0], d->settings.windowSize[1]);
	d->window->setWindowResolution(d->settings.resolution[0], d->settings.resolution[1]);
	d->window->setWindowPosition(d->settings.startPosition[0], d->settings.startPosition[1]);
	*/
	d->window->createWindow();

#ifdef _WINDOWS
	// 创建声音设备
	GMSoundPlayerDevice::createInstance(d->window);
#endif

	// 创建Glyph管理器
	GMGlyphManager* glyphManager;
	d->factory->createGlyphManager(&glyphManager);
	d->glyphManager.reset(glyphManager);

	// 初始化gameHandler
	d->gameHandler->init();

	d->clock.begin();
	// 消息循环
	while (true)
	{
		if (!(d->window->handleMessages()))
			break;

		if (!handleMessages())
			break;
		
		d->gameHandler->event(GM_EVENT_SIMULATE);
		if (d->gameHandler->isWindowActivate())
			d->gameHandler->event(GM_EVENT_ACTIVATE);
		d->gameHandler->event(GM_EVENT_RENDER);
		d->window->swapBuffers();
		d->clock.update();
	}
}

bool GameMachine::handleMessages()
{
	D(d);
	GameMachineMessage msg;
	while (d->messageQueue.size() > 0)
	{
		msg = d->messageQueue.back();

		switch (msg)
		{
		case gm::GM_MESSAGE_EXIT:
			return false;
		default:
			break;
		}

		d->messageQueue.pop();
	}
	return true;
}