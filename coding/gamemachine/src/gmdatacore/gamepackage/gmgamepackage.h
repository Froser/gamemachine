#ifndef __GAMEPACKAGE_H__
#define __GAMEPACKAGE_H__
#include "common.h"
#include <string>
#include "foundation/utilities/utilities.h"
#include "foundation/vector.h"
BEGIN_NS

enum PackageIndex
{
	PI_MAPS,
	PI_SHADERS,
	PI_TEXSHADERS,
	PI_TEXTURES,
	PI_MODELS,
	PI_SOUNDS,
};

class GMBSPGameWorld;
struct IGamePackageHandler
{
	virtual ~IGamePackageHandler() {}
	virtual void init() = 0;
	virtual bool readFileFromPath(const GMString& path, REF GMBuffer* buffer) = 0;
	virtual GMString pathRoot(PackageIndex index) = 0;
	virtual AlignedVector<GMString> getAllFiles(const GMString& directory) = 0;
};

GM_PRIVATE_OBJECT(GMGamePackage)
{
	GMString packagePath;
	AutoPtr<IGamePackageHandler> handler;
	IFactory* factory;
};

class GMBSPGameWorld;
class GMGamePackage : public GMObject
{
	DECLARE_PRIVATE(GMGamePackage)

	friend class GameMachine;

	GMGamePackage(IFactory* factory);

public:
	Data* gamePackageData();
	void loadPackage(const GMString& path);
	void createBSPGameWorld(const GMString& map, OUT GMBSPGameWorld** gameWorld);
	bool readFile(PackageIndex index, const GMString& filename, REF GMBuffer* buffer, REF GMString* fullFilename = nullptr);
	AlignedVector<GMString> getAllFiles(const GMString& directory);

public:
	GMString pathOf(PackageIndex index, const GMString& filename);
	// 一般情况下，建议用readFile代替readFileFromPath，除非是想指定特殊的路径
	bool readFileFromPath(const GMString& path, REF GMBuffer* buffer);
};

END_NS
#endif
