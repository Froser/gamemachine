#ifndef __RESOURCE_CONTAINER_H__
#define __RESOURCE_CONTAINER_H__
#include <gmcommon.h>
#include <set>
#include "../gmdatacore/gmmodel.h"
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

GM_PRIVATE_OBJECT(GMTextureContainer)
{
	std::set<TextureItem<std::string>, Name_Less<TextureItem<std::string> > > textures;
};

class GMTextureContainer : public GMObject
{
	DECLARE_PRIVATE(GMTextureContainer);

public:
	typedef TextureItem<std::string> TextureItemType;

public:
	~GMTextureContainer();

	void insert(TextureItemType& item);
	const TextureItemType* find(const char* name);
};

GM_PRIVATE_OBJECT(GMTextureContainer_ID)
{
	std::set<TextureItem<GMint>, ID_Less<TextureItem<GMint> > > textures;
};

class GMTextureContainer_ID : public GMObject
{
	DECLARE_PRIVATE(GMTextureContainer_ID)

public:
	typedef TextureItem<GMint> TextureItemType;

public:
	~GMTextureContainer_ID();

public:
	void insert(TextureItemType& item);
	const TextureItemType* find(GMint id);
};

GM_PRIVATE_OBJECT(GMModelContainer)
{
	Map<GMuint, GMModel*> items;
	GMint id = 0;
};

struct GMModelContainerItemIndex
{
	GMint id = -1;
};

class GMModelContainer : public GMObject
{
	DECLARE_PRIVATE(GMModelContainer)

public:
	~GMModelContainer();

public:
	GMModelContainerItemIndex insert(AUTORELEASE GMModel* model);
	GMModel* find(GMModelContainerItemIndex index);
};

GM_PRIVATE_OBJECT(GMResourceContainer)
{
	GMTextureContainer textureContainer;
	GMTextureContainer_ID lightmapContainer;
	GMModelContainer modelContainer;
};

class GMResourceContainer : public GMObject
{
	DECLARE_PRIVATE(GMResourceContainer)

public:
	GMTextureContainer& getTextureContainer()
	{
		D(d);
		return d->textureContainer;
	}

	GMTextureContainer_ID& getLightmapContainer()
	{
		D(d);
		return d->lightmapContainer;
	}

	GMModelContainer& getModelContainer()
	{
		D(d);
		return d->modelContainer;
	}
};

END_NS
#endif