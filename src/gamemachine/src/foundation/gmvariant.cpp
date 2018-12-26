#include "stdafx.h"
#include "assert.h"
#include "gmstring.h"
#include "gmvariant.h"

#define BEGIN_DEFINE_TYPE_ENUM_MAP() \
	template <typename T> struct TypeEnumMap { enum { Value = GMVariant::Unknown }; }; \
	struct InvalidType {}; \
	template <GMVariant::Type> struct EnumTypeMap { typedef InvalidType StructType; };

#define DEFINE_TYPE_ENUM_MAP(Type, Enum) \
	template <> struct TypeEnumMap<Type> { enum { Value = Enum }; }; \
	template <> struct EnumTypeMap<Enum> { typedef Type StructType; };

#define END_DEFINE_TYPE_ENUM_MAP()

BEGIN_DEFINE_TYPE_ENUM_MAP()
	DEFINE_TYPE_ENUM_MAP(GMint32, GMVariant::I32)
	DEFINE_TYPE_ENUM_MAP(GMint64, GMVariant::I64)
	DEFINE_TYPE_ENUM_MAP(GMuint32, GMVariant::UInt)
	DEFINE_TYPE_ENUM_MAP(GMfloat, GMVariant::Float)
	DEFINE_TYPE_ENUM_MAP(bool, GMVariant::Boolean)
	DEFINE_TYPE_ENUM_MAP(GMObject&, GMVariant::ObjectRef)
	DEFINE_TYPE_ENUM_MAP(GMObject*, GMVariant::ObjectPointer)
	DEFINE_TYPE_ENUM_MAP(void*, GMVariant::Pointer)
	DEFINE_TYPE_ENUM_MAP(GMVec2, GMVariant::Vec2)
	DEFINE_TYPE_ENUM_MAP(GMVec3, GMVariant::Vec3)
	DEFINE_TYPE_ENUM_MAP(GMVec4, GMVariant::Vec4)
	DEFINE_TYPE_ENUM_MAP(GMQuat, GMVariant::Quat)
	DEFINE_TYPE_ENUM_MAP(GMMat4, GMVariant::Mat4)
	DEFINE_TYPE_ENUM_MAP(GMString, GMVariant::String)
END_DEFINE_TYPE_ENUM_MAP()

template <typename T>
void GMVariant::makeOwned(const T& obj)
{
	m_data.p = new T(obj);
}

template <GMVariant::Type T>
void GMVariant::copyOwned(const GMVariant& rhs)
{
	typedef typename EnumTypeMap<T>::StructType RealType;
	m_data.p = new RealType(rhs.get<RealType>(rhs.m_data.p));
}

template <typename T>
bool GMVariant::deleteOwned()
{
	if (m_type == ((GMVariant::Type)(TypeEnumMap<T>::Value)))
	{
		GM_delete(reinterpret_cast<T*>(m_data.p));
		m_data.p = nullptr;
		return true;
	}
	return false;
}

template <typename T>
const T& GMVariant::get(const void* const data) const
{
	enum { Type = TypeEnumMap<T>::Value };
	GM_STATIC_ASSERT(Type != Unknown, "Unknown type");
	static T defaultValue = T();
	if (m_type == Type)
		return reinterpret_cast<const T&>(*reinterpret_cast<const T*>(data));
	GM_ASSERT(!"Type not match.");
	return defaultValue;
}

template <typename T>
const T& GMVariant::rawGet(const void* const data) const
{
	enum { Type = TypeEnumMap<T>::Value };
	GM_STATIC_ASSERT(Type != Unknown, "Unknown type");
	return reinterpret_cast<const T&>(*reinterpret_cast<const T*>(data));
}

void GMVariant::clearOwned()
{
	if (
#if !GM_USE_DX11
		deleteOwned<GMVec2>() ||
		deleteOwned<GMVec3>() ||
		deleteOwned<GMVec4>() ||
		deleteOwned<GMQuat>() ||
		deleteOwned<GMMat4>() ||
#endif
		deleteOwned<GMString>()
	)
	{
		m_type = Unknown;
	}
}

GMVariant::GMVariant()
	: m_type(Unknown)
{
}

GMVariant::GMVariant(GMint32 i32)
	: m_type(I32)
{
	m_data.i32 = i32;
}

GMVariant::GMVariant(GMint64 i64)
	: m_type(I64)
{
	m_data.i64 = i64;
}

GMVariant::GMVariant(GMuint32 u)
	: m_type(UInt)
{
	m_data.u = u;
}

GMVariant::GMVariant(GMfloat f)
	: m_type(Float)
{
	m_data.f = f;
}

GMVariant::GMVariant(bool b)
	: m_type(Boolean)
{
	m_data.b = b;
}

GMVariant::GMVariant(const GMVec2& v)
{
#if GM_USE_DX11
	m_data.v2 = v;
#else
	makeOwned<GMVec2>(v);
#endif
	m_type = Vec2;
}

GMVariant::GMVariant(const GMVec3& v)
{
#if GM_USE_DX11
	m_data.v3 = v;
#else
	makeOwned<GMVec3>(v);
#endif
	m_type = Vec3;
}

GMVariant::GMVariant(const GMVec4& v)
{
#if GM_USE_DX11
	m_data.v4 = v;
#else
	makeOwned<GMVec4>(v);
#endif
	m_type = Vec4;
}

GMVariant::GMVariant(const GMQuat& q)
{
#if GM_USE_DX11
	m_data.q = q;
#else
	makeOwned<GMQuat>(q);
#endif
	m_type = Quat;
}

