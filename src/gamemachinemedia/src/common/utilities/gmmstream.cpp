#include "stdafx.h"
#include "gmmstream.h"

BEGIN_MEDIA_NS

GM_PRIVATE_OBJECT_UNALIGNED(GMMStream)
{
	gm::GMbyte* data = nullptr;
	gm::GMManualResetEvent preventRead;
	std::atomic_uint ptr;
	gm::GMsize_t capacity = 0;
	bool writing = false;
};

GMMStream::GMMStream()
{
	GM_CREATE_DATA();

	D(d);
	d->ptr = 0;
}

GMMStream::~GMMStream()
{
	D(d);
	if (d->data)
		delete d->data;
}

void GMMStream::resize(gm::GMsize_t sz)
{
	D(d);
	d->data = new gm::GMbyte[sz];
	d->capacity = sz;
}

void GMMStream::beginWrite()
{
	D(d);
	d->writing = true;
	d->preventRead.reset();
}

void GMMStream::endWrite()
{
	D(d);
	d->writing = false;
	d->preventRead.set();
}

void GMMStream::rewind()
{
	D(d);
	d->ptr = 0;
}

bool GMMStream::isFull()
{
	D(d);
	return d->capacity == d->ptr;
}

bool GMMStream::isWriting()
{
	D(d);
	return d->writing;
}

void GMMStream::fill(gm::GMbyte byte)
{
	D(d);
	size_t sz = d->capacity - d->ptr;
	if (sz)
		memset(d->data, 0u, sz);
}

GMMStream& GMMStream::operator <<(gm::GMbyte byte)
{
	D(d);
	GM_ASSERT(d->ptr < d->capacity);
	d->data[d->ptr] = byte;
	d->ptr++;
	return *this;
}

bool GMMStream::read(gm::GMbyte* buffer)
{
	D(d);
	d->preventRead.wait();
	if (!isFull())
		return false;

	memcpy_s(buffer, d->capacity, d->data, d->capacity);
	return true;
}

END_MEDIA_NS