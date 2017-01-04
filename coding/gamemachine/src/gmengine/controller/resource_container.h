#ifndef __RESOURCE_CONTAINER_H__
#define __RESOURCE_CONTAINER_H__
#include "common.h"
#include <set>
#include "gmdatacore/gmmap/gmmap.h"
BEGIN_NS

class TextureContainer
{
public:
	struct TextureItem
	{
		ID id;
		TextureType type;
		ITexture* texture;
	};

	~TextureContainer();

	void insert(TextureItem& item);
	const TextureItem* find(ID id);

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

private:
	TextureContainer m_textureContainer;
};

END_NS
#endif