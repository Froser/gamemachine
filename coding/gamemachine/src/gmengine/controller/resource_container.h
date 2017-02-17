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
		return left.id < right.id;
	}
};

typedef GMuint ID;

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