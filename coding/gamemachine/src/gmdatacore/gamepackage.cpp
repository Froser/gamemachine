﻿#include "stdafx.h"
#include "gamepackage.h"
#include "gmengine/controllers/factory.h"
#include <sys/stat.h>
#include "utilities/assert.h"
#include "gmengine/elements/bspgameworld.h"
#include "gmengine/controllers/gamemachine.h"

GamePackage::GamePackage(GameMachine* gm, IFactory* factory)
{
	D(d);
	d.factory = factory;
	d.gameMachine = gm;
}

GamePackageData& GamePackage::gamePackageData()
{
	D(d);
	return d;
}

IGamePackageHandler* GamePackage::getHandler()
{
	D(d);
	return d.handler;
}

void GamePackage::loadPackage(const char* path)
{
	// TODO 可能读取多个pk，分优先级
	// 这个以后再做
	D(d);
	struct stat s;
	stat(path, &s);

	if ((s.st_mode & S_IFMT) == S_IFDIR)
	{
		// 读取整个目录
		size_t len = strlen(path);
		if (path[len - 2] == '/' || path[len - 2] == '\\')
			d.packagePath = path;
		else
			d.packagePath = std::string(path) + '/';

		IGamePackageHandler* handler = nullptr;
		d.factory->createGamePackage(this, &handler);
		d.handler.reset(handler);
	}
	else
	{
		// 可能是.7z等情况
		ASSERT(false);
	}

	d.handler->init();
}

void GamePackage::createBSPGameWorld(const char* map, OUT BSPGameWorld** gameWorld)
{
	D(d);
	ASSERT(gameWorld);

	BSPGameWorld* world = new BSPGameWorld(this);
	*gameWorld = world;

	IFactory* factory = d.gameMachine->getFactory();
	IGraphicEngine* engine = d.gameMachine->getGraphicEngine();
	world->setGameMachine(d.gameMachine);
	world->loadBSP(path(PI_MAPS, map).c_str());
}

std::string GamePackage::path(PackageIndex index, const char* filename)
{
	D(d);
	ASSERT(d.handler);
	return d.handler->pathRoot(index) + filename;
}