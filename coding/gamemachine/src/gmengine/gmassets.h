#ifndef __GMASSETS_H__
#define __GMASSETS_H__
#include <gmcommon.h>
#include <set>
#include "../gmdatacore/gmmodel.h"
BEGIN_NS

// 默认的一些资产路径
#define GM_ASSET_TEXTURES	"/textures"
#define GM_ASSET_LIGHTMAPS	"/lightmaps"
#define GM_ASSET_MODELS		"/models"

enum class GMAssetType
{
	None,
	Texture,
	Model,
};

struct GMAsset
{
	GMAssetType type = GMAssetType::None;
	void* asset = nullptr;
};

struct GMAssetName
{
	enum
	{
		NAME_MAX = 128
	};

	Array<char, 128> name;

	operator const char*() const
	{
		return name.data();
	}

	bool operator ==(const char* n)
	{
		return strEqual(name.data(), n);
	}

	GMAssetName(const char* n)
	{
		strcpy_s(name.data(), NAME_MAX, n);
	}

	GMAssetName() = default;

	GMAssetName(const GMAssetName& rhs)
	{
		strcpy_s(name.data(), NAME_MAX, rhs.name.data());
	}
};

struct GMAssetNameCmp
{
	bool operator ()(const GMAssetName& left, const GMAssetName& right) const
	{
		return strcmp(left.name.data(), right.name.data()) < 0;
	}
};

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
	GMAssetName name;
	Multimap<GMAssetName, GMAssetsNode*, GMAssetNameCmp> childs;
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

public:
	GMAsset insertAsset(GMAssetType type, void* asset);
	GMAsset insertAsset(const char* path, const GMAssetName& name, GMAssetType type, void* asset);
	GMAssetsNode* getNodeFromPath(const char* path, bool createIfNotExists = false);
	void createNodeFromPath(const char* path);

public:
	static GMAsset createIsolatedAsset(GMAssetType type, void* data);
	static GMAssetsNode* findChild(GMAssetsNode* parentNode, const GMAssetName& name, bool createIfNotExists = false);
	static GMAssetsNode* makeChild(GMAssetsNode* parentNode, const GMAssetName& name);
	static GMString combinePath(std::initializer_list<GMString> args, REF char* path = nullptr, REF char* lastPart = nullptr);

private:
	static GMAssetsNode* getNodeFromPath(GMAssetsNode* node, const char* path, bool createIfNotExists = false);
	static void clearChildNode(GMAssetsNode* self);
	static void deleteAsset(GMAssetsNode* node);

private:
	void clearOrphans();
};

END_NS
#endif