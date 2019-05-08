#include "stdafx.h"
#include "gmbuffer.h"
#include "foundation/debug.h"

GMBuffer::GMBuffer()
	: isOwned(true)
	, size(0)
	, data(nullptr)
	, ref(nullptr)
{
}

GMBuffer::GMBuffer(const GMBuffer& buf)
	: GMBuffer()
{
	*this = buf;
}

GMBuffer::GMBuffer(GMBuffer&& buf) GM_NOEXCEPT
	: GMBuffer()
{
	*this = std::move(buf);
}

GMBuffer::GMBuffer(GMbyte* rhs, GMsize_t sz, bool owned)
	: GMBuffer()
{
	ref = new GMAtomic<GMuint32>(1);
	isOwned = owned;
	size = sz;
	if (isOwned)
	{
		data = new GMbyte[size];
		if (rhs)
			memcpy_s(data, size, rhs, size);
	}
	else
	{
		data = rhs;
	}
}

GMBuffer::GMBuffer(GMBuffer& buf, GMsize_t offset)
	: GMBuffer()
{
	isOwned = false;
	data = buf.getData() + offset;
	size = buf.getSize() - offset;
}

GMBuffer::~GMBuffer()
{
	releaseRef();
}

GMBuffer& GMBuffer::operator=(const GMBuffer& rhs)
{
	if (this != &rhs)
	{
		releaseRef();
		size = rhs.size;
		data = rhs.data;
		ref = rhs.ref;
		isOwned = rhs.isOwned;
		addRef();
	}
	return *this;
}

GMBuffer& GMBuffer::operator=(GMBuffer&& rhs) GM_NOEXCEPT
{
	swap(rhs);
	return *this;
}

GMBuffer GMBuffer::createBufferView(const GMBuffer& buf, GMsize_t offset)
{
	return GMBuffer(const_cast<GMBuffer&>(buf), offset);
}

GMBuffer GMBuffer::createBufferView(GMbyte* data, GMsize_t size)
{
	return GMBuffer(data, size, false);
}

const GMbyte* GMBuffer::getData() const
{
	return data;
}

GMbyte* GMBuffer::getData()
{
	return data;
}

GMsize_t GMBuffer::getSize() const
{
	return size;
}

bool GMBuffer::isOwnedBuffer() const
{
	return isOwned;
}

void GMBuffer::resize(GMsize_t sz, GMbyte* d)
{
	if (sz > size)
	{
		*this = GMBuffer(d, sz, true);
	}
	else if (sz < size)
	{
		size = sz;
		if (d)
			memcpy_s(data, size, d, size);
	}
}

void GMBuffer::swap(GMBuffer& rhs)
{
	if (this != &rhs)
	{
		std::swap(size, rhs.size);
		std::swap(data, rhs.data);
		std::swap(ref, rhs.ref);
		std::swap(isOwned, rhs.isOwned);
	}
}

void GMBuffer::convertToStringBuffer()
{
	GMBuffer buf;
	buf.resize(size + 1);
	memcpy_s(buf.data, size, data, size);
	// 在末尾补0
	buf.data[size] = 0;
	*this = buf;
}

void GMBuffer::addRef()
{
	if (ref)
		++(*ref);
}

void GMBuffer::releaseRef()
{
	if (ref)
	{
		--(*ref);
		if (*ref == 0)
		{
			size = 0;
			GM_delete(ref);
			if (isOwnedBuffer())
				GM_delete_array(data);
		}
	}
}
