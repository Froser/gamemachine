#ifndef __GAMEPACKAGE_H__
#define __GAMEPACKAGE_H__
#include "common.h"
#include <string>
#include "utilities/autoptr.h"
#include "utilities/assert.h"
#include <vector>
BEGIN_NS

enum PackageIndex
{
	PI_MAPS,
	PI_SHADERS,
	PI_TEXSHADERS,
	PI_TEXTURES,
};

struct GamePackageBuffer
{
	GamePackageBuffer()
		: needRelease(false)
		, buffer(nullptr)
		, size(0)
	{
	}

	~GamePackageBuffer()
	{
		if (needRelease)
		{
			ASSERT(buffer);
			delete buffer;
		}
	}

	void convertToStringBuffer()
	{
		GMbyte* newBuffer = new GMbyte[size + 1];
		memcpy(newBuffer, buffer, size);
		newBuffer[size] = 0;
		size++;
		if (needRelease && buffer)
			delete buffer;
		needRelease = true;
		buffer = newBuffer;
	}

	GMbyte* buffer; // 从GamePackage读取出来的缓存
	GMuint size; // 缓存大小
	bool needRelease; // 表示是否需要手动释放
};

class BSPGameWorld;
struct IGamePackageHandler
{
	virtual ~IGamePackageHandler() {}
	virtual void init() = 0;
	virtual bool readFileFromPath(const char* path, REF GamePackageBuffer* buffer) = 0;
	virtual std::string pathRoot(PackageIndex index) = 0;
	virtual std::vector<std::string> getAllFiles(const char* directory) = 0;
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
	bool readFileFromPath(const char* path, REF GamePackageBuffer* buffer);
	std::string path(PackageIndex index, const char* filename);
	std::vector<std::string> getAllFiles(const char* directory);
};

END_NS
#endif