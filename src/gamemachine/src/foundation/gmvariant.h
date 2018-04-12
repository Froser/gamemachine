#ifndef __GMVARIANT_H__
#define __GMVARIANT_H__
#include <gmcommon.h>

struct GMVec2;
struct GMVec3;
struct GMVec4;
struct GMQuat;
struct GMMat4;

BEGIN_NS

class GMString;
class GMStates;

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
		States,
		Pointer,
	};

	union Data
	{
		GMint i32;
		GMuint u;
		GMfloat f;
		bool b;
		void* p;
		GMint64 i64 = 0;
		GMVec2 v2;
		GMVec3 v3;
		GMVec4 v4;
		GMQuat q;
		GMMat4 m;
	};

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
	GMVariant(void*);
	GMVariant(const GMStates&);
	GMVariant(const GMVariant&);

public:
	GMint toInt() const;
	GMint64 toInt64() const;
	GMuint toUInt() const;
	GMfloat toFloat() const;
	bool toBool() const;
	const GMVec2& toVec2() const;
	const GMVec3& toVec3() const;
	const GMVec4& toVec4() const;
	const GMQuat& toQuat() const;
	const GMMat4& toMat4() const;
	void* toPointer() const;
	const GMString& toString() const;
	const GMStates& toStates() const;

private:
	template <typename T> void makeOwned(const T& obj);
	template <typename T> bool deleteOwned();
	template <typename T, GMVariant::Type Type> const T& get(const void* const data) const;
	void clearOwned();

private:
	Data m_data;
	Type m_type = Unknown;
};

END_NS
#endif