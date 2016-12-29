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

Component::Component()
	: m_offset(0)
	, m_verticesCount(0)
	, m_edgeCountPerPolygon(DefaultEdgesCount)
	, m_firstPtr(nullptr)
	, m_countPtr(nullptr)
{
	memset(&m_material, 0, sizeof(m_material));
}

Component::~Component()
{
	if (m_firstPtr)
		delete[] m_firstPtr;
	if (m_countPtr)
		delete[] m_countPtr;
}

void Component::generatePolygonProperties()
{
	GMint offset = m_offset;
	m_firstPtr = new GMint[m_verticesCount];
	m_countPtr = new GMint[m_verticesCount];
	GMint first = m_offset;
	for (GMuint i = 0; i < m_verticesCount / m_edgeCountPerPolygon; i++)
	{
		m_firstPtr[i] = offset;
		m_countPtr[i] = m_edgeCountPerPolygon;
		offset += m_edgeCountPerPolygon;
	}
}

Object::Object()
	: m_painter(nullptr)
	, m_arrayId(0)
	, m_bufferId(0)
{
}

Object::~Object()
{
	m_painter->dispose();

	for (auto iter = m_components.begin(); iter != m_components.cend(); iter++)
	{
		delete *iter;
	}
}

void Object::appendComponent(AUTORELEASE Component* component, GMuint count)
{
	component->m_verticesCount = count;
	component->generatePolygonProperties();
	m_components.push_back(component);
}

void Object::disposeMemory()
{
	m_vertices.clear();
	m_normals.clear();
	m_uvs.clear();
}