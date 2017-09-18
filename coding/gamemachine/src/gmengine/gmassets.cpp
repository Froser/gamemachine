#include "stdafx.h"
#include "gmassets.h"

GMTextureAssets::~GMTextureAssets()
{
	D(d);
	for (auto& texture : d->textures)
	{
		delete texture.texture;
	}
}

void GMTextureAssets::insert(GMTextureAssets::TextureItemType& item)
{
	D(d);
	GM_ASSERT(d->textures.find(item) == d->textures.end());
	d->textures.insert(item);
}

const GMTextureAssets::TextureItemType* GMTextureAssets::find(const char* name)
{
	D(d);
	GMTextureAssets::TextureItemType _key;
	_key.id = name;
	auto it = d->textures.find(_key);
	if (it == d->textures.end())
		return nullptr;

	return &(*it);
}

GMTextureAssets_ID::~GMTextureAssets_ID()
{
	D(d);
	for (auto& texture : d->textures)
	{
		delete texture.texture;
	}
}

void GMTextureAssets_ID::insert(GMTextureAssets_ID::TextureItemType& item)
{
	D(d);
	GM_ASSERT(d->textures.find(item) == d->textures.end());
	d->textures.insert(item);
}

const GMTextureAssets_ID::TextureItemType* GMTextureAssets_ID::find(GMint id)
{
	D(d);
	GMTextureAssets_ID::TextureItemType _key = { id };
	auto it = d->textures.find(_key);
	if (it == d->textures.end())
		return nullptr;

	return &(*it);
}

GMModelAssets::~GMModelAssets()
{
	D(d);
	for (auto& item : d->items)
	{
		if (item.second)
			delete item.second;
	}
}

GMModelContainerItemIndex GMModelAssets::insert(AUTORELEASE GMModel* model)
{
	D(d);
	d->items[d->id] = model;
	GMModelContainerItemIndex r;
	r.id = d->id++;
	return r;
}

GMModel* GMModelAssets::find(GMModelContainerItemIndex index)
{
	D(d);
	GMuint _key = index.id;
	auto it = d->items.find(_key);
	if (it == d->items.end())
		return nullptr;

	return it->second;
}