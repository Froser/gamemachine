#include "stdafx.h"
#include "tools.h"
#include <linearmath.h>
#include "assert.h"
#include <zlib.h>
#include <fstream>

BEGIN_NS
//GMClock
GM_PRIVATE_OBJECT_UNALIGNED(GMClock)
{
	GMint64 frequency;
	GMint64 timeCycles;
	GMint64 begin, end;
	GMint64 deltaCycles;
	GMfloat timeScale;
	GMfloat fps;
	bool paused;

	//以下用于计算帧率
	GMint32 frameCount;
	GMint64 lastCycle;
};

GMClock::GMClock()
{
	GM_CREATE_DATA(GMClock);

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
	GM_ASSERT(delta > 0);

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
	return cycleToSecond(now - d->begin) * d->timeScale;
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
	return gm::highResolutionTimerFrequency();
}

GMint64 GMClock::highResolutionTimer()
{
	return gm::highResolutionTimer();
}

GMfloat GMClock::cycleToSecond(GMint64 cycle)
{
	D(d);
	return cycle / (GMfloat)d->frequency;
}

//GMStopwatch
GM_PRIVATE_OBJECT_UNALIGNED(GMStopwatch)
{
	GMint64 frequency;
	GMint64 start;
	GMint64 end;
};

GMStopwatch::GMStopwatch()
{
	GM_CREATE_DATA(GMStopwatch);

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
	GMVec3 n = Cross((p1 - p0), (p2 - p0));
	n = FastNormalize(n);
	setNormal(n);
	calculateIntercept(p0);
}

bool GMPlane::intersect3(const GMPlane & p2, const GMPlane & p3, GMVec3 & result)//find point of intersection of 3 planes
{
	GMfloat denominator = Dot(normal, (Cross(p2.normal, p3.normal)));
	//scalar triple product of normals
	if (denominator == 0.0f)									//if zero
		return false;										//no intersection

	GMVec3 temp1, temp2, temp3;
	temp1 = (Cross(GMVec3(p2.normal), GMVec3(p3.normal))) * getIntercept();
	temp2 = (Cross(GMVec3(p3.normal), GMVec3(normal))) * p2.getIntercept();
	temp3 = (Cross(GMVec3(normal), GMVec3(p2.normal))) * p3.getIntercept();

	result = (temp1 + temp2 + temp3) / (-denominator);

	return true;
}

GMfloat GMPlane::getDistance(const GMVec3 & point) const
{
	return Dot(point, normal) + getIntercept();
}

PointPosition GMPlane::classifyPoint(const GMVec3 & point) const
{
	GMfloat distance = getDistance(point);

	if (distance > EPSILON)
		return PointPosition::POINT_IN_FRONT_OF_PLANE;

	if (distance < -EPSILON)
		return PointPosition::POINT_BEHIND_PLANE;

	return PointPosition::POINT_ON_PLANE;
}

GMPlane GMPlane::lerp(const GMPlane & p2, GMfloat factor)
{
	GMPlane result;
	GMVec3 n = normal * (1.0f - factor) + p2.normal * factor;
	result.setNormal(FastNormalize(n));
	result.setIntercept(getIntercept() * (1.0f - factor) + p2.getIntercept() *factor);
	return result;
}

bool GMPlane::operator ==(const GMPlane & rhs) const
{
	if (normal == rhs.normal)
		return true;

	return false;
}

//MemoryStream
GM_PRIVATE_OBJECT_UNALIGNED(GMMemoryStream)
{
	const GMbyte* ptr = nullptr;
	const GMbyte* start = nullptr;
	const GMbyte* end = nullptr;
	GMsize_t size = 0;
};

GMMemoryStream::GMMemoryStream(const GMbyte* buffer, GMsize_t size)
{
	GM_CREATE_DATA(GMMemoryStream);

	D(d);
	d->start = buffer;
	d->size = size;
	d->ptr = buffer;
	d->end = d->start + d->size;
}

