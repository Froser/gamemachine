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

	world->setGameMachine(d.gameMachine);
	world->loadBSP(map);
}

bool GamePackage::readFile(PackageIndex index, const char* filename, REF GamePackageBuffer* buffer, REF std::string* fullFilename)
{
	D(d);
	std::string p = pathOf(index, filename);
	if (fullFilename)
		*fullFilename = p;
	return readFileFromPath(p.c_str(), buffer);
}

AlignedVector<std::string> GamePackage::getAllFiles(const char* directory)
{
	D(d);
	ASSERT(d.handler);
	return d.handler->getAllFiles(directory);
}

std::string GamePackage::pathOf(PackageIndex index, const char* filename)
{
	D(d);
	ASSERT(d.handler);
	return d.handler->pathRoot(index) + filename;
}

bool GamePackage::readFileFromPath(const char* path, REF GamePackageBuffer* buffer)
{
	D(d);
	ASSERT(d.handler);
	bool b = d.handler->readFileFromPath(path, buffer);
	gm_hook2(GamePackage, readFileFromPath, path, buffer);
	return b;
}
