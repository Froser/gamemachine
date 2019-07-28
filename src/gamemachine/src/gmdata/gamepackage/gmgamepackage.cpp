#include "stdafx.h"
#include "gmgamepackage.h"
#include "gmgamepackagehandler.h"
#include "gmgamepackage_p.h"
#include <sys/stat.h>

BEGIN_NS
GMGamePackage::GMGamePackage()
{
	GM_CREATE_DATA();
}

GMGamePackage::~GMGamePackage()
{

}

GMGamePackage::Data* GMGamePackage::gamePackageData()
{
	D(d);
	return d;
}

void GMGamePackage::loadPackage(const GMString& path)
{
	D(d);
	size_t len = path.length();
	char path_temp[FILENAME_MAX];
	GMString::stringCopy(path_temp, path.toStdString().c_str());
	if (path_temp[len - 1] == '/' || path_temp[len - 1] == '\\')
		*(path_temp + len - 1) = 0;

	struct stat s;
	stat(path_temp, &s);

	IGamePackageHandler* handler = nullptr;
	if ((s.st_mode & S_IFMT) == S_IFDIR)
	{
		// 读取整个目录
		d->packagePath = std::string(path_temp);
		createGamePackage(this, GMGamePackageType::Directory, &handler);
	}
	else
	{
		d->packagePath = std::string(path_temp);
		createGamePackage(this, GMGamePackageType::Zip, &handler);
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

GMString GMGamePackage::pathOf(GMPackageIndex index, const GMString& filename)
{
	D(d);
	GM_ASSERT(d->handler);
	return d->handler->pathOf(index, filename);
}

bool GMGamePackage::readFileFromPath(const GMString& path, REF GMBuffer* buffer)
{
	D(d);
	GM_ASSERT(d->handler);
	bool b = d->handler->readFileFromPath(path, buffer);
	hook<const GMString&, GMBuffer*>("GMGamePackage_readFileFromPath", path, buffer);
	return b;
}

bool GMGamePackage::exists(GMPackageIndex index, const GMString& filename)
{
	D(d);
	GM_ASSERT(d->handler);
	return d->handler->exists(index, filename);
}

void GMGamePackage::createGamePackage(GMGamePackage* pk, GMGamePackageType t, OUT IGamePackageHandler** handler)
{
	switch (t)
	{
	case GMGamePackageType::Directory:
	{
		GMDefaultGamePackageHandler* h = new GMDefaultGamePackageHandler(pk);
		*handler = h;
	}
	break;
	case GMGamePackageType::Zip:
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
END_NS