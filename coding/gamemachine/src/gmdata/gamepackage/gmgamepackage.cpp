#include "stdafx.h"
#include "gmgamepackage.h"
#include "gmgamepackagehandler.h"
#include <sys/stat.h>

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
		createGamePackage(this, GPT_DIRECTORY, &handler);
	}
	else
	{
		d->packagePath = std::string(path_temp);
		createGamePackage(this, GPT_ZIP, &handler);
	}

	d->handler.reset(handler);
	d->handler->init();
}

bool GMGamePackage::readFile(GMPackageIndex index, const GMString& filename, REF GMBuffer* buffer, REF GMString* fullFilename)
{
	D(d);
	GMString p = pathOf(index, filename);
	if (fullFilename)
		*fullFilename = p;
	return readFileFromPath(p, buffer);
}

void GMGamePackage::beginReadFile(GMPackageIndex index, const GMString& filename, GMAsyncCallback callback, OUT IAsyncResult** ar, REF GMString* fullFilename)
{
	GMString p = pathOf(index, filename);
	if (fullFilename)
		*fullFilename = p;
	return beginReadFileFromPath(p, callback, ar);
}

Vector<GMString> GMGamePackage::getAllFiles(const GMString& directory)
{
	D(d);
	GM_ASSERT(d->handler);
	return d->handler->getAllFiles(directory);
}

GMString GMGamePackage::pathOf(GMPackageIndex index, const GMString& filename)
{
	D(d);
	GM_ASSERT(d->handler);
	return d->handler->pathRoot(index) + filename;
}

bool GMGamePackage::readFileFromPath(const GMString& path, REF GMBuffer* buffer)
{
	D(d);
	GM_ASSERT(d->handler);
	bool b = d->handler->readFileFromPath(path, buffer);
	gm_hook2(GMGamePackage, readFileFromPath, &path, buffer);
	return b;
}

void GMGamePackage::beginReadFileFromPath(const GMString& path, GMAsyncCallback& callback, OUT IAsyncResult** ar)
{
	D(d);
	d->handler->beginReadFileFromPath(path, callback, ar);
}

void GMGamePackage::createGamePackage(GMGamePackage* pk, GamePackageType t, OUT IGamePackageHandler** handler)
{
	switch (t)
	{
	case gm::GPT_DIRECTORY:
	{
		GMDefaultGamePackageHandler* h = new GMDefaultGamePackageHandler(pk);
		*handler = h;
	}
	break;
	case gm::GPT_ZIP:
	{
		GMZipGamePackageHandler* h = new GMZipGamePackageHandler(pk);
		*handler = h;
	}
	break;
	default:
		GM_ASSERT(false);
		break;
	}
}

GMGamePackageAsyncResult::~GMGamePackageAsyncResult()
{
	D(d);
	delete d->thread;
}

GMObject* GMGamePackageAsyncResult::state()
{
	D(d);
	return &d->buffer;
}

bool GMGamePackageAsyncResult::isComplete()
{
	D(d);
	return d->thread->isDone();
}

GMEvent& GMGamePackageAsyncResult::waitHandle()
{
	D(d);
	return d->thread->waitEvent();
}
