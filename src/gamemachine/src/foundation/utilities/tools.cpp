#include "stdafx.h"
#include "tools.h"
#include <linearmath.h>
#include "assert.h"
#include "foundation/vector.h"

//GMClock
GMClock::GMClock()
{
	D(d);
	d->fps = 0;
	d->timeScale = 1.f;
}

void GMClock::setTimeScale(GMfloat s)
{
	D(d);
	d->timeScale = s;
}

void GMClock::setPaused(bool b)
{
	D(d);
	d->paused = b;
}

void GMClock::begin()
{
	D(d);
	d->timeCycles = 0;
	d->paused = false;
	d->frequency = highResolutionTimerFrequency();
	d->begin = highResolutionTimer();
	d->frameCount = 0;
	d->lastCycle = 0;
	d->deltaCycles = 0;
}

// 每一帧运行一次update
void GMClock::update()
{
	D(d);
	d->end = highResolutionTimer();
	GMint64 delta = d->end - d->begin;

	if (!d->paused)
	{
		d->deltaCycles = delta * d->timeScale;
		d->timeCycles += d->deltaCycles;
	}
	else
	{
		d->deltaCycles = 0;
	}

	++d->frameCount;
	GMint64 deltaFrameCycle = d->end - d->lastCycle;
	if (deltaFrameCycle > d->frequency)
	{
		d->fps = d->frameCount / cycleToSecond(deltaFrameCycle);
		d->frameCount = 0;
		d->lastCycle = d->end;
	}

	d->begin = d->end;
}

GMfloat GMClock::elapsedFromStart()
{
	D(d);
	GMint64 now = highResolutionTimer();
	return cycleToSecond(now - d->begin);
}

GMfloat GMClock::getFps()
{
	D(d);
	return d->fps;
}

GMfloat GMClock::getTime()
{
	D(d);
	return cycleToSecond(d->timeCycles);
}

GMfloat GMClock::evaluateDeltaTime()
{
	D(d);
	return cycleToSecond(d->deltaCycles);
}

// platforms/[os]/timer.cpp
extern "C" GMint64 highResolutionTimerFrequency();
extern "C" GMint64 highResolutionTimer();

GMint64 GMClock::highResolutionTimerFrequency()
{
	return ::highResolutionTimerFrequency();
}

GMint64 GMClock::highResolutionTimer()
{
	return ::highResolutionTimer();
}

GMfloat GMClock::cycleToSecond(GMint64 cycle)
{
	D(d);
	return cycle / (GMfloat)d->frequency;
}

//GMStopwatch
GMStopwatch::GMStopwatch()
{
	D(d);
	d->frequency = GMClock::highResolutionTimerFrequency();
	d->start = 0;
	d->end = 0;
}

void GMStopwatch::start()
{
	D(d);
	d->start = GMClock::highResolutionTimer();
}

void GMStopwatch::stop()
{
	D(d);
	d->end = GMClock::highResolutionTimer();
}

GMfloat GMStopwatch::timeInSecond()
{
	D(d);
	return timeInCycle() / (GMfloat)d->frequency;
}

GMint64 GMStopwatch::timeInCycle()
{
	D(d);
	return d->end - d->start;
}

GMfloat GMStopwatch::nowInSecond()
{
	D(d);
	return nowInCycle() / (GMfloat)d->frequency;
}

GMint64 GMStopwatch::nowInCycle()
{
	D(d);
	auto now = GMClock::highResolutionTimer();
	return now - d->start;
}

//Plane
#define EPSILON 0.01f

void GMPlane::setFromPoints(const GMVec3 & p0, const GMVec3 & p1, const GMVec3 & p2)
{
	normal = Cross((p1 - p0), (p2 - p0));
	normal = FastNormalize(normal);
	calculateIntercept(p0);
}

bool GMPlane::intersect3(const GMPlane & p2, const GMPlane & p3, GMVec3 & result)//find point of intersection of 3 planes
{
	GMfloat denominator = Dot(normal, (Cross(p2.normal, p3.normal)));
	//scalar triple product of normals
	if (denominator == 0.0f)									//if zero
		return false;										//no intersection

	GMVec3 temp1, temp2, temp3;
	temp1 = (Cross(p2.normal, p3.normal))*intercept;
	temp2 = (Cross(p3.normal, normal))*p2.intercept;
	temp3 = (Cross(normal, p2.normal))*p3.intercept;

	result = (temp1 + temp2 + temp3) / (-denominator);

	return true;
}

GMfloat GMPlane::getDistance(const GMVec3 & point) const
{
	return Dot(point, normal) + intercept;
}

PointPosition GMPlane::classifyPoint(const GMVec3 & point) const
{
	GMfloat distance = getDistance(point);

	if (distance > EPSILON)
		return POINT_IN_FRONT_OF_PLANE;

	if (distance < -EPSILON)
		return POINT_BEHIND_PLANE;

	return POINT_ON_PLANE;
}

