#ifndef __GMCONFIG_H__
#define __GMCONFIG_H__
#include "foundation/gmobject.h"
#include <map>
#include <string>
BEGIN_NS

union GMConfigValueStruct
{
	GMLargeInteger int64;
	GMint int32;
	GMfloat f32;
	bool boolean;
	void* ptr;
};

enum GMConfigValueType
{
	VT_Float32,
	VT_Int32,
	VT_Int64,
};

struct GMConfigValue
{
	GMConfigValueType type;
	GMConfigValueStruct value;
};

GM_PRIVATE_OBJECT(GMConfig)
{
	std::map<std::string, GMConfigValue> values;
};

class GMConfig : public GMObject
{
	DECLARE_PRIVATE(GMConfig)

	friend class GameMachine;

private:
	GMConfig() {}

public:
	void setInt32(GMint i)
	{
		GMConfigValue t;
		t.type = VT_Int32;
		t.value.int32 = i;
	}

	GMint getInt32(const char* key, GMint defaultValue, bool* hasValue = nullptr , bool* correctType = nullptr)
	{
		GMConfigValue* v = find(key);
		if (!v)
		{
			if (hasValue)
				*hasValue = false;
			return defaultValue;
		}

		if (correctType)
			*correctType = v->type == VT_Int32;

		if (hasValue)
			*hasValue = true;

		return v->value.int32;
	}

	void setInt64(GMLargeInteger i)
	{
		GMConfigValue t;
		t.type = VT_Int64;
		t.value.int64 = i;
	}

	GMLargeInteger getInt64(const char* key, GMLargeInteger defaultValue, bool* hasValue = nullptr, bool* correctType = nullptr)
	{
		GMConfigValue* v = find(key);
		if (!v)
		{
			if (hasValue)
				*hasValue = false;
			return defaultValue;
		}

		if (correctType)
			*correctType = v->type == VT_Int64;

		if (hasValue)
			*hasValue = true;

		return v->value.int64;
	}

	void setFloat32(GMfloat i)
	{
		GMConfigValue t;
		t.type = VT_Float32;
		t.value.f32 = i;
	}

	GMfloat getFloat32(const char* key, GMfloat defaultValue, bool* hasValue = nullptr, bool* correctType = nullptr)
	{
		GMConfigValue* v = find(key);
		if (!v)
		{
			if (hasValue)
				*hasValue = false;
			return defaultValue;
		}

		if (correctType)
			*correctType = v->type == VT_Float32;

		if (hasValue)
			*hasValue = true;

		return v->value.f32;
	}

private:
	GMConfigValue* find(const std::string& key)
	{
		D(d);
		auto iter = d->values.find(key);
		if (iter == d->values.end())
			return nullptr;
		return &((*iter).second);
	}
};

END_NS
#endif