GMMemoryStream::~GMMemoryStream()
{

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

void GMMemoryStream::seek(GMsize_t cnt, SeekMode mode)
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
GM_PRIVATE_OBJECT_UNALIGNED(Bitset)
{
	GMint32 numBytes = 0;
	GMbyte* bits = nullptr;
};

Bitset::~Bitset()
{
	D(d);
	GM_delete_array(d->bits);
}

Bitset::Bitset()
{
	GM_CREATE_DATA(Bitset);

	D(d);
	d->numBytes = 0;
	d->bits = nullptr;
}

bool Bitset::init(GMint32 numberOfBits)
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

void Bitset::clearAll()
{
	D(d);
	memset(d->bits, 0, d->numBytes);
}

void Bitset::setAll()
{
	D(d);
	memset(d->bits, 0xFF, d->numBytes);
}

void Bitset::clear(GMint32 bitNumber)
{
	D(d);
	d->bits[bitNumber >> 3] &= ~(1 << (bitNumber & 7));
}

void Bitset::set(GMint32 bitNumber)
{
	D(d);
	d->bits[bitNumber >> 3] |= 1 << (bitNumber & 7);
}

GMbyte Bitset::isSet(GMint32 bitNumber)
{
	D(d);
	return d->bits[bitNumber >> 3] & 1 << (bitNumber & 7);
}

void Bitset::toggle(GMint32 bitNumber)
{
	if (isSet(bitNumber))
		clear(bitNumber);
	else
		set(bitNumber);
}

//Convertion
GMfloat GMConvertion::pointToInch(GMint32 pt)
{
	return pt / 72.f;
}

GMfloat GMConvertion::pointToPixel(GMint32 pt, bool horizontal)
{
	return pointToInch(pt) * (horizontal ? GMScreen::horizontalResolutionDpi() : GMScreen::verticalResolutionDpi());
}

bool GMConvertion::hexToRGB(const GMString& hex, GMfloat rgb[3])
{
	// 将#aabbcc形式的十六进制颜色值转换成0~1之间的浮点形式
	GM_ASSERT(hex.length() == 7);
	struct __InvalidHex : public std::runtime_error {
		__InvalidHex() : std::runtime_error("Invalid hex format.") {}
	};
	auto toDecFromHex = [](char ch) -> GMint32 {
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

GMBuffer GMConvertion::fromBase64(const GMBuffer& base64)
{
	GMuint32 buf = 0;
	GMint32 nbits = 0;
	GMBuffer tmp;
	tmp.resize((base64.getSize() * 3) / 4);

	GMsize_t offset = 0;
	for (GMsize_t i = 0; i < base64.getSize(); ++i) {
		GMint32 ch = base64.getData()[i];
		GMint32 d;

		if (ch >= 'A' && ch <= 'Z')
			d = ch - 'A';
		else if (ch >= 'a' && ch <= 'z')
			d = ch - 'a' + 26;
		else if (ch >= '0' && ch <= '9')
			d = ch - '0' + 52;
		else if (ch == '+')
			d = 62;
		else if (ch == '/')
			d = 63;
		else
			d = -1;

		if (d != -1) {
			buf = (buf << 6) | d;
			nbits += 6;
			if (nbits >= 8) {
				nbits -= 8;
				tmp.getData()[offset++] = buf >> nbits;
				buf &= (1 << nbits) - 1;
			}
		}
	}

	tmp.resize(offset);
	return std::move(tmp);
}

GMBuffer GMConvertion::toBase64(const GMBuffer& buffer)
{
	static const char alphabet[] = "ABCDEFGH" "IJKLMNOP" "QRSTUVWX" "YZabcdef"
		"ghijklmn" "opqrstuv" "wxyz0123" "456789+/";
	static const char padchar = '=';
	int padlen = 0;

	GMBuffer tmp;
	tmp.resize((buffer.getSize() * 4) / 3 + 3);

	GMsize_t i = 0;
	char *out = reinterpret_cast<char*>(tmp.getData());
	while (i < buffer.getSize())
	{
		int chunk = 0;
		chunk |= (int)((unsigned char)(buffer.getData()[i++])) << 16;
		if (i == buffer.getSize())
		{
			padlen = 2;
		}
		else
		{
			chunk |= (int)((unsigned char)(buffer.getData()[i++])) << 8;
			if (i == buffer.getSize()) padlen = 1;
			else chunk |= (int)((unsigned char)(buffer.getData()[i++]));
		}

		int j = (chunk & 0x00fc0000) >> 18;
		int k = (chunk & 0x0003f000) >> 12;
		int l = (chunk & 0x00000fc0) >> 6;
		int m = (chunk & 0x0000003f);
		*out++ = alphabet[j];
		*out++ = alphabet[k];
		if (padlen > 1) *out++ = padchar;
		else *out++ = alphabet[l];
		if (padlen > 0) *out++ = padchar;
		else *out++ = alphabet[m];
	}

	tmp.resize(out - reinterpret_cast<char*>(tmp.getData()));
	return tmp;
}

GMZip::ErrorCode GMZip::inflate(const GMBuffer& buf, REF GMBuffer& out, GMsize_t sizeHint)
{
	GMsize_t sizeIncFactor = 1;
	if (!sizeHint)
		sizeHint = 1;

	out.resize(sizeHint);
	z_stream stream;
	stream.zalloc = (alloc_func)0;
	stream.zfree = (free_func)0;
	stream.opaque = (voidpf)0;
	stream.next_in = (z_const Bytef *) buf.getData();
	stream.avail_in = (uInt) buf.getSize();
	stream.next_out = out.getData();
	stream.avail_out = (uInt) sizeHint;
	GMint32 err = Z_OK;
	if ((err = inflateInit2(&stream, MAX_WBITS + 32)) != Z_OK)
	{
		gm_error(gm_dbg_wrap("inflate error. error code: {0}"), GMString(err));
		return translateError(err);
	}

	while (true)
	{
		err = ::inflate(&stream, Z_NO_FLUSH);
		if (err == Z_STREAM_END)
			break;

		switch (err)
		{
		case Z_NEED_DICT:
			err = Z_DATA_ERROR;
		case Z_DATA_ERROR:
		case Z_MEM_ERROR:
		case Z_STREAM_ERROR:
		case Z_BUF_ERROR:
			inflateEnd(&stream);
			return translateError(err);
		}


		if (err != Z_STREAM_END)
		{
			// 内存不够的情况，重新生成一段数据
			GMsize_t newSize = sizeHint * (++sizeIncFactor);
			out.resize(newSize);
			stream.next_out = out.getData() + sizeHint;
			stream.avail_out = (uInt) sizeHint;
			sizeHint *= sizeIncFactor;
		}
	}

	out.resize(sizeHint - stream.avail_out);
	return translateError(inflateEnd(&stream));
}

GMZip::ErrorCode GMZip::translateError(GMint32 err)
{
	switch (err)
	{
	case Z_OK:
		return Ok;
	case Z_MEM_ERROR:
		return MemoryError;
	case Z_VERSION_ERROR:
		return VersionError;
	case Z_DATA_ERROR:
		return DataError;
	case Z_STREAM_ERROR:
		return StreamError;
	}
	return UnknownError;
}

END_NS