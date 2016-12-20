#include "stdafx.h"
#include "object.h"

#ifdef USE_OPENGL
#include "gmgl/objectdrawer.h"
#endif

Object::Object()
	: m_drawer(DEFAULT_DRAWER)
	, m_arrayId(0)
	, m_elementBufferId(0)
	, m_bufferId(0)
{
}

Object::~Object()
{
	delete[] m_vertices.data;
	delete[] m_indices.data;
}
