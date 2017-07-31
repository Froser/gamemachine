#ifndef __RESOURCE_CONTAINER_H__
#define __RESOURCE_CONTAINER_H__
#include "common.h"
#include <set>
#include "gmdatacore/gmmodel.h"
BEGIN_NS

template <typename T>
struct ID_Less
{
	bool operator ()(const T& left, const T& right) const
	{
		return (left.id < right.id);
	}
};

template <typename T>
struct Name_Less
{
	bool operator ()(const T& left, const T& right) const
	{
		return left.id < right.id;
	}
};

template <typename KeyType>
struct TextureItem
{
	KeyType id;
	ITexture* texture;
};

GM_PRIVATE_OBJECT(TextureContainer)
{
	std::set<TextureItem<std::string>, Name_Less<TextureItem<std::string> > > textures;
};

class TextureContainer : public GMObject
{
	DECLARE_PRIVATE(TextureContainer);

public:
	typedef TextureItem<std::string> TextureItemType;

public:
	~TextureContainer();

	void insert(TextureItemType& item);
	const TextureItemType* find(const char* name);
};

GM_PRIVATE_OBJECT(TextureContainer_ID)
{
	std::set<TextureItem<GMint>, ID_Less<TextureItem<GMint> > > textures;
};

class TextureContainer_ID : public GMObject
{
	DECLARE_PRIVATE(TextureContainer_ID)

public:
	typedef TextureItem<GMint> TextureItemType;

public:
	~TextureContainer_ID();

public:
	void insert(TextureItemType& item);
	const TextureItemType* find(GMint id);
};

GM_PRIVATE_OBJECT(ResourceContainer)
{
	TextureContainer textureContainer;
	TextureContainer_ID lightmapContainer;
};

class ResourceContainer : public GMObject
{
	DECLARE_PRIVATE(ResourceContainer)

public:
	TextureContainer& getTextureContainer()
	{
		D(d);
		return d->textureContainer;
	}

	TextureContainer_ID& getLightmapContainer()
	{
		D(d);
		return d->lightmapContainer;
	}
};

END_NS
#endif