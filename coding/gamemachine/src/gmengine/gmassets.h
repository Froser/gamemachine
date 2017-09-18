#ifndef __GMASSETS_H__
#define __GMASSETS_H__
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

GM_PRIVATE_OBJECT(GMTextureAssets)
{
	std::set<TextureItem<std::string>, Name_Less<TextureItem<std::string> > > textures;
};

class GMTextureAssets : public GMObject
{
	DECLARE_PRIVATE(GMTextureAssets);

public:
	typedef TextureItem<std::string> TextureItemType;

public:
	~GMTextureAssets();

	void insert(TextureItemType& item);
	const TextureItemType* find(const char* name);
};

GM_PRIVATE_OBJECT(GMTextureAssets_ID)
{
	std::set<TextureItem<GMint>, ID_Less<TextureItem<GMint> > > textures;
};

class GMTextureAssets_ID : public GMObject
{
	DECLARE_PRIVATE(GMTextureAssets_ID)

public:
	typedef TextureItem<GMint> TextureItemType;

public:
	~GMTextureAssets_ID();

public:
	void insert(TextureItemType& item);
	const TextureItemType* find(GMint id);
};

GM_PRIVATE_OBJECT(GMModelAssets)
{
	Map<GMuint, GMModel*> items;
	GMint id = 0;
};

struct GMModelContainerItemIndex
{
	GMint id = -1;
};

class GMModelAssets : public GMObject
{
	DECLARE_PRIVATE(GMModelAssets)

public:
	~GMModelAssets();

public:
	GMModelContainerItemIndex insert(AUTORELEASE GMModel* model);
	GMModel* find(GMModelContainerItemIndex index);
};

GM_PRIVATE_OBJECT(GMAssets)
{
	GMTextureAssets textureContainer;
	GMTextureAssets_ID lightmapContainer;
	GMModelAssets modelContainer;
};

class GMAssets : public GMObject
{
	DECLARE_PRIVATE(GMAssets)

public:
	GMTextureAssets& getTextureContainer()
	{
		D(d);
		return d->textureContainer;
	}

	GMTextureAssets_ID& getLightmapContainer()
	{
		D(d);
		return d->lightmapContainer;
	}

	GMModelAssets& getModelContainer()
	{
		D(d);
		return d->modelContainer;
	}
};

END_NS
#endif