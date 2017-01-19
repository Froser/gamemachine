#include "stdafx.h"
#include "object.h"
#include "utilities/vmath.h"

ObjectPainter::ObjectPainter(Object* obj)
	: m_object(obj)
{

}

Object* ObjectPainter::getObject()
{
	return m_object;
}

Object::~Object()
{
	if (m_painter)
		m_painter->dispose();

	BEGIN_FOREACH_OBJ(this, childObj)
	{
		if (childObj)
			delete childObj;
	}
	END_FOREACH_OBJ
}

Component::Component()
	: m_offset(0)
	, m_verticesCount(0)
	, m_polygonCount(0)
{
	memset(&m_material, 0, sizeof(m_material));
}

Component::~Component()
{
	for (GMuint i = 0; i < MaxTextureCount; i++)
	{
		TextureInfo texture = getMaterial().textures[i];
		if (texture.autorelease)
			delete texture.texture;
	}
}

ChildObject::ChildObject()
	: m_arrayId(0)
	, m_bufferId(0)
	, m_type(NormalObject)
	, m_mode(Triangle_Fan)
	, m_name("default")
{
}

ChildObject::ChildObject(const std::string& name)
	: m_arrayId(0)
	, m_bufferId(0)
	, m_type(NormalObject)
	, m_mode(Triangle_Fan)
{
	m_name = name;
}

ChildObject::~ChildObject()
{
	for (auto iter = m_components.begin(); iter != m_components.cend(); iter++)
	{
		delete *iter;
	}
}

void ChildObject::appendComponent(AUTORELEASE Component* component, GMuint verticesCount)
{
	component->m_verticesCount = verticesCount;
	m_components.push_back(component);
}

void ChildObject::disposeMemory()
{
	m_vertices.clear();
	m_normals.clear();
	m_uvs.clear();
}