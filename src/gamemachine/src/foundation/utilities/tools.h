#ifndef __UTILITIES_H__
#define __UTILITIES_H__
#include <gmcommon.h>
#include <linearmath.h>
#include <time.h>
#include <random>
#include <atomic>
BEGIN_NS

// 此类包含了各种实用工具
// GMScopePtr:
template <typename T, typename DeleteFunc = std::default_delete<T>>
using GMScopedPtr = GMOwnedPtr<T, DeleteFunc>;

//GMClock
GM_PRIVATE_OBJECT(GMClock)
{
	GMint64 frequency;
	GMint64 timeCycles;
	GMint64 begin, end;
	GMint64 deltaCycles;
	GMfloat timeScale;
	GMfloat fps;
	bool paused;

	//以下用于计算帧率
	GMint frameCount;
	GMint64 lastCycle;
};

class GMClock : public GMObject
{
	GM_DECLARE_PRIVATE(GMClock)

public:
	GMClock();
	~GMClock() {}

public:
	void setTimeScale(GMfloat);
	void setPaused(bool);
	void begin();
	void update();
	GMfloat elapsedFromStart();
	GMfloat getFps();
	GMfloat getTime();
	GMfloat evaluateDeltaTime();

public:
	static GMint64 highResolutionTimerFrequency();
	static GMint64 highResolutionTimer();

private:
	GMfloat cycleToSecond(GMint64 cycle);
};

//GMStopwatch
GM_PRIVATE_OBJECT(GMStopwatch)
{
	GMint64 frequency;
	GMint64 start;
	GMint64 end;
};

class GMStopwatch : public GMObject
{
	GM_DECLARE_PRIVATE(GMStopwatch);

public:
	GMStopwatch();

public:
	void start();
	void stop();
	GMfloat timeInSecond();
	GMint64 timeInCycle();
	GMfloat nowInSecond();
	GMint64 nowInCycle();
};

//Plane
enum PointPosition
{
	POINT_ON_PLANE = 0,
	POINT_IN_FRONT_OF_PLANE,
	POINT_BEHIND_PLANE,
};

GM_ALIGNED_STRUCT(GMPlane)
{
	GMPlane() : normal(GMVec3(0.0f, 0.0f, 0.0f)), intercept(0.0f)
	{
	}

	GMPlane(const GMVec3& newNormal, GMfloat newIntercept) : normal(newNormal), intercept(newIntercept)
	{
	}

	GMPlane(const GMPlane & rhs)
	{
		normal = rhs.normal;
		intercept = rhs.intercept;
	}

	GMPlane& operator=(const GMPlane& plane)
	{
		normal = plane.normal;
		intercept = plane.intercept;
		return *this;
	}

	void setNormal(const GMVec3 & rhs) { normal = rhs; }
	void setIntercept(GMfloat newIntercept) { intercept = newIntercept; }
	void setFromPoints(const GMVec3 & p0, const GMVec3 & p1, const GMVec3 & p2);

	void calculateIntercept(const GMVec3 & pointOnPlane) { intercept = - Dot(normal, pointOnPlane); }

	GMVec3 getNormal() { return normal; }
	GMfloat getIntercept() { return intercept; }

	//find point of intersection of 3 planes
	bool intersect3(const GMPlane & p2, const GMPlane & p3, GMVec3 & result);

	GMfloat getDistance(const GMVec3 & point) const;
	PointPosition classifyPoint(const GMVec3 & point) const;

	GMPlane lerp(const GMPlane & p2, GMfloat factor);

	//operators
	bool operator==(const GMPlane & rhs) const;
	bool operator!=(const GMPlane & rhs) const
	{
		return!((*this) == rhs);
	}

	//unary operators
	GMPlane operator-(void) const { return GMPlane(-normal, -intercept); }
	GMPlane operator+(void) const { return (*this); }

	//member variables
	GMVec3 normal;	//X.N+intercept=0
	GMfloat intercept;
};

//GMFrustumPlanes
GM_ALIGNED_STRUCT(GMFrustumPlanes)
{
	GMPlane nearPlane;
	GMPlane farPlane;
	GMPlane topPlane;
	GMPlane bottomPlane;
	GMPlane leftPlane;
	GMPlane rightPlane;
};

//GMMemoryStream
GM_PRIVATE_OBJECT(GMMemoryStream)
{
	const GMbyte* ptr;
	const GMbyte* start;
	const GMbyte* end;
	GMsize_t size;
};

class GMMemoryStream : public GMObject
{
	GM_DECLARE_PRIVATE(GMMemoryStream)

public:
	enum SeekMode
	{
		FromStart,
		FromNow,
	};

public:
	GMMemoryStream(const GMbyte* buffer, GMsize_t size);

public:
	GMsize_t read(GMbyte* buf, GMsize_t size);
	GMsize_t peek(GMbyte* buf, GMsize_t size);
	void seek(GMuint cnt, SeekMode = FromStart);
	void rewind();
	GMsize_t size();
	GMsize_t tell();
	GMbyte get();
};

