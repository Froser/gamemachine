#include "stdafx.h"
#include "debug.h"
#include "utilities/assert.h"

Debug* Debug::getDebugger()
{
	static Debug s_dbg;
	return &s_dbg;
}

Debug::Debug()
{

}

void Debug::setInt(GMint key, GMint value)
{
	m_ints[key] = value;
}

GMint Debug::getInt(GMint key)
{
	ASSERT(m_ints.find(key) != m_ints.end());
	return m_ints[key];
}