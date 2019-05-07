#include "stdafx.h"
#include "gmbuffer.h"

GMBuffer::~GMBuffer()
{
	if (needRelease)
	{
		GM_delete_array(buffer);
	}
}

GMBuffer::GMBuffer(const GMBuffer& rhs)
{
	*this = rhs;
}

GMBuffer::GMBuffer(GMBuffer&& rhs) GM_NOEXCEPT
{
	swap(rhs);
}

GMBuffer& GMBuffer::operator =(GMBuffer&& rhs) GM_NOEXCEPT
{
	swap(rhs);
	return *this;
}

GMBuffer& GMBuffer::operator =(const GMBuffer& rhs)
{
	this->needRelease = rhs.needRelease;
	if (rhs.needRelease)
	{
		this->size = rhs.size;
		buffer = new GMbyte[this->size];
		memcpy_s(buffer, size, rhs.buffer, this->size);
	}
	else
	{
		this->size = rhs.size;
		this->buffer = rhs.buffer;
	}
	return *this;
}

void GMBuffer::convertToStringBuffer()
{
	GMbyte* newBuffer = new GMbyte[size + 1];
	memcpy_s(newBuffer, size, buffer, size);
	newBuffer[size] = 0;
	size++;
	if (needRelease && buffer)
		GM_delete_array(buffer);
	needRelease = true;
	buffer = newBuffer;
}

void GMBuffer::convertToStringBufferW()
{
	GMwchar* newBuffer = new GMwchar[size + 1];
	memcpy_s(newBuffer, size, buffer, size);
	newBuffer[size] = 0;
	size += sizeof(GMwchar);
	if (needRelease && buffer)
		GM_delete_array(buffer);
	needRelease = true;
	buffer = reinterpret_cast<GMbyte*>(newBuffer);
}

void GMBuffer::swap(GMBuffer& rhs)
{
	GM_SWAP(buffer, rhs.buffer);
	GM_SWAP(size, rhs.size);
	GM_SWAP(needRelease, rhs.needRelease);
}

GMBufferView::GMBufferView(const GMBuffer& rhs, GMsize_t offset)
{
	buffer = rhs.buffer + offset;
	size = rhs.size - offset;
	needRelease = false;
}

GMBufferView::~GMBufferView()
{
	GM_ASSERT(!needRelease);
}