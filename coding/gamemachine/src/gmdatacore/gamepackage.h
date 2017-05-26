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

struct GamePackageBuffer
{
	GamePackageBuffer()
		: buffer(nullptr)
		, size(0)
		, needRelease(false)
	{
	}

	~GamePackageBuffer()
	{
		if (needRelease)
		{
			ASSERT(buffer);
			delete[] buffer;
		}
	}

	GamePackageBuffer& operator =(const GamePackageBuffer& rhs)
	{
		this->needRelease = rhs.needRelease;
		this->size = rhs.size;
		buffer = new GMbyte[this->size];
		memcpy(buffer, rhs.buffer, this->size);
		return *this;
	}

	void convertToStringBuffer()
	{
		GMbyte* newBuffer = new GMbyte[size + 1];
		memcpy(newBuffer, buffer, size);
		newBuffer[size] = 0;
		size++;
		if (needRelease && buffer)
			delete[] buffer;
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
	virtual AlignedVector<std::string> getAllFiles(const char* directory) = 0;
};

struct IFactory;
GM_PRIVATE_OBJECT(GamePackage)
{
	std::string packagePath;
	AutoPtr<IGamePackageHandler> handler;
	IFactory* factory;
	GameMachine* gameMachine;
};

class BSPGameWorld;
class GamePackage
{
	DECLARE_PRIVATE(GamePackage)

public:
	GamePackage(GameMachine* gm, IFactory* factory);

public:
	Data* gamePackageData();
	void loadPackage(const char* path);
	void createBSPGameWorld(const char* map, OUT BSPGameWorld** gameWorld);
	bool readFile(PackageIndex index, const char* filename, REF GamePackageBuffer* buffer, REF std::string* fullFilename = nullptr);
	AlignedVector<std::string> getAllFiles(const char* directory);

public:
	std::string pathOf(PackageIndex index, const char* filename);
	// 一般情况下，建议用readFile代替readFileFromPath，除非是想指定特殊的路径
	bool readFileFromPath(const char* path, REF GamePackageBuffer* buffer);
};

END_NS
#endif
