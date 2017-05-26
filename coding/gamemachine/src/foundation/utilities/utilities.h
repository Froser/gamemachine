#ifndef __UTILITIES_H__
#define __UTILITIES_H__
#include "common.h"
#include "foundation/linearmath.h"
#include "foundation/vector.h"
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

//ComPtr:
#ifdef _WINDOWS
#define __uuid(c) __uuidof(c)

template <class E>
class ComPtr
{
public:
	ComPtr() { m_ptr = NULL; }
	ComPtr(E* p)
	{
		m_ptr = p;
		if (m_ptr)
			m_ptr->AddRef();
	}
	ComPtr(const ComPtr<E>& p)
	{
		m_ptr = p.m_ptr;
		if (m_ptr)
			m_ptr->AddRef();
	}
	ComPtr(IUnknown* pUnk) : m_ptr(NULL)
	{
		if (pUnk)
			pUnk->QueryInterface(__uuid(E), (void**)&m_ptr);
	}

	ComPtr(IUnknown* pUnk, REFIID iid) : m_ptr(NULL)
	{
		if (pUnk)
			pUnk->QueryInterface(iid, (void**)&m_ptr);
	}

	template <class Type>
	ComPtr(const ComPtr<Type>& p) : m_ptr(NULL)
	{
		if (p)
			p->QueryInterface(__uuid(E), (void**)&m_ptr);
	}

	~ComPtr()
	{
		if (m_ptr)
			m_ptr->Release();
	}
	void attach(E* p)
	{
		if (m_ptr)
			m_ptr->Release();
		m_ptr = p;
	}
	void detach(E** ppv)
	{
		ASSERT(0); // 过时的用法!
		ASSERT(ppv != &m_ptr);
		*ppv = m_ptr;
		m_ptr = NULL;
	}
	E* detach()
	{
		E* tmp = m_ptr;
		m_ptr = NULL;
		return tmp;
	}
	void clear()
	{
		if (m_ptr)
		{
			m_ptr->Release();
			m_ptr = NULL;
		}
	}
	E* operator=(E* p)
	{
		if (p)
			p->AddRef();
		if (m_ptr)
			m_ptr->Release();
		return m_ptr = p;
	}
	E* operator=(const ComPtr<E>& p)
	{
		if (p.m_ptr)
			p.m_ptr->AddRef();
		if (m_ptr)
			m_ptr->Release();
		return m_ptr = p.m_ptr;
	}
	E* operator=(int __nil)
	{
		ASSERT(__nil == 0);
		if (m_ptr)
		{
			m_ptr->Release();
			m_ptr = NULL;
		}
		return NULL;
	}
	E* operator=(long __nil)
	{
		ASSERT(__nil == 0);
		if (m_ptr)
		{
			m_ptr->Release();
			m_ptr = NULL;
		}
		return NULL;
	}
	E* operator=(IUnknown* pUnk)
	{
		E* pTemp = NULL;
		if (pUnk)
			pUnk->QueryInterface(__uuid(E), (void**)&pTemp);
		if (m_ptr)
			m_ptr->Release();
		return m_ptr = pTemp;
	}

	template <class Type>
	E* operator=(const ComPtr<Type>& p)
	{
		E* pTemp = NULL;
		if (p)
			p->QueryInterface(__uuid(E), (void**)&pTemp);
		if (m_ptr)
			m_ptr->Release();
		return m_ptr = pTemp;
	}

	BOOL IsEqualObject(IUnknown* pOther) const
	{
		if (pOther)
		{
			if (m_ptr == NULL)
				return FALSE;
			IUnknown* pThis;
			m_ptr->QueryInterface(IID_IUnknown, (void**)&pThis);
			pOther->QueryInterface(IID_IUnknown, (void**)&pOther);
			pThis->Release();
			pOther->Release();
			return (pThis == pOther);
		}
		return (m_ptr == NULL);
	}
	E& operator*() const
	{
		return *m_ptr;
	}
	operator E*() const
	{
		return m_ptr;
	}
	E* get() const
	{
		return m_ptr;
	}
	E** operator&() // The assert on operator& usually indicates a bug.
	{
		ASSERT(m_ptr == NULL); return &m_ptr;
	}
	E* operator->() const
	{
		return m_ptr;
	}
	BOOL operator!() const
	{
		return m_ptr == NULL;
	}
	BOOL operator==(E* p) const
	{
		return m_ptr == p;
	}
	BOOL operator!=(E* p) const
	{
		return m_ptr != p;
	}

protected:
	E* m_ptr;
};
#endif

//FPSCounter
GM_PRIVATE_OBJECT(FPSCounter)
{
	GMfloat fps;
	GMfloat lastTime;
	GMfloat time;
	GMfloat immediate_lastTime;
	GMfloat elapsed_since_last_frame;
	GMlong frames;
};

class FPSCounter
{
	DECLARE_PRIVATE(FPSCounter)

public:
	FPSCounter();
	~FPSCounter() {}

public:
	void update();
	GMfloat getFps();
	GMfloat getElapsedSinceLastFrame();
};

//Plane
enum PointPosition
{
	POINT_ON_PLANE = 0,
	POINT_IN_FRONT_OF_PLANE,
	POINT_BEHIND_PLANE,
};

