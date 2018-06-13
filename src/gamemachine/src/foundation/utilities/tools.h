#ifndef __UTILITIES_H__
#define __UTILITIES_H__
#include <gmcommon.h>
#include <linearmath.h>
#include "../vector.h"
#include <time.h>
BEGIN_NS

// 此类包含了各种实用工具
// GMScopePtr:
template<class Type>
struct GMScopePtrRef
{
	explicit GMScopePtrRef(Type *right)
		: m_ref(right)
	{
	}

	Type *m_ref;
};

template<class Type>
class GMScopePtr
{
public:
	typedef GMScopePtr<Type> MyType;
	typedef Type ElementType;

	explicit GMScopePtr(Type *ptr = nullptr)
		: m_ptr(ptr)
	{
	}

	GMScopePtr(MyType& right)
		: m_ptr(right.release())
	{
	}

	GMScopePtr(GMScopePtrRef<Type> right)
	{
		Type *ptr = right.m_ref;
		right.m_ref = 0;
		m_ptr = ptr;
	}

	template<class OtherType>
	operator GMScopePtr<OtherType>()
	{
		return (GMScopePtr<OtherType>(*this));
	}

	template<class OtherType>
	operator GMScopePtrRef<OtherType>()
	{
		OtherType *Cvtptr = m_ptr;
		GMScopePtrRef<OtherType> _Ans(Cvtptr);
		m_ptr = 0;
		return (_Ans);
	}

	template<class OtherType>
	MyType& operator=(GMScopePtr<OtherType>& right)
	{
		reset(right.release());
		return (*this);
	}

	template<class OtherType>
	GMScopePtr(GMScopePtr<OtherType>& right)
		: m_ptr(right.release())
	{
	}

	MyType& operator=(MyType& right)
	{
		reset(right.release());
		return (*this);
	}

	MyType& operator=(GMScopePtrRef<Type> right)
	{
		Type *ptr = right.m_ref;
		right.m_ref = 0;
		reset(ptr);
		return (*this);
	}

	~GMScopePtr()
	{
		delete m_ptr;
	}

	Type& operator*() const
	{
		return (*get());
	}

	Type *operator->() const
	{
		return (get());
	}

	Type *get() const
	{
		return (m_ptr);
	}

	Type *release()
	{
		Type *_Tmp = m_ptr;
		m_ptr = 0;
		return (_Tmp);
	}

	operator Type *()
	{
		return (m_ptr);
	}

	operator bool()
	{
		return !!m_ptr;
	}

	void reset(Type *ptr = nullptr)
	{
		if (m_ptr && ptr != m_ptr)
			delete m_ptr;
		m_ptr = ptr;
	}

private:
	Type *m_ptr;
};

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
	GM_DISABLE_ASSIGN(GMEvent)
	GM_DISABLE_COPY(GMEvent)
	GM_DEFAULT_MOVE_BEHAVIOR(GMEvent)

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

// Unit conversion
struct GMConvertion
{
	static GMfloat pointToInch(GMint pt);
	static GMfloat pointToPixel(GMint pt);
	static bool hexToRGB(const GMString& hex, GMfloat rgb[3]);
};

END_NS
#endif
