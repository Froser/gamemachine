#ifndef __UTILITIES_H__
#define __UTILITIES_H__
#include <gmcommon.h>
#include <linearmath.h>
#include "../vector.h"
#include <time.h>
BEGIN_NS

// 此类包含了各种实用工具
// AutoPtr:
template<class Type>
struct AutoPtrRef
{
	explicit AutoPtrRef(Type *right)
		: m_ref(right)
	{
	}

	Type *m_ref;
};

template<class Type>
class AutoPtr
{
public:
	typedef AutoPtr<Type> MyType;
	typedef Type ElementType;

	explicit AutoPtr(Type *ptr = nullptr)
		: m_ptr(ptr)
	{
	}

	AutoPtr(MyType& right)
		: m_ptr(right.release())
	{
	}

	AutoPtr(AutoPtrRef<Type> right)
	{
		Type *ptr = right.m_ref;
		right.m_ref = 0;
		m_ptr = ptr;
	}

	template<class OtherType>
	operator AutoPtr<OtherType>()
	{
		return (AutoPtr<OtherType>(*this));
	}

	template<class OtherType>
	operator AutoPtrRef<OtherType>()
	{
		OtherType *Cvtptr = m_ptr;
		AutoPtrRef<OtherType> _Ans(Cvtptr);
		m_ptr = 0;
		return (_Ans);
	}

	template<class OtherType>
	MyType& operator=(AutoPtr<OtherType>& right)
	{
		reset(right.release());
		return (*this);
	}

	template<class OtherType>
	AutoPtr(AutoPtr<OtherType>& right)
		: m_ptr(right.release())
	{
	}

	MyType& operator=(MyType& right)
	{
		reset(right.release());
		return (*this);
	}

	MyType& operator=(AutoPtrRef<Type> right)
	{
		Type *ptr = right.m_ref;
		right.m_ref = 0;
		reset(ptr);
		return (*this);
	}

	~AutoPtr()
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
	GMLargeInteger frequency;
	GMLargeInteger timeCycles;
	GMLargeInteger begin, end;
	GMLargeInteger deltaCycles;
	GMfloat timeScale;
	GMfloat fps;
	bool paused;

	//以下用于计算帧率
	GMint frameCount;
	GMLargeInteger lastCycle;
};

class GMClock : public GMObject
{
	DECLARE_PRIVATE(GMClock)

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
	static GMLargeInteger highResolutionTimerFrequency();
	static GMLargeInteger highResolutionTimer();

private:
	GMfloat cycleToSecond(GMLargeInteger cycle);
};

//GMStopwatch
GM_PRIVATE_OBJECT(GMStopwatch)
{
	GMLargeInteger frequency;
	GMLargeInteger start;
	GMLargeInteger end;
};

class GMStopwatch : public GMObject
{
	DECLARE_PRIVATE(GMStopwatch);

public:
	GMStopwatch();

public:
	void start();
	void stop();
	GMfloat timeInSecond();
	GMLargeInteger timeInCycle();
	GMfloat nowInSecond();
	GMLargeInteger nowInCycle();
};

//Plane
enum PointPosition
{
	POINT_ON_PLANE = 0,
	POINT_IN_FRONT_OF_PLANE,
	POINT_BEHIND_PLANE,
};

struct GMPlane : public GMObject, public GMAlignmentObject
{
	GMPlane() : normal(linear_math::Vector3(0.0f, 0.0f, 0.0f)), intercept(0.0f)
	{
	}

	GMPlane(const linear_math::Vector3& newNormal, GMfloat newIntercept) : normal(newNormal), intercept(newIntercept)
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

	void setNormal(const linear_math::Vector3 & rhs) { normal = rhs; }
	void setIntercept(GMfloat newIntercept) { intercept = newIntercept; }
	void setFromPoints(const linear_math::Vector3 & p0, const linear_math::Vector3 & p1, const linear_math::Vector3 & p2);

	void calculateIntercept(const linear_math::Vector3 & pointOnPlane) { intercept = -linear_math::dot(normal, pointOnPlane); }

	void normalize(void);

	linear_math::Vector3 getNormal() { return normal; }
	GMfloat getIntercept() { return intercept; }

	//find point of intersection of 3 planes
	bool intersect3(const GMPlane & p2, const GMPlane & p3, linear_math::Vector3 & result);

