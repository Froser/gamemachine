#ifndef __GMASSETS_H__
#define __GMASSETS_H__
#include <gmcommon.h>
#include <set>
#include <gmmodel.h>
#include <gmphysicsshape.h>
BEGIN_NS

// 默认的一些资产路径
#define GM_ASSET_TEXTURES	GMString(L"/textures")
#define GM_ASSET_LIGHTMAPS	GMString(L"/lightmaps")
#define GM_ASSET_MODELS		GMString(L"/models")

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

struct GMAsset
{
	GMAssetType type = GMAssetType::None;
	void* asset = nullptr;
};

inline bool operator ==(const GMAsset& a, const GMAsset& b)
{
	return a.type == b.type && a.asset == b.asset;
}

// 使用一种（多叉）树状结构，保存游戏中的资产
// 资产的根节点叫作root
#define ASSET_GETTER(retType, funcName, predictType)	\
	static retType funcName(const GMAsset& asset) {		\
		if (asset.type == predictType)					\
			return static_cast<retType>(asset.asset);	\
		return nullptr;									\
	}

struct GMAssetsNode;
struct GMAssetsNode
{
	GMString name;
	Multimap<GMString, GMAssetsNode*> childs;
	GMAsset asset;
};

GM_PRIVATE_OBJECT(GMAssets)
{
	GMAssetsNode* root = nullptr;
	Vector<GMAssetsNode*> orphans;
};

class GMAssets : public GMObject
{
	DECLARE_PRIVATE(GMAssets)

public:
	GMAssets();
	~GMAssets();

public:
	ASSET_GETTER(ITexture*, getTexture, GMAssetType::Texture);
	ASSET_GETTER(GMModel*, getModel, GMAssetType::Model);
	ASSET_GETTER(GMModels*, getModels, GMAssetType::Models);
	ASSET_GETTER(GMPhysicsShape*, getPhysicsShape, GMAssetType::PhysicsShape);

public:
	GMAsset insertAsset(const GMAsset& asset);
	GMAsset insertAsset(GMAssetType type, void* asset);
	GMAsset insertAsset(const GMString& path, const GMString& name, GMAssetType type, void* asset);
	GMAssetsNode* getNodeFromPath(const GMString& path, bool createIfNotExists = false);
	void createNodeFromPath(const GMString& path);

public:
	static GMAsset createIsolatedAsset(GMAssetType type, void* data);
	static GMAssetsNode* findChild(GMAssetsNode* parentNode, const GMString& name, bool createIfNotExists = false);
	static GMAssetsNode* makeChild(GMAssetsNode* parentNode, const GMString& name);
	static GMString combinePath(std::initializer_list<GMString> args, REF GMString* path = nullptr, REF GMString* lastPart = nullptr);

private:
	static GMAssetsNode* getNodeFromPath(GMAssetsNode* node, const GMString& path, bool createIfNotExists = false);
	static void clearChildNode(GMAssetsNode* self);
	static void deleteAsset(GMAssetsNode* node);

private:
	void clearOrphans();
};

END_NS
#endif