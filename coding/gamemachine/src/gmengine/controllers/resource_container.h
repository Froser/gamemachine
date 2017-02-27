#ifndef __RESOURCE_CONTAINER_H__
#define __RESOURCE_CONTAINER_H__
#include "common.h"
#include <set>
#include "gmdatacore/object.h"
BEGIN_NS

template <typename T>
class ID_Less
{
public:
	bool operator ()(const T& left, const T& right)
	{
		return (left.id < right.id);
	}
};

template <typename T>
class Name_Less
{
public:
	bool operator ()(const T& left, const T& right)
	{
		return left.name < right.name;
	}
};

class TextureContainer
{
public:
	struct TextureItem
	{
		std::string name;
		ITexture* texture;
	};

	~TextureContainer();

	void insert(TextureItem& item);
	const TextureItem* find(const char* name);

private:
	std::set<TextureItem, Name_Less<TextureItem> > m_textures;
};

class TextureContainer_ID
{
public:
	struct TextureItem
	{
		GMint id;
		ITexture* texture;
	};

	~TextureContainer_ID();

	void insert(TextureItem& item);
	const TextureItem* find(GMint id);

private:
	std::set<TextureItem, ID_Less<TextureItem> > m_textures;
};

class ResourceContainer
{
public:
	TextureContainer& getTextureContainer()
	{
		return m_textureContainer;
	}

	TextureContainer_ID& getLightmapContainer()
	{
		return m_lightmapContainer;
	}

private:
	TextureContainer m_textureContainer;
	TextureContainer_ID m_lightmapContainer;
};

END_NS
#endif