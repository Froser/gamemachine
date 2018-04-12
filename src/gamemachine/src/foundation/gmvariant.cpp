#include "stdafx.h"
#include "assert.h"
#include "linearmath.h"
#include "gmvariant.h"

#define BEGIN_DEFINE_TYPE_ENUM_MAP() template <typename T> struct TypeEnumMap { enum { Value = GMVariant::Unknown }; };
#define DEFINE_TYPE_ENUM_MAP(Type, Enum) template <> struct TypeEnumMap<Type> { enum { Value = Enum }; };
#define END_DEFINE_TYPE_ENUM_MAP()

BEGIN_DEFINE_TYPE_ENUM_MAP()
	DEFINE_TYPE_ENUM_MAP(GMString, GMVariant::String)
	DEFINE_TYPE_ENUM_MAP(GMStates, GMVariant::States)
END_DEFINE_TYPE_ENUM_MAP()

template <typename T>
void GMVariant::makeOwned(const T& obj)
{
	clearOwned();
	m_data.p = new T(obj);
}

template <typename T>
bool GMVariant::deleteOwned()
{
	GM_ASSERT(m_type != GMVariant::Unknown);
	if (m_type == ((GMVariant::Type)(TypeEnumMap<T>::Value)))
	{
		GM_delete(reinterpret_cast<T*>(m_data.p));
		m_data.p = nullptr;
	}
	return false;
}

template <typename T, GMVariant::Type Type>
const T& GMVariant::get(const void* const data) const
{
	static T defaultValue = T();
	if (m_type == Type)
		return reinterpret_cast<const T&>(*reinterpret_cast<const T*>(data));
	return defaultValue;
}

void GMVariant::clearOwned()
{
	(
		deleteOwned<GMString>() ||
		deleteOwned<GMStates>()
	);
}

GMVariant::GMVariant(GMint i32)
	: m_type(I32)
{
	m_data.i32 = i32;
}

GMVariant::GMVariant(GMint64 i64)
	: m_type(I64)
{
	m_data.i64 = i64;
}

GMVariant::GMVariant(GMuint u)
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
	: m_type(Vec2)
{
	m_data.v2 = v;
}

GMVariant::GMVariant(const GMVec3& v)
	: m_type(Vec3)
{
	m_data.v3 = v;
}

GMVariant::GMVariant(const GMVec4& v)
	: m_type(Vec4)
{
	m_data.v4 = v;
}

GMVariant::GMVariant(const GMQuat& q)
	: m_type(Quat)
{
	m_data.q = q;
}

GMVariant::GMVariant(const GMMat4& m)
	: m_type(Mat4)
{
	m_data.m = m;
}

GMVariant::GMVariant(const GMString& s)
	: m_type(String)
{
	makeOwned(s);
}

GMVariant::GMVariant(void* p)
	: m_type(Pointer)
{
	m_data.p = p;
}

GMVariant::GMVariant(const GMVariant& rhs)
{
	clearOwned();
	memcpy(&m_data, &rhs.m_data, sizeof(m_data));
	m_type = rhs.m_type;
}

GMVariant::GMVariant(const GMStates& s)
	: m_type(States)
{
	//makeOwned(new GMStates(s));
}

GMint GMVariant::toInt() const
{
	return get<GMint, I32>(&m_data.i32);
}

GMint64 GMVariant::toInt64() const
{
	return get<GMint, I64>(&m_data.i64);
}

GMuint GMVariant::toUInt() const
{
	return get<GMint, UInt>(&m_data.u);
}

GMfloat GMVariant::toFloat() const
{
	return get<GMfloat, Float>(&m_data.f);
}

bool GMVariant::toBool() const
{
	return get<bool, Boolean>(&m_data.b);
}

const GMVec2& GMVariant::toVec2() const
{
	return get<GMVec2, Vec2>(&m_data.v2);
}

const GMVec3& GMVariant::toVec3() const
{
	return get<GMVec3, Vec3>(&m_data.v3);
}

const GMVec4& GMVariant::toVec4() const
{
	return get<GMVec4, Vec4>(&m_data.v4);
}

const GMQuat& GMVariant::toQuat() const
{
	return get<GMQuat, Quat>(&m_data.q);
}

const GMMat4& GMVariant::toMat4() const
{
	return get<GMMat4, Mat4>(&m_data.m);
}

void* GMVariant::toPointer() const
{
	return get<void*, Pointer>(&m_data.p);
}

const GMString& GMVariant::toString() const
{
	return get<GMString, String>(m_data.p);
}

const GMStates& GMVariant::toStates() const
{
	return get<GMStates, States>(m_data.p);
}
