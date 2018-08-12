#ifndef __GMASSETS_H__
#define __GMASSETS_H__
#include <gmcommon.h>
#include <set>
#include <gmmodel.h>
#include <gmphysicsshape.h>
BEGIN_NS

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
	None, //!< 无类型，绝对不会用到
	Texture, //!< 纹理贴图类型
	Model, //!< 模型类型
	Models, //!< 一组模型类型
	PhysicsShape, //!< 物理形状类型
};

// 使用一种（多叉）树状结构，保存游戏中的资产
// 资产的根节点叫作root
#define GM_ASSET_GETTER(retType, funcName, predictType)		\
	retType funcName() {									\
		if (getType() == predictType)						\
			return static_cast<retType>(getAsset());		\
		return nullptr;										\
	}

GM_PRIVATE_OBJECT(GMAsset)
{
	GMAtomic<GMint>* ref = nullptr;
	GMAssetType type = GMAssetType::None;
	void* asset = nullptr;
};

class GMAsset
{
	GM_DECLARE_PRIVATE_NGO(GMAsset)
	GM_DECLARE_PROPERTY(Type, type, GMAssetType)
	GM_ASSET_GETTER(ITexture*, getTexture, GMAssetType::Texture);
	GM_ASSET_GETTER(GMModel*, getModel, GMAssetType::Model);
	GM_ASSET_GETTER(GMModels*, getModels, GMAssetType::Models);
	GM_ASSET_GETTER(GMPhysicsShape*, getPhysicsShape, GMAssetType::PhysicsShape);

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

private:
	void addRef();
	void release();
	void removeData();
};

inline bool operator ==(const GMAsset& a, const GMAsset& b)
{
	return a.getType() == b.getType() && a.getAsset() == b.getAsset();
}

GM_PRIVATE_OBJECT(GMAssets)
{
	Vector<GMAsset> unnamedAssets;
	HashMap<GMString, GMAsset, GMStringHashFunctor> childs;
};

class GMAssets : public GMObject
{
	GM_DECLARE_PRIVATE(GMAssets)

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