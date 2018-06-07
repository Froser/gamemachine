#include "stdafx.h"
#include "gmunibuffer.h"

GMUniBuffer::GMUniBuffer()
{
	D(d);
	GMUniProxyFactory::createUniProxy(this, &d->proxy);
}

GMUniBuffer::~GMUniBuffer()
{
	D(d);
	GM_delete(d->proxy);
}