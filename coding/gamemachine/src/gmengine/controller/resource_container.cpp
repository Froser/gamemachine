#include "stdafx.h"
#include "resource_container.h"
#include "utilities/assert.h"

TextureContainer::~TextureContainer()
{
	for (auto iter = m_textures.begin(); iter != m_textures.end(); iter++)
	{
		delete (*iter).texture;
	}
}

void TextureContainer::insert(TextureItem& item)
{
	ASSERT(m_textures.find(item) == m_textures.end());
	m_textures.insert(item);
}

const TextureContainer::TextureItem* TextureContainer::find(ID id)
{
	TextureItem _key = { id };
	auto it = m_textures.find(_key);
	if (it == m_textures.end())
		return nullptr;

	return &(*it);
}