#include "stdafx.h"
#include "gmassets.h"

GMAsset* GMAssets::insertAssert(const char* path, GMAssetType type, void* asset)
{
	GMAssetsNode* node = nullptr;
	char name[LINE_MAX];

	// 使用匿名的asset
	node = getNodeFromPath(path);
	nextAvailableName(node, name);
	node = findChild(node, name, true);
	node->name = name;
	node->asset.type = type;
	node->asset.asset = asset;
	return &node->asset;
}

GMAsset* GMAssets::insertAssert(const char* path, const GMAssetName& name, GMAssetType type, void* asset)
{
	GMAssetsNode* node = nullptr;
	node = getNodeFromPath(path);
	// 把(a/b, c/d)字符串类型的参数改写为(a/b/c, d)
	// TODO
	node = findChild(node, name, true);
	node->name = name;
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
	GMAssetsNode* node = d->root.child;
	return getNodeFromPath(node, path, createIfNotExists);
}

GMAsset GMAssets::createIsolatedAsset(GMAssetType type, void* data)
{
	GMAsset ret;
	ret.type = type;
	ret.asset = data;
	return ret;
}

GMAssetsNode* GMAssets::findChild(GMAssetsNode* node, const GMAssetName& name, bool createIfNotExists)
{
	Scanner s(name, splash);
	GMAssetsNode* last = nullptr;
	while (node)
	{
		if (node->name == name)
			return node;
		last = node;
		node = node->next;
	}

	GM_ASSERT(node == nullptr);
	if (createIfNotExists)
	{
		GM_ASSERT(last);
		last->next = new GMAssetsNode();
		last->next->name = name;
		return last->next;
	}

	GM_ASSERT(node == nullptr);
	return node; // node == nullptr
}

GMAssetsNode* GMAssets::getNodeFromPath(GMAssetsNode* beginNode, const char* path, bool createIfNotExists)
{
	Scanner s(path, splash);
	GMAssetsNode* node = beginNode;
	if (!node)
		return node;

	char sep[LINE_MAX];
	s.next(sep);
	while (sep)
	{
		node = findChild(node, sep, createIfNotExists);
		s.next(sep);
	}
	return node;
}

void GMAssets::nextAvailableName(GMAssetsNode* node, REF char* name)
{
	GMAssetsNode* n = node->child;
	GMuint id = 1;

	bool done = false;
	while (!done)
	{
		done = true;
		strcpy_s(name, LINE_MAX, ("__" + std::to_string(id)).c_str());
		while (n)
		{
			if (n->name == name)
			{
				done = false;
				break;
			}
			n = n->next;
		}
		++id;
	}
}

void GMAssets::clearChildNode(GMAssetsNode* node)
{
	if (node->child)
	{
		clearChildNode(node->child);
	}
	else
	{
		switch (node->asset.type)
		{
		case GMAssetType::Texture:
			delete getTexture(node->asset);
			break;
		case GMAssetType::Model:
			delete getModel(node->asset);
			break;
		default:
			GM_ASSERT(false);
			break;
		}
	}
}