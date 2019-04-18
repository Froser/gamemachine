#ifndef __UTILITIES_H__
#define __UTILITIES_H__
#include <gmcommon.h>
#include <linearmath.h>
#include <time.h>
#include <random>
BEGIN_NS

// 此类包含了各种实用工具
// GMScopePtr:
template <typename T, typename DeleteFunc = std::default_delete<T>>
using GMScopedPtr = GMOwnedPtr<T, DeleteFunc>;

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

class GMClock
{
	GM_DECLARE_PRIVATE_NGO(GMClock)

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
GM_PRIVATE_OBJECT_UNALIGNED(GMStopwatch)
{
	GMint64 frequency;
	GMint64 start;
	GMint64 end;
};

class GMStopwatch
{
	GM_DECLARE_PRIVATE_NGO(GMStopwatch);

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
	GMPlane() : normal(GMVec4(0.0f, 0.0f, 0.0f, 0.0f))
	{
	}

	GMPlane(const GMVec4& plane) : normal(plane)
	{

	}

	GMPlane(const GMPlane & rhs)
	{
		normal = rhs.normal;
	}

	GMPlane& operator=(const GMPlane& plane)
	{
		normal = plane.normal;
		return *this;
	}

	void setNormal(const GMVec3 & rhs)
	{
		normal = GMVec4(rhs, getIntercept());
	}
	
	inline void setIntercept(GMfloat newIntercept)
	{
		normal.setW(newIntercept);
	}

	void setFromPoints(const GMVec3 & p0, const GMVec3 & p1, const GMVec3 & p2);

	void calculateIntercept(const GMVec3 & pointOnPlane)
	{
		setIntercept(-Dot(GMVec3(normal), pointOnPlane));
	}

	inline GMVec3 getNormal() const { return normal; }
	inline GMfloat getIntercept() const { return normal.getW(); }
	inline GMVec4 getPlane() const { return normal; }

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
	GMPlane operator-(void) const { return GMPlane(-normal); }
	GMPlane operator+(void) const { return (*this); }

private:
	//member variables
	GMVec4 normal;	//X.N+intercept=0
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
GM_PRIVATE_OBJECT_UNALIGNED(GMMemoryStream)
{
	const GMbyte* ptr;
	const GMbyte* start;
	const GMbyte* end;
	GMsize_t size;
};

class GM_EXPORT GMMemoryStream
{
	GM_DECLARE_PRIVATE_NGO(GMMemoryStream)

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
	void seek(GMsize_t cnt, SeekMode = FromStart);
	void rewind();
	GMsize_t size();
	GMsize_t tell();
	GMbyte get();
};

//Bitset
GM_PRIVATE_OBJECT(Bitset)
{
	GMint32 numBytes;
	GMbyte* bits;
};

class GM_EXPORT Bitset : public GMObject
{
	GM_DECLARE_PRIVATE(Bitset)

public:
	Bitset();
	~Bitset();

	bool init(GMint32 numberOfBits);
	void clearAll();
	void setAll();
	void clear(GMint32 bitNumber);
	void set(GMint32 bitNumber);
	GMbyte isSet(GMint32 bitNumber);
	void toggle(GMint32 bitNumber);
};

//GMPath: platforms/[os]/path.cpp
struct GM_EXPORT GMPath
{
	enum SpecialFolder
	{
		Fonts,
	};

	static GMString directoryName(const GMString& fileName);
	static GMString filename(const GMString& fullPath);
	static GMString fullname(const GMString& dirName, const GMString& fullPath);
	static GMString getCurrentPath();
	static bool fileExists(const GMString& dir);
	static void createDirectory(const GMString& dir);
	static GMString getSpecialFolderPath(SpecialFolder);
};

//GMPath: platforms/[os]/screen.cpp
struct GM_EXPORT GMScreen
{
	static GMint32 horizontalResolutionDpi();
	static GMint32 verticalResolutionDpi();
};

//GMEvent: platforms/[os]/event.cpp
#if GM_WINDOWS
typedef HANDLE GMEventHandle;
#else
typedef void* GMEventHandle;
#endif

GM_PRIVATE_OBJECT(GMEvent)
{
	GMEventHandle handle;
};

// 表示一个Wait之后能够自动Set的事件
class GM_EXPORT GMEvent
{
	GM_DECLARE_PRIVATE_NGO(GMEvent)

protected:
	GMEvent(bool manualReset, bool initialState);

public:
	~GMEvent();

public:
	void wait(GMuint32 milliseconds = 0);
	void set();
	void reset();
};

class GM_EXPORT GMAutoResetEvent : public GMEvent
{
public:
	GMAutoResetEvent(bool initialState = false);
};

class GM_EXPORT GMManualResetEvent : public GMEvent
{
public:
	GMManualResetEvent(bool initialState = false);
};

struct GM_EXPORT GMConvertion
{
	static GMfloat pointToInch(GMint32 pt);
	static GMfloat pointToPixel(GMint32 pt, bool horizontal = true);
	static bool hexToRGB(const GMString& hex, GMfloat rgb[3]);
	static GMVec4 hexToRGB(const GMString& hex);
	static GMString toUnixString(const GMString& string);
	static GMString toWin32String(const GMString& string);
	static GMString toCurrentEnvironmentString(const GMString& string);
	static GMBuffer fromBase64(const GMBuffer& base64);
	static GMBuffer toBase64(const GMBuffer& buffer);
};

enum class GMClipboardMIME
{
	Text,
	Bitmap,
	Riff,
	Wave,
	UnicodeText,
	EndOfEnum,
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

	static ErrorCode inflate(const GMBuffer& buf, REF GMBuffer& out, GMsize_t sizeHint);

private:
	static ErrorCode translateError(GMint32);
};

END_NS
#endif
