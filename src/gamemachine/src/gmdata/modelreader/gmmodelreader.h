#ifndef __MODELREADER_H__
#define __MODELREADER_H__
#include <gmcommon.h>
#include <linearmath.h>
#include <gmgameobject.h>
#include <gmassets.h>
BEGIN_NS

class GMModel;
struct GMBuffer;
struct GMModelLoadSettings;
GM_INTERFACE(IModelReader)
{
	virtual ~IModelReader() {}
	virtual bool load(const GMModelLoadSettings& settings, GMBuffer& buffer, REF GMAsset& asset) = 0;
	virtual bool test(const GMBuffer& buffer) = 0;
};

class GMGamePackage;

//! 目录路径参考类型
/*!
  目录路径参考类型决定了模型读取器如何读取模型。
  \sa GMModelLoadSettings, GMModelReader
*/
enum class GMModelPathType
{
	Relative, //!< 采取相对路径来读取模型。GMGamePackage的模型路径中读取模型。GMGamePackage一定要初始化才可以使用本参考类型。
	Absolute, //!< 采取绝对路径来读取模型。
};

//! 读取一个模型的配置
/*!
  通过给GMModelReader::load传入一个读取配置，可以让GMModelReader正确读取一个模型。
  模型读取器通过GMGamePackage读取模型，在type=GMModelPathType::Relative的情况下，一定要使得GMGamePackage初始化，才能
  读取模型。
  \sa GMGamePackage, GMModelReader
*/
struct GMModelLoadSettings
{
	GMModelLoadSettings() = default;
	GMModelLoadSettings(
		const GMString& filename_,
		const GMString& directory_,
		const IRenderContext* context_,
		GMModelPathType type_ = GMModelPathType::Relative,
		bool flipTexcoords_ = true
	)
		: filename(filename_)
		, directory(directory_)
		, context(context_)
		, type(type_)
		, flipTexcoords(flipTexcoords_)
	{
	}

	GMString filename; //!< 需要被读取的模型文件
	GMString directory; //!< 模型所在目录
	const IRenderContext* context;
	GMModelPathType type; //!< 目录路径参考类型
	bool flipTexcoords; //!< 是否颠倒纹理坐标的y轴
};

class GMModelReader
{
public:
	enum ModelType
	{
		Auto,
		ModelType_Begin,
		Object = ModelType_Begin,
		GMMd5,
		AnyOther,
		ModelType_End,
	};

public:
	static bool load(const GMModelLoadSettings& settings, REF GMAsset& asset);
	static bool load(const GMModelLoadSettings& settings, ModelType type, REF GMAsset& asset);
	static IModelReader* getReader(ModelType type);

private:
	static ModelType test(const GMBuffer& buffer);
};

END_NS
#endif
