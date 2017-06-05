#ifndef __GMCONFIG_H__
#define __GMCONFIG_H__
#include "foundation/gmobject.h"
#include <map>
#include <string>
BEGIN_NS

union GMConfigValueStruct
{
	GMLargeInteger i64;
	GMint i32;
	GMfloat f32;
	bool boolean;
	void* ptr;
	char* str;
};

enum GMConfigValueType
{
	VT_Float32,
	VT_Int32,
	VT_Int64,
	VT_String,
};

struct GMConfigValue : public GMAlignmentObject
{
	DEFAULT_CONSTRUCTOR(GMConfigValue);

	GMConfigValue(const char* str)
	{
		type = VT_String;
		size_t len = strlen(str);
		value.str = new char[len];
		strcpy_s(value.str, len, str);
	}

	mutable GMConfigValueStruct value;
	GMConfigValueType type;
};

inline bool operator < (const GMConfigValue& lhs, const GMConfigValue& rhs)
{
	return lhs.value.i32 < lhs.value.i32;
}

inline bool operator == (const GMConfigValue& lhs, const GMConfigValue& rhs)
{
	if (lhs.type != rhs.type)
		return false;

	switch (lhs.type)
	{
	case VT_String:
		return strEqual(lhs.value.str, rhs.value.str);
	case VT_Float32:
		return lhs.value.f32 == rhs.value.f32;
	case VT_Int32:
		return lhs.value.i32 == rhs.value.i32;
	case VT_Int64:
	default:
		return lhs.value.i64 == rhs.value.i64;
	}
}

GM_PRIVATE_OBJECT(GMConfig)
{
	std::map<GMConfigValue, GMConfigValue> values;
};

class GMConfig : public GMObject
{
	DECLARE_PRIVATE(GMConfig)

	friend class GameMachine;

public:
	~GMConfig();

private:
	DEFAULT_CONSTRUCTOR(GMConfig);

public:
	void setInt32(const GMConfigValue& key, GMint i);
	GMint getInt32(const GMConfigValue& key, GMint defaultValue, bool* hasValue = nullptr, bool* correctType = nullptr);
	void setInt64(const GMConfigValue& key, GMLargeInteger i);
	GMLargeInteger getInt64(const GMConfigValue& key, GMLargeInteger defaultValue, bool* hasValue = nullptr, bool* correctType = nullptr);
	void setFloat32(const GMConfigValue& key, GMfloat i);
	GMfloat getFloat32(const GMConfigValue& key, GMfloat defaultValue, bool* hasValue = nullptr, bool* correctType = nullptr);
	void setString(const GMConfigValue& key, const char* str);
	const char* getString(const GMConfigValue& key, const char* defaultValue, bool* hasValue = nullptr, bool* correctType = nullptr);

private:
	GMConfigValue* find(const GMConfigValue& key);
	void releaseString(const GMConfigValue* value);
};

END_NS
#endif