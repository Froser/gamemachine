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
	Texture,
	Model,
};

struct GMAsset
{
	GMAssetType type;
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

// 使用一种（多叉）树状结构，保存游戏中的资产
// 资产的根节点叫作root
#define ASSET_GETTER(retType, funcName, predictType)	\
	static retType funcName(const GMAsset& asset) {			\
		if (asset.type == predictType)					\
			return static_cast<retType>(asset.asset);	\
		return nullptr;									\
	}

struct GMAssetsNode;
struct GMAssetsNode
{
	GMAssetName name;
	GMAssetsNode* next = nullptr;
	GMAssetsNode* child = nullptr;
	GMAsset asset;
};

GM_PRIVATE_OBJECT(GMAssets)
{
	GMAssetsNode root;
};

class GMAssets : public GMObject
{
	DECLARE_PRIVATE(GMAssets)

public:
	GMAssets() = default;

public:
	ASSET_GETTER(ITexture*, getTexture, GMAssetType::Texture);
	ASSET_GETTER(GMModel*, getModel, GMAssetType::Model);

public:
	GMAsset* insertAssert(const char* path, GMAssetType type, void* asset);
	GMAsset* insertAssert(const char* path, const GMAssetName& name, GMAssetType type, void* asset);
	GMAssetsNode* getNodeFromPath(const char* path, bool createIfNotExists = false);

public:
	static GMAsset createIsolatedAsset(GMAssetType type, void* data);
	static GMAssetsNode* findChild(GMAssetsNode* node, const GMAssetName& name, bool createIfNotExists = false);
	static GMString combinePath(std::initializer_list<GMString> args);

private:
	static GMAssetsNode* getNodeFromPath(GMAssetsNode* node, const char* path, bool createIfNotExists = false);
	static void nextAvailableName(GMAssetsNode* node, REF char* name);
	static void clearChildNode(GMAssetsNode* node);
};

END_NS
#endif