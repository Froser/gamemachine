#include "stdafx.h"
#include "gmassets.h"

GMAssets::GMAssets()
{
	// 创建默认目录
	createNodeFromPath(GM_ASSET_MODELS);
	createNodeFromPath(GM_ASSET_TEXTURES);
	createNodeFromPath(GM_ASSET_LIGHTMAPS);
}

GMAssets::~GMAssets()
{
	D(d);
	clearChildNode(&d->root);
}

GMAsset* GMAssets::insertAsset(const char* path, GMAssetType type, void* asset)
{
	GMAssetsNode* node = nullptr;
	char name[GMAssetName::NAME_MAX];

	// 使用匿名的asset
	node = getNodeFromPath(path, true);
	node = findLastChild(node);
	GM_ASSERT(node);
	node->name = name;
	node->asset.type = type;
	node->asset.asset = asset;
	return &node->asset;
}

GMAsset* GMAssets::insertAsset(const char* path, const GMAssetName& name, GMAssetType type, void* asset)
{
	GMAssetsNode* node = nullptr;
	// 把(a/b, c/d)字符串类型的参数改写为(a/b/c, d)
	char newPath[GMAssetName::NAME_MAX], newName[GMAssetName::NAME_MAX];
	combinePath({ path, name.name.data() }, newPath, newName);
	gm_info("Adding asset: path=%s, name=%s", newPath, newName);
	node = getNodeFromPath(newPath, true);
	node = findChild(node, newName, true);
	GM_ASSERT(node);
	node->name = newName;
	node->asset.type = type;
	node->asset.asset = asset;
	return &node->asset;
}

static bool splash(char in)
{
	return in == '/' || in == '\\';
}

GMAssetsNode* GMAssets::getNodeFromPath(const char* path, bool createIfNotExists)
{
	D(d);
	GMAssetsNode* node = &d->root;
	return getNodeFromPath(node, path, createIfNotExists);
}

void GMAssets::createNodeFromPath(const char* path)
{
	getNodeFromPath(path, true);
}

GMAsset GMAssets::createIsolatedAsset(GMAssetType type, void* data)
{
	GMAsset ret;
	ret.type = type;
	ret.asset = data;
	return ret;
}

GMAssetsNode* GMAssets::findChild(GMAssetsNode* parentNode, const GMAssetName& name, bool createIfNotExists)
{
	Scanner s(name, splash);
	if (!parentNode)
		return nullptr;

	GMAssetsNode *node = parentNode->child, *last = nullptr;
	while (node)
	{
		if (node->name == name)
			return node;
		last = node;
		node = node->next;
	}

	GM_ASSERT(!node);
	if (createIfNotExists)
	{
		if (!last)
		{
			parentNode->child = new GMAssetsNode();
			parentNode->child->name = name;
			return parentNode->child;
		}
		else
		{
			last->next = new GMAssetsNode();
			last->next->name = name;
			return last->next;
		}
	}

	GM_ASSERT(!node);
	return node; // node == nullptr
}

GMAssetsNode* GMAssets::findLastChild(GMAssetsNode* parentNode, bool createIfNotExists)
{
	GMAssetsNode* node = parentNode->child;
	if (!node)
	{
		if (createIfNotExists)
		{
			parentNode->child = new GMAssetsNode();
			return parentNode;
		}
		return nullptr;
	}
	else
	{
		while (node)
		{
			node = node->next;
		}
		return node;
	}
}

GMString GMAssets::combinePath(std::initializer_list<GMString> args, REF char* path, REF char* lastPart)
{
	GMString result;
	char name[GMAssetName::NAME_MAX];
	for (auto arg : args)
	{
		std::string l = arg.toStdString();
		Scanner s(l.c_str(), splash);
		s.next(name);
		while (*name)
		{
			if (lastPart)
				strcpy_s(lastPart, GMAssetName::NAME_MAX, name);
			if (path)
				strcpy_s(path, GMAssetName::NAME_MAX, result.toStdString().c_str());
			
			result += name;
			result += "/";
			s.next(name);
		}
	}
	return result;
}

GMAssetsNode* GMAssets::getNodeFromPath(GMAssetsNode* beginNode, const char* path, bool createIfNotExists)
{
	Scanner s(path, splash);
	GMAssetsNode* node = beginNode;
	if (!node)
		return node;

	char sep[LINE_MAX];
	s.next(sep);
	while (*sep)
	{
		node = findChild(node, sep, createIfNotExists);
		if (!node)
			return nullptr;
		s.next(sep);
	}
	return node;
}

void GMAssets::clearChildNode(GMAssetsNode* parentNode)
{
	GMAssetsNode* node = parentNode->child;
	while (node)
	{
		if (node->child)
			clearChildNode(node);
		switch (node->asset.type)
		{
		case GMAssetType::Texture:
			delete getTexture(parentNode->asset);
			break;
		case GMAssetType::Model:
			delete getModel(parentNode->asset);
			break;
		default:
			GM_ASSERT(false);
			break;
		}
		node = node->next;
	}
}