GMVariant::GMVariant(const GMMat4& m)
{
#if GM_USE_DX11
	m_data.m = m;
#else
	makeOwned<GMMat4>(m);
#endif
	m_type = Mat4;
}

GMVariant::GMVariant(const GMString& s)
{
	makeOwned(s);
	m_type = String;
}

GMVariant::GMVariant(const char* s)
	: GMVariant(GMString(s))
{
}

GMVariant::GMVariant(const GMwchar* s)
	: GMVariant(GMString(s))
{
}

GMVariant::GMVariant(GMObject& p)
	: m_type(ObjectRef)
{
	m_data.p = &p;
}

GMVariant::GMVariant(GMObject* p)
	: m_type(ObjectPointer)
{
	m_data.p = p;
}

GMVariant::GMVariant(void* p)
	: m_type(Pointer)
{
	m_data.p = p;
}

GMVariant::~GMVariant()
{
	clearOwned();
}

GMVariant::GMVariant(const GMVariant& rhs)
{
	*this = rhs;
}

GMVariant::GMVariant(GMVariant&& rhs)
{
	*this = std::move(rhs);
}

GMVariant& GMVariant::operator=(const GMVariant& rhs)
{
	clearOwned();
	if (rhs.m_type == String)
		copyOwned<String>(rhs);
#if !GM_USE_DX11
	else if (rhs.m_type == Vec2)
		copyOwned<Vec2>(rhs);
	else if (rhs.m_type == Vec3)
		copyOwned<Vec3>(rhs);
	else if (rhs.m_type == Vec4)
		copyOwned<Vec4>(rhs);
	else if (rhs.m_type == Quat)
		copyOwned<Quat>(rhs);
	else if (rhs.m_type == Mat4)
		copyOwned<Mat4>(rhs);
#endif
	else
		memcpy(&m_data, &rhs.m_data, sizeof(rhs.m_data));
	m_type = rhs.m_type;
	return *this;
}

GMVariant& GMVariant::operator=(GMVariant&& rhs)
{
	memcpy(&m_data, &rhs.m_data, sizeof(rhs.m_data));
	m_type = rhs.m_type;
	rhs.m_type = Unknown;
	return *this;
}

GMint32 GMVariant::toInt() const
{
	if (m_type == I64)
	{
		// 窄化转型，检查是否越界
		GMint64 i64 = toInt64();
		GMint64 maxI64 = static_cast<GMint64>(std::numeric_limits<GMint32>::max());
		GM_ASSERT(i64 <= maxI64 || !"GMVariant (toInt): int64 to int32 convertion overflow.");
		return static_cast<GMint32>(i64);
	}
	return get<GMint32>(&m_data.i32);
}

GMint64 GMVariant::toInt64() const
{
	if (m_type == I32)
		return get<GMint32>(&m_data.i32);
	return get<GMint64>(&m_data.i64);
}

GMuint32 GMVariant::toUInt() const
{
	return get<GMuint32>(&m_data.u);
}

GMfloat GMVariant::toFloat() const
{
	if (m_type == I32 || m_type == I64)
		return rawGet<GMint32>(&m_data.i32);
	if (m_type == UInt)
		return rawGet<GMuint32>(&m_data.u);

	return get<GMfloat>(&m_data.f);
}

GMObject* GMVariant::toObject() const
{
	if (m_type == ObjectRef)
		const_cast<GMVariant*>(this)->m_type = ObjectPointer;
	return get<GMObject*>(&m_data.p);
}

bool GMVariant::toBool() const
{
	return get<bool>(&m_data.b);
}

void* GMVariant::toPointer() const
{
	return get<void*>(&m_data.p);
}

const GMVec2& GMVariant::toVec2() const
{
#if GM_USE_DX11
	return get<GMVec2>(&m_data.v2);
#else
	return get<GMVec2>(m_data.p);
#endif
}

GMVec2& GMVariant::toVec2()
{
	const GMVariant* pThis = this;
	return const_cast<GMVec2&>(pThis->toVec2());
}

const GMVec3& GMVariant::toVec3() const
{
#if GM_USE_DX11
	return get<GMVec3>(&m_data.v3);
#else
	return get<GMVec3>(m_data.p);
#endif
}

GMVec3& GMVariant::toVec3()
{
	const GMVariant* pThis = this;
	return const_cast<GMVec3&>(pThis->toVec3());
}

const GMVec4& GMVariant::toVec4() const
{
#if GM_USE_DX11
	return get<GMVec4>(&m_data.v4);
#else
	return get<GMVec4>(m_data.p);
#endif
}

GMVec4& GMVariant::toVec4()
{
	const GMVariant* pThis = this;
	return const_cast<GMVec4&>(pThis->toVec4());
}

const GMQuat& GMVariant::toQuat() const
{
#if GM_USE_DX11
	return get<GMQuat>(&m_data.q);
#else
	return get<GMQuat>(m_data.p);
#endif
}

GMQuat& GMVariant::toQuat()
{
	const GMVariant* pThis = this;
	return const_cast<GMQuat&>(pThis->toQuat());
}

const GMMat4& GMVariant::toMat4() const
{
#if GM_USE_DX11
	return get<GMMat4>(&m_data.m);
#else
	return get<GMMat4>(m_data.p);
#endif
}

GMMat4& GMVariant::toMat4()
{
	const GMVariant* pThis = this;
	return const_cast<GMMat4&>(pThis->toMat4());
}

const GMString& GMVariant::toString() const
{
	return get<GMString>(m_data.p);
}

GMString& GMVariant::toString()
{
	const GMVariant* pThis = this;
	return const_cast<GMString&>(pThis->toString());
}