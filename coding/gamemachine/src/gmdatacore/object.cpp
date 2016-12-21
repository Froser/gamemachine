#include "stdafx.h"
#include "object.h"

#ifdef USE_OPENGL
#include "gmgl/objectdrawer.h"
#endif

Component::Component()
	: m_offset(0)
	, m_count(0)
{
}

Component::~Component()
{
}

Object::Object()
	: m_drawer(DEFAULT_DRAWER)
	, m_arrayId(0)
	, m_bufferId(0)
	, m_elementBufferId(0)
{

}

Object::~Object()
{
	for (auto iter = m_components.begin(); iter != m_components.cend(); iter++)
	{
		delete *iter;
	}

	delete[] m_vertices.data;
	delete[] m_indices.data;
}