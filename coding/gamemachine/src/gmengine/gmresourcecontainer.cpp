#include "stdafx.h"
#include "gmresourcecontainer.h"

GMTextureContainer::~GMTextureContainer()
{
	D(d);
	for (auto& texture : d->textures)
	{
		delete texture.texture;
	}
}

void GMTextureContainer::insert(GMTextureContainer::TextureItemType& item)
{
	D(d);
	ASSERT(d->textures.find(item) == d->textures.end());
	d->textures.insert(item);
}

const GMTextureContainer::TextureItemType* GMTextureContainer::find(const char* name)
{
	D(d);
	GMTextureContainer::TextureItemType _key;
	_key.id = name;
	auto it = d->textures.find(_key);
	if (it == d->textures.end())
		return nullptr;

	return &(*it);
}

GMTextureContainer_ID::~GMTextureContainer_ID()
{
	D(d);
	for (auto& texture : d->textures)
	{
		delete texture.texture;
	}
}

void GMTextureContainer_ID::insert(GMTextureContainer_ID::TextureItemType& item)
{
	D(d);
	ASSERT(d->textures.find(item) == d->textures.end());
	d->textures.insert(item);
}

const GMTextureContainer_ID::TextureItemType* GMTextureContainer_ID::find(GMint id)
{
	D(d);
	GMTextureContainer_ID::TextureItemType _key = { id };
	auto it = d->textures.find(_key);
	if (it == d->textures.end())
		return nullptr;

	return &(*it);
}

GMModelContainer::~GMModelContainer()
{
	D(d);
	for (auto& item : d->items)
	{
		if (item.second)
			delete item.second;
	}
}

GMModelContainerItemIndex GMModelContainer::insert(AUTORELEASE GMModel* model)
{
	D(d);
	d->items[d->id] = model;
	GMModelContainerItemIndex r;
	r.id = d->id++;
	return r;
}

GMModel* GMModelContainer::find(GMModelContainerItemIndex index)
{
	D(d);
	GMuint _key = index.id;
	auto it = d->items.find(_key);
	if (it == d->items.end())
		return nullptr;

	return it->second;
}