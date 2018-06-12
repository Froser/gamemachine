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

void GMUniBuffer::setChar(GMsize_t pos, GMwchar ch)
{
	D(d);
	d->buffer[pos] = ch;
	d->proxy->setDirty();
}

bool GMUniBuffer::insertChar(GMsize_t pos, GMwchar ch)
{
	D(d);
	if (pos < 0)
		return false;

	if (pos == d->buffer.length())
	{
		d->buffer.append(ch);
	}
	else
	{
		GMString newStr = d->buffer.substr(0, pos);
		newStr.append(ch);
		newStr.append(d->buffer.substr(pos, d->buffer.length() - pos));
		d->buffer = std::move(newStr);
	}
	d->proxy->setDirty();
	return true;
}

bool GMUniBuffer::removeChar(GMsize_t pos)
{
	D(d);
	if (pos < 0 || pos >= d->buffer.length())
		return false;
	
	GMString newStr = d->buffer.substr(0, pos);
	if (pos + 1 < d->buffer.length())
		newStr += d->buffer.substr(pos + 1, d->buffer.length() - pos - 1);
	d->buffer = std::move(newStr);
	d->proxy->setDirty();
	return true;
}

bool GMUniBuffer::removeChars(GMsize_t startPos, GMsize_t endPos)
{
	D(d);
	if (startPos > endPos)
	{
		GM_SWAP(startPos, endPos);
	}

	if (startPos < 0 || startPos > d->buffer.length())
		return false;

	if (endPos < 0 || endPos > d->buffer.length())
		return false;

	if (startPos == endPos)
		return false;

	GMString newStrA = d->buffer.substr(0, startPos);
	if (endPos < d->buffer.length())
		newStrA.append(d->buffer.substr(endPos, d->buffer.length() - endPos));
	d->buffer = std::move(newStrA);
	d->proxy->setDirty();
	return true;
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