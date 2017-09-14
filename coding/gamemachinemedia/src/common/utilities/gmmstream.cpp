#include "stdafx.h"
#include <gamemachine.h>
#include "gmmstream.h"

GMMStream::GMMStream()
{
	D(d);
	d->ptr = 0;
}

GMMStream::~GMMStream()
{
	D(d);
	if (d->data)
		delete d->data;
}

void GMMStream::resize(size_t sz)
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