//Bitset
GM_PRIVATE_OBJECT(Bitset)
{
	GMint numBytes;
	GMbyte* bits;
};

class Bitset : public GMObject
{
	GM_DECLARE_PRIVATE(Bitset)

public:
	Bitset()
	{
		D(d);
		d->numBytes = 0;
		d->bits = nullptr;
	}
	~Bitset()
	{
		D(d);
		GM_delete_array(d->bits);
	}

	bool init(GMint numberOfBits);

	inline void clearAll()
	{
		D(d);
		memset(d->bits, 0, d->numBytes);
	}

	inline void setAll()
	{
		D(d);
		memset(d->bits, 0xFF, d->numBytes);
	}

	inline void clear(GMint bitNumber)
	{
		D(d);
		d->bits[bitNumber >> 3] &= ~(1 << (bitNumber & 7));
	}

	inline void set(GMint bitNumber)
	{
		D(d);
		d->bits[bitNumber >> 3] |= 1 << (bitNumber & 7);
	}

	inline GMbyte isSet(GMint bitNumber)
	{
		D(d);
		return d->bits[bitNumber >> 3] & 1 << (bitNumber & 7);
	}

	inline void toggle(GMint bitNumber)
	{
		if (isSet(bitNumber))
			clear(bitNumber);
		else
			set(bitNumber);
	}

};

//GMPath: platforms/[os]/path.cpp
struct GMPath
{
	enum SpecialFolder
	{
		Fonts,
	};

	static GMString directoryName(const GMString& fileName);
	static GMString filename(const GMString& fullPath);
	static GMString fullname(const GMString& dirName, const GMString& fullPath);
	static GMString getCurrentPath();
	static Vector<GMString> getAllFiles(const GMString& directory);
	static bool directoryExists(const GMString& dir);
	static void createDirectory(const GMString& dir);
	static GMString getSpecialFolderPath(SpecialFolder);
};

//GMPath: platforms/[os]/screen.cpp
struct GMScreen
{
	static GMfloat dpi();
};

//GMEvent: platforms/[os]/event.cpp
#if GM_WINDOWS
typedef HANDLE GMEventHandle;
#else
typedef void* GMEventHandle; //TODO
#endif

GM_PRIVATE_OBJECT(GMEvent)
{
	GMEventHandle handle;
};

// 表示一个Wait之后能够自动Set的事件
class GMEvent : public GMObject
{
	GM_DECLARE_PRIVATE(GMEvent)

protected:
	GMEvent(bool manualReset, bool initialState);

public:
	~GMEvent();

public:
	void wait(GMuint milliseconds = 0);
	void set();
	void reset();
};

class GMAutoResetEvent : public GMEvent
{
public:
	GMAutoResetEvent(bool initialState = false);
};

class GMManualResetEvent : public GMEvent
{
public:
	GMManualResetEvent(bool initialState = false);
};

// Atomic
template <typename T>
using GMAtomic = std::atomic<T>;

struct GMConvertion
{
	static GMfloat pointToInch(GMint pt);
	static GMfloat pointToPixel(GMint pt);
	static bool hexToRGB(const GMString& hex, GMfloat rgb[3]);
	static GMVec4 hexToRGB(const GMString& hex);
	static GMString toUnixString(const GMString& string);
	static GMString toWin32String(const GMString& string);
	static GMString toCurrentEnvironmentString(const GMString& string);
	static GMBuffer fromBase64(const GMBuffer& base64);
};

enum class GMClipboardMIME
{
	Text,
	Bitmap,
	Riff,
	Wave,
	UnicodeText,
};

struct GMClipboard
{
	static void setData(GMClipboardMIME mime, const GMBuffer& buffer);
	static GMBuffer getData(GMClipboardMIME mime);
};

template <typename Engine>
class GMRandom
{
public:
	template<typename T>
	static inline T random_real(T min, T max)
	{
		std::uniform_real_distribution<T> dist(min, max);
		auto &mt = getEngine();
		return dist(mt);
	}

	template<typename T>
	static inline T random_int(T min, T max)
	{
		std::uniform_int_distribution<T> dist(min, max);
		auto &mt = getEngine();
		return dist(mt);
	}

private:
	static Engine& getEngine();
};

template <typename Engine>
Engine& GMRandom<Engine>::getEngine()
{
	static std::random_device seed_gen;
	static Engine engine(seed_gen());
	return engine;
}

using GMRandomMt19937 = GMRandom<std::mt19937>;

class GMZip
{
public:
	enum ErrorCode
	{
		Ok,
		MemoryError,
		VersionError,
		DataError,
		StreamError,
		UnknownError,
	};

	static ErrorCode inflateMemory(const GMBuffer& buf, REF GMBuffer& out, REF GMsize_t& outSize, GMsize_t sizeHint);

private:
	static ErrorCode translateError(GMint);
};

END_NS
#endif