struct Plane
{
	Plane() : normal(linear_math::Vector3(0.0f, 0.0f, 0.0f)), intercept(0.0f)
	{}
	Plane(const linear_math::Vector3& newNormal, GMfloat newIntercept) : normal(newNormal), intercept(newIntercept)
	{}
	Plane(const Plane & rhs)
	{
		normal = rhs.normal;
		intercept = rhs.intercept;
	}

	~Plane() {}

	void setNormal(const linear_math::Vector3 & rhs) { normal = rhs; }
	void setIntercept(GMfloat newIntercept) { intercept = newIntercept; }
	void setFromPoints(const linear_math::Vector3 & p0, const linear_math::Vector3 & p1, const linear_math::Vector3 & p2);

	void calculateIntercept(const linear_math::Vector3 & pointOnPlane) { intercept = -linear_math::dot(normal, pointOnPlane); }

	void normalize(void);

	linear_math::Vector3 getNormal() { return normal; }
	GMfloat getIntercept() { return intercept; }

	//find point of intersection of 3 planes
	bool intersect3(const Plane & p2, const Plane & p3, linear_math::Vector3 & result);

	GMfloat getDistance(const linear_math::Vector3 & point) const;
	PointPosition classifyPoint(const linear_math::Vector3 & point) const;

	Plane lerp(const Plane & p2, GMfloat factor);

	//operators
	bool operator==(const Plane & rhs) const;
	bool operator!=(const Plane & rhs) const
	{
		return!((*this) == rhs);
	}

	//unary operators
	Plane operator-(void) const { return Plane(-normal, -intercept); }
	Plane operator+(void) const { return (*this); }

	//member variables
	linear_math::Vector3 normal;	//X.N+intercept=0
	GMfloat intercept;
};


//Frustum
class Frustum
{
public:
	Frustum(GMfloat fovy, GMfloat aspect, GMfloat n, GMfloat f);

public:
	void update();
	bool isPointInside(const linear_math::Vector3& point);
	bool isBoundingBoxInside(const linear_math::Vector3* vertices);
	linear_math::Matrix4x4 getPerspective();
	void updateViewMatrix(linear_math::Matrix4x4& viewMatrix, linear_math::Matrix4x4& projMatrix);

private:
	Plane planes[6];
	GMfloat m_fovy;
	GMfloat m_aspect;
	GMfloat m_n;
	GMfloat m_f;
	linear_math::Matrix4x4 m_viewMatrix;
	linear_math::Matrix4x4 m_projMatrix;
};

//Scanner
typedef bool(*CharPredicate)(char in);

class Scanner
{
public:
	explicit Scanner(const char* line);
	explicit Scanner(const char* line, CharPredicate predicate);
	explicit Scanner(const char* line, bool skipSame, CharPredicate predicate);

public:
	void next(char* out);
	void nextToTheEnd(char* out);
	bool nextFloat(GMfloat* out);
	bool nextInt(GMint* out);

private:
	const char* m_p;
	bool m_skipSame;
	CharPredicate m_predicate;
	bool m_valid;
};

//MemoryStream
class MemoryStream
{
public:
	enum SeekMode
	{
		FromStart,
		FromNow,
	};

public:
	MemoryStream(const GMbyte* data, GMuint size);

public:
	GMuint read(GMbyte* buf, GMuint size);
	GMuint peek(GMbyte* buf, GMuint size);
	void seek(GMuint cnt, SeekMode = FromStart);
	void rewind();
	GMuint size();
	GMuint tell();
	GMbyte get();

private:
	const GMbyte* m_ptr;
	const GMbyte* m_start;
	const GMbyte* m_end;
	GMuint m_size;
};

//Bitset
class Bitset
{
public:
	Bitset() : numBytes(0), bits(NULL)
	{}
	~Bitset()
	{
		if (bits)
			delete[] bits;
		bits = NULL;
	}

	bool init(int numberOfBits);
	void clearAll();
	void setAll();

	void clear(int bitNumber);
	void set(int bitNumber);

	unsigned char isSet(int bitNumber);

protected:
	GMint numBytes;	//size of bits array
	unsigned char* bits;
};

//Camera

struct CameraLookAt
{
	linear_math::Vector3 lookAt;
	linear_math::Vector3 position;
};

struct PositionState
{
	GMfloat yaw;
	GMfloat pitch;
	linear_math::Vector3 position;
	GMfloat pitchLimitRad;
};

inline linear_math::Matrix4x4 getViewMatrix(const CameraLookAt& lookAt)
{
	return linear_math::lookat(lookAt.position, lookAt.lookAt + lookAt.position, linear_math::Vector3(0, 1, 0));
}

class Camera
{
public:
	static void calcCameraLookAt(const PositionState& state, REF CameraLookAt& lookAt);
};

//Path
struct Path
{
	static std::string directoryName(const std::string& fileName);
	static std::string filename(const std::string& fullPath);
	static std::string getCurrentPath();
	static AlignedVector<std::string> getAllFiles(const char* directory);
	static bool directoryExists(const std::string& dir);
	static void createDirectory(const std::string& dir);
};

END_NS
#endif