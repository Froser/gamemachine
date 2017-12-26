#ifndef __GAMEPACKAGE_H__
#define __GAMEPACKAGE_H__
#include <gmcommon.h>
#include <tools.h>
#include <gmthreads.h>
BEGIN_NS

enum class GMGamePackageType
{
	Directory,
	Zip,
};

//! 游戏资源类型
/*!
  游戏资源类型决定了获取资源文件的路径。
*/
enum class GMPackageIndex
{
	Maps, //!< 地图类型，存放在"资源包/maps"下
	Shaders, //!< 着色器程序类型，如GLSL程序，存放在"资源包/shaders"下
	TexShaders, //!< 特效类型，存放在"资源包/texshaders"下
	Textures, //!< 纹理类型，存放在"资源包/textures"下
	Models, //!< 模型类型，存放在"资源包/models"下
	Audio, //!< 声音文件类型，存放在"资源包/audio"下
	Scripts, //!< 脚本类型，存放在"资源包/scripts"下
};

class GMBSPGameWorld;
GM_INTERFACE(IGamePackageHandler)
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
};

class GMBSPGameWorld;

//! 游戏资源包管理器。
/*!
  使用游戏资源包管理器，可以很方便在资源包中读取原始数据。
  资源包可以是一个文件夹，也可以是一个zip压缩包，取决于读取资源包时传入的资源类型。
*/
class GMGamePackage : public GMObject
{
	DECLARE_PRIVATE(GMGamePackage)

	friend class GameMachine;

public:
	//! 获取资源包管理器私有成员。
	/*!
	  不建议IGamePackageHandler派生类以外的类使用此方法。
	  \return 资源包管理器私有成员。
	*/
	Data* gamePackageData();

	//! 从路径读取资源包。
	/*!
	  从指定路径读取资源。
	  如果路径是一个文件夹，资源包管理器将使用文件夹模式，所有资源将从系统磁盘中读取。文件的完整路径即获取文件的绝对路径。
	  如果路径是一个zip文件，资源包管理器将使用zip模式，所有资源将从zip包中读取。文件的完整路径即相对于zip根目录(/)的一个路径。
	  \param path 读取资源的路径。
	*/
	void loadPackage(const GMString& path);

	//! 读取某个资源。
	/*!
	  在读取资源之前，确保loadPackage()被正常调用。
	  此方法会读取在"资源包路径/类型/文件名"下的资源。
	  此方法将会调用readFileFromPath，触发它当中的钩子。
	  \param index 资源类型。此方法根据资源类型，在资源包根路径下寻找子路径。
	  \param filename 资源的文件名。
	  \param fullFilename 获取资源的完整文件名（包含路径）。
	  \return 资源是否成功读取。
	  \sa loadPackage(), readFileFromPath()
	*/
	bool readFile(GMPackageIndex index, const GMString& filename, REF GMBuffer* buffer, REF GMString* fullFilename = nullptr);

	void beginReadFile(GMPackageIndex index, const GMString& filename, GMAsyncCallback callback, OUT IAsyncResult** ar, REF GMString* fullFilename = nullptr);

	//! 获取资源包指定路径下的所有文件路径。
	/*!
	  调用此方法之前，确保loadPackage()被正常调用。
	  \return 得到的所有文件路径。
	  \sa loadPackage()
	*/
	Vector<GMString> getAllFiles(const GMString& directory);

	//! 获取指定类型的指定文件的完整路径。
	/*!
	  GameMachine建议将同种资源类型的文件（如声音文件、图像文件、着色器程序等），分别放在它们对于的文件夹内。
	  如声音文件放在资源包的audio文件夹内，地图文件放在资源包的maps文件夹内。
	  通过此方法，指定资源类型和文件名，可以得到资源文件的路径。
	  对于文件夹类型的资源包，将会得到一个完整路径；对于zip类型的资源包，将会得到相对于zip根目录(/)的一个路径。
	  \param index 资源类型。
	  \param filename 待获取路径的文件名。
	  \return 文件名的路径。
	*/
	GMString pathOf(GMPackageIndex index, const GMString& filename);

	//! 从完整路径读取资源。此方法包含钩子。
	/*!
	  如果资源包是文件夹类型，完整路径表示从盘符根目录开始的绝对路径。如果资源包是zip类型，完整路径则表示zip根目录(/)
	  的路径。
	  一般情况下，不建议使用readFileFromPath()，而是使用readFile()来读取文件。readFile()本质上也是通过拼凑出一个资
	  源路径，再来调用此方法获取资源。但是，直接调用readFileFromPath()会破坏对资源路径读取的封装。GameMachine建议使
	  用通过GMPackageIndex来寻找资源。
	  \return 资源是否成功读取。
	  \sa GMPackageIndex, readFile()
	  \hook GMGamePackage_readFileFromPath
	*/
	bool readFileFromPath(const GMString& path, REF GMBuffer* buffer);
	void beginReadFileFromPath(const GMString& path, GMAsyncCallback& callback, OUT IAsyncResult** ar);

protected:
	virtual void createGamePackage(GMGamePackage* pk, GMGamePackageType t, OUT IGamePackageHandler** handler);
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
	virtual GMBuffer* state() override;
	virtual bool isComplete() override;
	virtual GMEvent& waitHandle() override;

public:
	void setThread(AUTORELEASE GMThread* thread) { D(d); d->thread = thread; }
};

END_NS
#endif
