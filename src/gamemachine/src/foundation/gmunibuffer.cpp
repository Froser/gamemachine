#include "stdafx.h"
#include "gmunibuffer.h"

#ifdef max
#undef max
#endif

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

void GMUniBuffer::setContext(const IRenderContext* context)
{
	D(d);
	d->context = context;
	d->proxy->setContext(context);
}

void GMUniBuffer::setBuffer(const GMString& buffer)
{
	D(d);
	d->buffer = buffer;
}

GMint GMUniBuffer::getLength()
{
	D(d);
	GM_ASSERT(std::numeric_limits<GMint>::max() > (GMint)d->buffer.length());
	return (GMint) d->buffer.length();
}

bool GMUniBuffer::CPtoX(GMint cp, bool trail, GMint* x)
{
	D(d);
	return d->proxy->CPtoX(cp, trail, x);
}

bool GMUniBuffer::XtoCP(GMint x, GMint* cp, bool* trail)
{
	D(d);
	return d->proxy->XtoCP(x, cp, trail);
}

void GMUniBuffer::getPriorItemPos(GMint cp, GMint* prior)
{
	D(d);
	d->proxy->getPriorItemPos(cp, prior);
}

void GMUniBuffer::getNextItemPos(GMint cp, GMint* prior)
{
	D(d);
	d->proxy->getNextItemPos(cp, prior);
}