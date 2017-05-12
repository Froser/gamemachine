#include "stdafx.h"
#include "memorystream.h"
#include "assert.h"

MemoryStream::MemoryStream(const GMbyte* data, GMuint size)
	: m_start(data)
	, m_size(size)
	, m_ptr(data)
{
	m_end = m_start + m_size;
}

GMuint MemoryStream::read(GMbyte* buf, GMuint size)
{
	if (m_ptr >= m_end)
		return 0;

	GMuint realSize = m_ptr + size > m_end ? m_end - m_ptr : size;
	memcpy(buf, m_ptr, realSize);
	m_ptr += realSize;
	return realSize;
}

GMuint MemoryStream::peek(GMbyte* buf, GMuint size)
{
	if (m_ptr >= m_end)
		return 0;

	GMuint realSize = m_ptr + size > m_end ? m_end - m_ptr : size;
	memcpy(buf, m_ptr, realSize);
	return realSize;
}

void MemoryStream::rewind()
{
	m_ptr = m_start;
}

GMuint MemoryStream::size()
{
	return m_size;
}

GMuint MemoryStream::tell()
{
	return m_ptr - m_start;
}

GMbyte MemoryStream::get()
{
	GMbyte c;
	read(&c, 1);
	return c;
}

void MemoryStream::seek(GMuint cnt, SeekMode mode)
{
	if (mode == MemoryStream::FromStart)
		m_ptr = m_start + cnt;
	else if (mode == MemoryStream::FromNow)
		m_ptr += cnt;
	else
		ASSERT(false);
}