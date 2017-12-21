#include "stdafx.h"
#include "gmstring.h"
#include <memory>

namespace
{
	std::allocator<GMChar> allocator;
}

GMStringData* GMStringData::sharedNull()
{
	static GMStringData d = { 0, 0 };
	return &d;
}

GMStringData* GMStringData::allocate(size_t size)
{
}

GMString::GMString(GMChar* unicode, size_t size)
{
	if (!unicode)
	{
		d = Data::sharedNull();
	}
	else
	{
		if (size < 0)
		{
			size = 0;
			while (!unicode[size].isNull())
				++size;
		}
		if (!size)
		{
			d = Data::allocate(0);
		}
		else
		{
			d = Data::allocate(size + 1);
			d->size = size;
			memcpy_s(d->data(), size * sizeof(GMChar), unicode, size * sizeof(GMChar));
			d->data()[size] = '\0';
		}
	}
}