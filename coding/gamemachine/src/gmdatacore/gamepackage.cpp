#include "stdafx.h"
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
	size_t len = strlen(path);
	char path_temp[FILENAME_MAX];
	strcpy_s(path_temp, path);
	if (path_temp[len - 1] == '/' || path_temp[len - 1] == '\\')
		*(path_temp + len - 1) = 0;

	struct stat s;
	stat(path_temp, &s);

	IGamePackageHandler* handler = nullptr;
	if ((s.st_mode & S_IFMT) == S_IFDIR)
	{
		// 读取整个目录
		d.packagePath = std::string(path_temp) + '/';
		d.factory->createGamePackage(this, GPT_DIRECTORY, &handler);
	}
	else
	{
		d.packagePath = std::string(path_temp);
		d.factory->createGamePackage(this, GPT_ZIP, &handler);
	}

	d.handler.reset(handler);
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

void GamePackage::readFileFromPath(const char* path, REF GamePackageBuffer* buffer)
{
	D(d);
	ASSERT(d.handler);
	return d.handler->readFileFromPath(path, buffer);
}

std::string GamePackage::path(PackageIndex index, const char* filename)
{
	D(d);
	ASSERT(d.handler);
	return d.handler->pathRoot(index) + filename;
}

std::vector<std::string> GamePackage::getAllFiles(const char* directory)
{
	D(d);
	ASSERT(d.handler);
	return d.handler->getAllFiles(directory);
}