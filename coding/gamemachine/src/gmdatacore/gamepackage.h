#ifndef __GAMEPACKAGE_H__
#define __GAMEPACKAGE_H__
#include "common.h"
#include <string>
#include "utilities/autoptr.h"
BEGIN_NS

enum PackageIndex
{
	PI_MAPS,
	PI_SHADERS,
	PI_TEXSHADERS,
	PI_TEXTURES,
};

class BSPGameWorld;
struct IGamePackageHandler
{
	virtual void init() = 0;
	virtual void readFileFromPath(const char* path, OUT GMbyte** buffer) = 0;
	virtual std::string pathRoot(PackageIndex index) = 0;
};

struct IFactory;
class GameMachine;
struct GamePackagePrivate
{
	std::string packagePath;
	AutoPtr<IGamePackageHandler> handler;
	IFactory* factory;
	GameMachine* gameMachine;
};

class BSPGameWorld;
typedef GamePackagePrivate GamePackageData;
class GamePackage
{
	DEFINE_PRIVATE(GamePackage)

public:
	GamePackage(GameMachine* gm, IFactory* factory);

public:
	GamePackageData& gamePackageData();
	IGamePackageHandler* getHandler();
	void loadPackage(const char* path);
	void createBSPGameWorld(const char* map, OUT BSPGameWorld** gameWorld);
	std::string path(PackageIndex index, const char* filename);
};

END_NS
#endif