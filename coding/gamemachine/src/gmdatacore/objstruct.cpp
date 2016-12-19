#include "stdafx.h"
#include "objstruct.h"

void GLObjectCallback::draw(GMuint id)
{
	glCallList(id);
}

void GLObjectCallback::dispose(GMuint id, const TextureMap& textureMap)
{
	for (auto iter = textureMap.cbegin(); iter != textureMap.cend(); iter++)
	{
		const TextureInfo& info = (*iter).second;
		if (info.id == 0)
			continue;

		GMuint id[] = { info.id };
		glDeleteTextures(1, id);
		delete info.texture;
	}

	glDeleteLists(id, 1);
}

Object::Object()
	: m_pCallback(new GLObjectCallback())
{

}

Object::Object(GMuint id, const TextureMap& textureMap)
	: m_objectId(id)
	, m_textureMap(textureMap)
	, m_pCallback(new GLObjectCallback())
{

}

Object& Object::operator =(Object& obj)
{
	this->m_objectId = obj.m_objectId;
	this->m_pCallback = obj.m_pCallback;
	this->m_textureMap = obj.m_textureMap;
	return *this;
}

Object::~Object()
{
	m_pCallback->dispose(m_objectId, m_textureMap);
	delete m_pCallback;
}

void Object::draw()
{
	m_pCallback->draw(m_objectId);
}