#ifndef __GMSTRING_H__
#define __GMSTRING_H__
#include <defines.h>
#include <gmobject.h>
#include "assert.h"
#include "gmchar.h"

BEGIN_NS

class GMStringData
{
public:
	GMChar* data;
	size_t size;

public:
	static GMStringData* sharedNull();
	static GMStringData* allocate(size_t size);

public:
	GMChar* data()
	{
		return data;
	}
};

class GMString
{
	typedef GMStringData Data;

public:
	GMString(GMChar* unicode, size_t size);

private:
	Data* d;
};

END_NS
#endif
