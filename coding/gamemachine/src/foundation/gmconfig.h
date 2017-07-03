#ifndef __GMCONFIG_H__
#define __GMCONFIG_H__
#include "foundation/gmobject.h"
#include <map>
#include <string>
BEGIN_NS

#define GMCfg (GameMachine::instance().getConfigManager())
#define GMGetBuiltIn(i) (GMCfg->getInt32(GMConfig_BuiltInOptions::i, GMConfig_BuiltInOptions::BUILTIN_OPTIONS_INVALID))
#define GMSetBuiltIn(i, value) (GMCfg->setInt32(GMConfig_BuiltInOptions::i, value))

// 关于内部（如调试）的一些配置
struct GMConfig_BuiltInOptions
{
	enum
	{
		BUILTIN_OPTIONS_INVALID = -1,
		
		CALCULATE_BSP_FACE = 0,
		POLYGON_LINE_MODE,
		DRAW_ONLY_SKY,
		DRAW_NORMAL, // see DrawNormalOptions

		RUN_PROFILE,

		BUILTIN_OPTIONS_END,
	};

	enum
	{
		DRAW_NORMAL_OFF,
		DRAW_NORMAL_EYESPACE,
		DRAW_NORMAL_CAMERASPACE,
		DRAW_NORMAL_END
	};
};

union GMConfigValueStruct
{
	GMLargeInteger i64 = 0;
	GMint i32;
	GMfloat f32;
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
	GMConfigValue() = default;

	GMConfigValue(const char* str)
	{
		type = VT_String;
		size_t len = strlen(str);
		value.str = new char[len];
		strcpy_s(value.str, len, str);
	}

	GMConfigValue(GMint i)
	{
		type = VT_Int32;
		value.i32 = i;
	}

	GMConfigValue(GMLargeInteger i)
	{
		type = VT_Int64;
		value.i64 = i;
	}

	GMConfigValue(GMfloat f)
	{
		type = VT_Float32;
		value.f32 = f;
	}

	mutable GMConfigValueStruct value;
	GMConfigValueType type;
};

inline bool operator < (const GMConfigValue& lhs, const GMConfigValue& rhs)
{
	if (lhs.type != rhs.type)
		return lhs.type < rhs.type;

	switch (lhs.type)
	{
	case VT_String:
		return strEqual(lhs.value.str, rhs.value.str);
	case VT_Float32:
		return lhs.value.f32 < rhs.value.f32;
	case VT_Int32:
		return lhs.value.i32 < rhs.value.i32;
	case VT_Int64:
	default:
		return lhs.value.i64 < rhs.value.i64;
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
	GMConfig();

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
	void initInternal();
};

END_NS
#endif