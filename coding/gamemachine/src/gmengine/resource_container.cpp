#include "stdafx.h"
#include "resource_container.h"

TextureContainer::~TextureContainer()
{
	D(d);
	for (auto& texture : d->textures)
	{
		delete texture.texture;
	}
}

void TextureContainer::insert(TextureContainer::TextureItemType& item)
{
	D(d);
	ASSERT(d->textures.find(item) == d->textures.end());
	d->textures.insert(item);
}

const TextureContainer::TextureItemType* TextureContainer::find(const char* name)
{
	D(d);
	TextureContainer::TextureItemType _key;
	_key.id = name;
	auto it = d->textures.find(_key);
	if (it == d->textures.end())
		return nullptr;

	return &(*it);
}

TextureContainer_ID::~TextureContainer_ID()
{
	D(d);
	for (auto& texture : d->textures)
	{
		delete texture.texture;
	}
}

void TextureContainer_ID::insert(TextureContainer_ID::TextureItemType& item)
{
	D(d);
	ASSERT(d->textures.find(item) == d->textures.end());
	d->textures.insert(item);
}

const TextureContainer_ID::TextureItemType* TextureContainer_ID::find(GMint id)
{
	D(d);
	TextureContainer_ID::TextureItemType _key = { id };
	auto it = d->textures.find(_key);
	if (it == d->textures.end())
		return nullptr;

	return &(*it);
}