	GMfloat getDistance(const linear_math::Vector3 & point) const;
	PointPosition classifyPoint(const linear_math::Vector3 & point) const;

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
	linear_math::Vector3 normal;	//X.N+intercept=0
	GMfloat intercept;
};

//Frustum
enum class GMFrustumType
{
	Perspective,
	Orthographic,
};

GM_PRIVATE_OBJECT(GMFrustum)
{
	GMFrustumType type = GMFrustumType::Perspective;
	GMPlane planes[6];
	union
	{
		struct
		{
			GMfloat fovy;
			GMfloat aspect;
		};

		struct
		{
			GMfloat left;
			GMfloat right;
			GMfloat bottom;
			GMfloat top;
		};
	};
	GMfloat n;
	GMfloat f;

	linear_math::Matrix4x4 viewMatrix;
	linear_math::Matrix4x4 projMatrix;
};

class GMFrustum : public GMObject
{
	DECLARE_PRIVATE(GMFrustum)

public:
	GMFrustum() = default;
	void initOrtho(GMfloat left, GMfloat right, GMfloat bottom, GMfloat top, GMfloat n, GMfloat f);
	void initPerspective(GMfloat fovy, GMfloat aspect, GMfloat n, GMfloat f);

public:
	void update();
	bool isPointInside(const linear_math::Vector3& point);
	bool isBoundingBoxInside(const linear_math::Vector3* vertices);
	linear_math::Matrix4x4 getPerspective();
	void updateViewMatrix(linear_math::Matrix4x4& viewMatrix, linear_math::Matrix4x4& projMatrix);
};

//Scanner
typedef bool(*CharPredicate)(char in);

GM_PRIVATE_OBJECT(Scanner)
{
	const char* p;
	bool skipSame;
	CharPredicate predicate;
	bool valid;
};

class Scanner : public GMObject
{
	DECLARE_PRIVATE(Scanner)

public:
	explicit Scanner(const char* line);
	explicit Scanner(const char* line, CharPredicate predicate);
	explicit Scanner(const char* line, bool skipSame, CharPredicate predicate);

public:
	void next(char* out);
	void nextToTheEnd(char* out);
	bool nextFloat(GMfloat* out);
	bool nextInt(GMint* out);
};

//GMMemoryStream
GM_PRIVATE_OBJECT(GMMemoryStream)
{
	const GMbyte* ptr;
	const GMbyte* start;
	const GMbyte* end;
	GMuint size;
};

class GMMemoryStream : public GMObject
{
	DECLARE_PRIVATE(GMMemoryStream)

public:
	enum SeekMode
	{
		FromStart,
		FromNow,
	};

public:
	GMMemoryStream(const GMbyte* buffer, GMuint size);

public:
	GMuint read(GMbyte* buf, GMuint size);
	GMuint peek(GMbyte* buf, GMuint size);
	void seek(GMuint cnt, SeekMode = FromStart);
	void rewind();
	GMuint size();
	GMuint tell();
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
	DECLARE_PRIVATE(Bitset)

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

//Camera
GM_ALIGNED_16(struct) CameraLookAt
{
	linear_math::Vector3 lookAt;
	linear_math::Vector3 position;
};

GM_ALIGNED_16(struct) PositionState
{
	linear_math::Vector3 position;
	GMfloat yaw;
	GMfloat pitch;
};

inline linear_math::Matrix4x4 getViewMatrix(const CameraLookAt& lookAt)
{
	return linear_math::lookat(lookAt.position, lookAt.lookAt + lookAt.position, linear_math::Vector3(0, 1, 0));
}

//GMPath: platforms/[os]/path.cpp
struct GMPath
{
	static GMString directoryName(const GMString& fileName);
	static GMString filename(const GMString& fullPath);
	static GMString getCurrentPath();
	static Vector<GMString> getAllFiles(const GMString& directory);
	static bool directoryExists(const GMString& dir);
	static void createDirectory(const GMString& dir);
};

//GMEvent: platforms/[os]/event.cpp
#if _WINDOWS
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
	DECLARE_PRIVATE(GMEvent)
	GM_DISABLE_ASSIGN(GMEvent)
	GM_DISABLE_COPY(GMEvent)

public:
	GMEvent(GMEvent&& e) noexcept
	{
		swap(e);
	}

	GMEvent& operator=(GMEvent&& e) noexcept
	{
		swap(e);
		return *this;
	}

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

END_NS
#endif
