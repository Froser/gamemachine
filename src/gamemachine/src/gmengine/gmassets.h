#ifndef __GMASSETS_H__
#define __GMASSETS_H__
#include <gmcommon.h>
BEGIN_NS

struct ITexture;
class GMModel;
class GMModels;
class GMPhysicsShape;

// 默认的一些资产路径
#define GM_ASSET_TEXTURES	GMString(L"/textures/")
#define GM_ASSET_LIGHTMAPS	GMString(L"/lightmaps/")
#define GM_ASSET_MODELS		GMString(L"/models/")

//! 游戏资产的类型
/*!
  表示某资产的类型。
*/
enum class GMAssetType
{
	Invalid, //!< 无类型，绝对不会用到
	Texture, //!< 纹理贴图类型
	Model, //!< 模型类型
	Models, //!< 一组模型类型
	PhysicsShape, //!< 物理形状类型
};

#define GM_DECLARE_ASSET_GETTER(retType, funcName) \
	retType funcName();

#define GM_DEFINE_ASSET_GETTER(retType, funcName, predictType)		\
	retType GMAsset::funcName() {									\
		if (getType() == predictType)								\
			return static_cast<retType>(getAsset());				\
		return nullptr;												\
	}

GM_PRIVATE_OBJECT_UNALIGNED(GMAsset)
{
	GMAtomic<GMlong>* ref = nullptr;
	GMAssetType type = GMAssetType::Invalid;
	void* asset = nullptr;
};

class GMAsset
{
	GM_DECLARE_PRIVATE_NGO(GMAsset)

public:
	GM_DECLARE_ASSET_GETTER(ITexture*, getTexture);
	GM_DECLARE_ASSET_GETTER(GMModel*, getModel);
	GM_DECLARE_ASSET_GETTER(GMModels*, getModels);
	GM_DECLARE_ASSET_GETTER(GMPhysicsShape*, getPhysicsShape);

public:
	GMAsset();
	GMAsset(GMAssetType type, void* asset);
	GMAsset(const GMAsset& asset);
	GMAsset(GMAsset&& asset) GM_NOEXCEPT;
	~GMAsset();

	GMAsset& operator=(const GMAsset& asset);
	GMAsset& operator=(GMAsset&& asset) GM_NOEXCEPT;

public:
	inline void* getAsset() const GM_NOEXCEPT
	{
		D(d);
		return d->asset;
	}

	inline bool isEmpty() const GM_NOEXCEPT
	{
		D(d);
		return !d->asset;
	}

	inline GMAssetType getType() const GM_NOEXCEPT
	{
		D(d);
		return d->type;
	}

	inline void setType(GMAssetType type) GM_NOEXCEPT
	{
		D(d);
		d->type = type;
	}

	template <typename T>
	T get()
	{
		return static_cast<T>(getAsset());
	}

private:
	void addRef();
	void release();
	void removeData();

public:
	static const GMAsset& invalidAsset();
};

inline bool operator ==(const GMAsset& a, const GMAsset& b) GM_NOEXCEPT
{
	return a.getType() == b.getType() && a.getAsset() == b.getAsset();
}

inline bool operator !=(const GMAsset& a, const GMAsset& b) GM_NOEXCEPT
{
	return !(a == b);
}

GM_PRIVATE_OBJECT_UNALIGNED(GMAssets)
{
	Vector<GMAsset> unnamedAssets;
	HashMap<GMString, GMAsset, GMStringHashFunctor> childs;
};

class GMAssets
{
	GM_DECLARE_PRIVATE_NGO(GMAssets)

public:
	GMAssets() = default;
	~GMAssets() = default;

public:
	GMAsset addAsset(GMAsset asset);
	GMAsset addAsset(const GMString& name, GMAsset asset);
	GMAsset getAsset(GMsize_t index);
	GMAsset getAsset(const GMString& name);
};

END_NS
#endif