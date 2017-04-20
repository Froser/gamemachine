#ifndef __GAMEPACKAGE_H__
#define __GAMEPACKAGE_H__
#include "common.h"
#include <string>
#include "utilities/autoptr.h"
BEGIN_NS

struct IGamePackageHandler
{
	virtual void init() = 0;
	virtual GMbyte* readFileFromPath(const char* path) = 0;
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
	void loadPackage(const char* path);
	void createBSPGameWorld(const char* bspPath, OUT BSPGameWorld** gameWorld);
};

END_NS
#endif