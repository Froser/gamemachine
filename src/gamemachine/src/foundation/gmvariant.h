#ifndef __GMVARIANT_H__
#define __GMVARIANT_H__
#include <defines.h>
#include <linearmath.h>
BEGIN_NS

class GMString;
class GMObject;
struct GMVariant
{
public:
	enum Type
	{
		Unknown,
		I32,
		I64,
		UInt,
		Float,
		Boolean,
		Vec2,
		Vec3,
		Vec4,
		Quat,
		Mat4,
		String,
		ObjectPointer,
		Pointer,
	};

	union Data
	{
		GMint i32;
		GMuint u;
		GMfloat f;
		bool b;
		void* p;
		GMint64 i64;
#if GM_USE_DX11
		GMVec2 v2;
		GMVec3 v3;
		GMVec4 v4;
		GMQuat q;
		GMMat4 m;
#endif
	};

	GMVariant();
	GMVariant(GMint);
	GMVariant(GMint64);
	GMVariant(GMuint);
	GMVariant(GMfloat);
	GMVariant(bool);
	GMVariant(const GMVec2&);
	GMVariant(const GMVec3&);
	GMVariant(const GMVec4&);
	GMVariant(const GMQuat&);
	GMVariant(const GMMat4&);
	GMVariant(const GMString&);
	GMVariant(const char*);
	GMVariant(const GMwchar*);
	GMVariant(GMObject*);
	GMVariant(void*);
	~GMVariant();

	GMVariant(const GMVariant&);
	GMVariant(GMVariant&&);
	GMVariant& operator=(const GMVariant&);
	GMVariant& operator=(GMVariant&&);

public:
	GMint toInt() const;
	GMint64 toInt64() const;
	GMuint toUInt() const;
	GMfloat toFloat() const;
	GMObject* toObject() const;
	void* toPointer() const;
	bool toBool() const;

	const GMVec2& toVec2() const;
	GMVec2& toVec2();
	const GMVec3& toVec3() const;
	GMVec3& toVec3();
	const GMVec4& toVec4() const;
	GMVec4& toVec4();
	const GMQuat& toQuat() const;
	GMQuat& toQuat();
	const GMMat4& toMat4() const;
	GMMat4& toMat4();
	const GMString& toString() const;
	GMString& toString();

	template <typename T>
	T toEnum() const
	{
		return (T)toInt();
	}

	template <typename T>
	T objectCast()
	{
		return (T)toObject();
	}

	bool isInt() const { return m_type == I32; }
	bool isInt64() const { return m_type == I64; }
	bool isUInt() const { return m_type == UInt; }
	bool isFloat() const { return m_type == Float; }
	bool isPointer() const { return m_type == Pointer; }
	bool isBool() const { return m_type == Boolean; }
	bool isVec2() const { return m_type == Vec2; }
	bool isVec3() const { return m_type == Vec3; }
	bool isVec4() const { return m_type == Vec4; }
	bool isQuat() const { return m_type == Quat; }
	bool isMat4() const { return m_type == Mat4; }
	bool isString() const { return m_type == String; }
	bool isObject() const { return m_type == ObjectPointer; }

private:
	template <typename T> void makeOwned(const T& obj);
	template <GMVariant::Type Type> void copyOwned(const GMVariant& rhs);
	template <typename T> bool deleteOwned();
	template <typename T> const T& get(const void* const data) const;
	void clearOwned();

private:
	Data m_data;
	Type m_type = Unknown;
};

END_NS
#endif