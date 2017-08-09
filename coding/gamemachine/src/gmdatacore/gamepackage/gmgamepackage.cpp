#include "stdafx.h"
#include "gmgamepackage.h"
#include "gmengine/gmbspgameworld.h"
#include <sys/stat.h>

GMGamePackage::GMGamePackage(IFactory* factory)
{
	D(d);
	d->factory = factory;
}

GMGamePackage::Data* GMGamePackage::gamePackageData()
{
	D(d);
	return d;
}

void GMGamePackage::loadPackage(const GMString& path)
{
	// TODO 可能读取多个pk，分优先级
	// 这个以后再做
	D(d);
	size_t len = path.length();
	char path_temp[FILENAME_MAX];
	path.copyString(path_temp);
	if (path_temp[len - 1] == '/' || path_temp[len - 1] == '\\')
		*(path_temp + len - 1) = 0;

	struct stat s;
	stat(path_temp, &s);

	IGamePackageHandler* handler = nullptr;
	if ((s.st_mode & S_IFMT) == S_IFDIR)
	{
		// 读取整个目录
		d->packagePath = std::string(path_temp) + '/';
		d->factory->createGamePackage(this, GPT_DIRECTORY, &handler);
	}
	else
	{
		d->packagePath = std::string(path_temp);
		d->factory->createGamePackage(this, GPT_ZIP, &handler);
	}

	d->handler.reset(handler);
	d->handler->init();
}

void GMGamePackage::createBSPGameWorld(const GMString& map, OUT GMBSPGameWorld** gameWorld)
{
	D(d);
	ASSERT(gameWorld);

	GMBSPGameWorld* world = new GMBSPGameWorld();
	*gameWorld = world;
	world->loadBSP(map);
}

bool GMGamePackage::readFile(GMPackageIndex index, const GMString& filename, REF GMBuffer* buffer, REF GMString* fullFilename)
{
	D(d);
	GMString p = pathOf(index, filename);
	if (fullFilename)
		*fullFilename = p;
	return readFileFromPath(p, buffer);
}

Vector<GMString> GMGamePackage::getAllFiles(const GMString& directory)
{
	D(d);
	ASSERT(d->handler);
	return d->handler->getAllFiles(directory);
}

GMString GMGamePackage::pathOf(GMPackageIndex index, const GMString& filename)
{
	D(d);
	ASSERT(d->handler);
	return d->handler->pathRoot(index) + filename;
}

bool GMGamePackage::readFileFromPath(const GMString& path, REF GMBuffer* buffer)
{
	D(d);
	ASSERT(d->handler);
	bool b = d->handler->readFileFromPath(path, buffer);
	gm_hook2(GMGamePackage, readFileFromPath, &path, buffer);
	return b;
}
