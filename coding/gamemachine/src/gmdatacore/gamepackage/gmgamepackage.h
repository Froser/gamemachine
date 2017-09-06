#ifndef __GAMEPACKAGE_H__
#define __GAMEPACKAGE_H__
#include <gmcommon.h>
#include <utilities.h>
#include <gmthreads.h>
BEGIN_NS

enum class GMPackageIndex
{
	Maps,
	Shaders,
	TexShaders,
	Textures,
	Models,
	Sounds,
	Scripts,
};

class GMBSPGameWorld;
struct IGamePackageHandler
{
	virtual ~IGamePackageHandler() {}
	virtual void init() = 0;
	virtual bool readFileFromPath(const GMString& path, REF GMBuffer* buffer) = 0;
	virtual void beginReadFileFromPath(const GMString& path, GMAsyncCallback& callback, OUT IAsyncResult** ar) = 0;
	virtual GMString pathRoot(GMPackageIndex index) = 0;
	virtual Vector<GMString> getAllFiles(const GMString& directory) = 0;
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
	bool readFile(GMPackageIndex index, const GMString& filename, REF GMBuffer* buffer, REF GMString* fullFilename = nullptr);
	void beginReadFile(GMPackageIndex index, const GMString& filename, GMAsyncCallback callback, OUT IAsyncResult** ar, REF GMString* fullFilename = nullptr);
	Vector<GMString> getAllFiles(const GMString& directory);

public:
	GMString pathOf(GMPackageIndex index, const GMString& filename);
	// 一般情况下，建议用readFile代替readFileFromPath，除非是想指定特殊的路径
	bool readFileFromPath(const GMString& path, REF GMBuffer* buffer);
	void beginReadFileFromPath(const GMString& path, GMAsyncCallback& callback, OUT IAsyncResult** ar);
};

GM_PRIVATE_OBJECT(GMGamePackageAsyncResult)
{
	GMThread* thread;
	GMBuffer buffer;
};

class GMGamePackageAsyncResult : public IAsyncResult
{
	DECLARE_PRIVATE(GMGamePackageAsyncResult);

public:
	GMGamePackageAsyncResult() = default;
	~GMGamePackageAsyncResult();

public:
	virtual GMObject* state() override;
	virtual bool isComplete() override;
	virtual GMEvent& waitHandle() override;

public:
	void setThread(AUTORELEASE GMThread* thread) { D(d); d->thread = thread; }
};

END_NS
#endif