GMPlane GMPlane::lerp(const GMPlane & p2, GMfloat factor)
{
	GMPlane result;
	result.normal = normal * (1.0f - factor) + p2.normal * factor;
	result.normal = FastNormalize(result.normal);

	result.intercept = intercept*(1.0f - factor) + p2.intercept*factor;

	return result;
}

bool GMPlane::operator ==(const GMPlane & rhs) const
{
	if ((normal == rhs.normal) && (intercept == rhs.intercept))
		return true;

	return false;
}

//MemoryStream
GMMemoryStream::GMMemoryStream(const GMbyte* buffer, GMsize_t size)
{
	D(d);
	d->start = buffer;
	d->size = size;
	d->ptr = buffer;
	d->end = d->start + d->size;
}

GMsize_t GMMemoryStream::read(GMbyte* buf, GMsize_t size)
{
	D(d);
	if (d->ptr >= d->end)
		return 0;

	GMsize_t realSize = d->ptr + size > d->end ? d->end - d->ptr : size;
	if (buf)
		memcpy(buf, d->ptr, realSize);
	d->ptr += realSize;
	return realSize;
}

GMsize_t GMMemoryStream::peek(GMbyte* buf, GMsize_t size)
{
	D(d);
	if (d->ptr >= d->end)
		return 0;

	GMsize_t realSize = d->ptr + size > d->end ? d->end - d->ptr : size;
	memcpy(buf, d->ptr, realSize);
	return realSize;
}

void GMMemoryStream::rewind()
{
	D(d);
	d->ptr = d->start;
}

GMsize_t GMMemoryStream::size()
{
	D(d);
	return d->size;
}

GMsize_t GMMemoryStream::tell()
{
	D(d);
	return d->ptr - d->start;
}

GMbyte GMMemoryStream::get()
{
	GMbyte c;
	read(&c, 1);
	return c;
}

void GMMemoryStream::seek(GMuint cnt, SeekMode mode)
{
	D(d);
	if (mode == GMMemoryStream::FromStart)
		d->ptr = d->start + cnt;
	else if (mode == GMMemoryStream::FromNow)
		d->ptr += cnt;
	else
		GM_ASSERT(false);
}

//Bitset
bool Bitset::init(GMint numberOfBits)
{
	D(d);
	//Delete any memory allocated to bits
	GM_delete_array(d->bits);

	//Calculate size
	d->numBytes = (numberOfBits >> 3) + 1;

	//Create memory
	d->bits = new unsigned char[d->numBytes];
	if (!d->bits)
	{
		gm_error(gm_dbg_wrap("Unable to allocate space for a Bitset of {0} bits"), GMString(numberOfBits));
		return false;
	}

	clearAll();

	return true;
}

//Convertion
GMfloat GMConvertion::pointToInch(GMint pt)
{
	return pt / 72.f;
}

GMfloat GMConvertion::pointToPixel(GMint pt)
{
	return pointToInch(pt) * GMScreen::dpi();
}

bool GMConvertion::hexToRGB(const GMString& hex, GMfloat rgb[3])
{
	// 将#aabbcc形式的十六进制颜色值转换成0~1之间的浮点形式
	GM_ASSERT(hex.length() == 7);
	struct __InvalidHex : public std::runtime_error {
		__InvalidHex() : std::runtime_error("Invalid hex format.") {}
	};
	auto toDecFromHex = [](char ch) -> GMint {
		switch (ch)
		{
		case '0':
			return 0;
		case '1':
			return 1;
		case '2':
			return 2;
		case '3':
			return 3;
		case '4':
			return 4;
		case '5':
			return 5;
		case '6':
			return 6;
		case '7':
			return 7;
		case '8':
			return 8;
		case '9':
			return 9;
		case 'A':
		case 'a':
			return 10;
		case 'B':
		case 'b':
			return 11;
		case 'C':
		case 'c':
			return 12;
		case 'D':
		case 'd':
			return 13;
		case 'E':
		case 'e':
			return 14;
		case 'F':
		case 'f':
			return 15;
		default:
			throw __InvalidHex();
		}
	};

	if (hex[0] != '#')
		return false;

	try
	{
		rgb[0] = ((toDecFromHex(hex[1]) << 4) + toDecFromHex(hex[2])) / 255.f;
		rgb[1] = ((toDecFromHex(hex[3]) << 4) + toDecFromHex(hex[4])) / 255.f;
		rgb[2] = ((toDecFromHex(hex[5]) << 4) + toDecFromHex(hex[6])) / 255.f;
		return true;
	}
	catch (__InvalidHex)
	{
		return false;
	}
}

GMVec4 GMConvertion::hexToRGB(const GMString& hex)
{
	GMfloat rgb[3];
	if (hexToRGB(hex, rgb))
		return GMVec4(rgb[0], rgb[1], rgb[2], 1.f);

	return GMVec4(0, 0, 0, 0);
}

GMString GMConvertion::toUnixString(const GMString& string)
{
	return string.replace(L"\r\n", L"\n");
}

GMString GMConvertion::toWin32String(const GMString& string)
{
	return toUnixString(string).replace(L"\n", L"\r\n");
}
