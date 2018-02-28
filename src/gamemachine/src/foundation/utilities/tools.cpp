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
	GMLargeInteger delta = d->end - d->begin;

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
	GMLargeInteger deltaFrameCycle = d->end - d->lastCycle;
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
	GMLargeInteger now = highResolutionTimer();
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
extern "C" GMLargeInteger highResolutionTimerFrequency();
extern "C" GMLargeInteger highResolutionTimer();

GMLargeInteger GMClock::highResolutionTimerFrequency()
{
	return ::highResolutionTimerFrequency();
}

GMLargeInteger GMClock::highResolutionTimer()
{
	return ::highResolutionTimer();
}

GMfloat GMClock::cycleToSecond(GMLargeInteger cycle)
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

GMLargeInteger GMStopwatch::timeInCycle()
{
	D(d);
	return d->end - d->start;
}

GMfloat GMStopwatch::nowInSecond()
{
	D(d);
	return nowInCycle() / (GMfloat)d->frequency;
}

GMLargeInteger GMStopwatch::nowInCycle()
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
GMMemoryStream::GMMemoryStream(const GMbyte* buffer, GMuint size)
{
	D(d);
	d->start = buffer;
	d->size = size;
	d->ptr = buffer;
	d->end = d->start + d->size;
}

GMuint GMMemoryStream::read(GMbyte* buf, GMuint size)
{
	D(d);
	if (d->ptr >= d->end)
		return 0;

	GMuint realSize = d->ptr + size > d->end ? d->end - d->ptr : size;
	memcpy(buf, d->ptr, realSize);
	d->ptr += realSize;
	return realSize;
}

GMuint GMMemoryStream::peek(GMbyte* buf, GMuint size)
{
	D(d);
	if (d->ptr >= d->end)
		return 0;

	GMuint realSize = d->ptr + size > d->end ? d->end - d->ptr : size;
	memcpy(buf, d->ptr, realSize);
	return realSize;
}

void GMMemoryStream::rewind()
{
	D(d);
	d->ptr = d->start;
}

GMuint GMMemoryStream::size()
{
	D(d);
	return d->size;
}

GMuint GMMemoryStream::tell()
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
		gm_error(L"Unable to allocate space for a Bitset of %d bits", numberOfBits);
		return false;
	}

	clearAll();

	return true;
}