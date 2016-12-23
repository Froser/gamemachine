#include "stdafx.h"
#include "object.h"

#ifdef USE_OPENGL
#include "gmgl/gmglobjectdrawer.h"
#endif

Component::Component(GMuint edgeCountPerPolygon)
	: m_offset(0)
	, m_count(0)
	, m_edgeCountPerPolygon(edgeCountPerPolygon)
	, m_firstPtr(nullptr)
	, m_countPtr(nullptr)
{
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
	m_firstPtr = new GMint[m_count];
	m_countPtr = new GMint[m_count];
	GMint first = m_offset;
	for (GMint i = 0; i < m_count / m_edgeCountPerPolygon; i++)
	{
		m_firstPtr[i] = offset;
		m_countPtr[i] = m_edgeCountPerPolygon;
		offset += m_edgeCountPerPolygon;
	}
}

Object::Object()
	: m_drawer(DEFAULT_DRAWER)
	, m_arrayId(0)
	, m_bufferId(0)
{
	memset(&m_vertices, 0, sizeof(m_vertices));
	memset(&m_vertices, 0, sizeof(m_normals));
}

Object::~Object()
{
	m_drawer->dispose(this);

	for (auto iter = m_components.begin(); iter != m_components.cend(); iter++)
	{
		delete *iter;
	}

	if (m_vertices.data)
		delete[] m_vertices.data;
}

void Object::appendComponent(Component* component, GMuint count)
{
	component->m_count = count;
	component->generatePolygonProperties();
	m_components.push_back(component);
}