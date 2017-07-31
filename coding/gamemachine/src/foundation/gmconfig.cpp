#include "stdafx.h"
#include "gmconfig.h"

GMConfig::GMConfig()
{
	initInternal();
}

GMConfig::~GMConfig()
{
	D(d);
	for (auto iter = d->values.begin(); iter != d->values.end(); iter++)
	{
		releaseString(&(*iter).first);
		releaseString(&(*iter).second);
	}
}

void GMConfig::setInt32(const GMConfigValue& key, GMint i)
{
	D(d);
	GMConfigValue* v = find(key);
	if (v)
	{
		releaseString(v);
		v->type = VT_Int32;
		v->value.i32 = i;
	}
	else
	{
		GMConfigValue t;
		t.type = VT_Int32;
		t.value.i32 = i;
		d->values[key] = t;
	}
}

GMint GMConfig::getInt32(const GMConfigValue& key, GMint defaultValue, bool* hasValue, bool* correctType)
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

	return v->value.i32;
}

void GMConfig::setInt64(const GMConfigValue& key, GMLargeInteger i)
{
	D(d);
	GMConfigValue* v = find(key);
	if (v)
	{
		releaseString(v);
		v->type = VT_Int64;
		v->value.i64 = i;
	}
	else
	{
		GMConfigValue t;
		t.type = VT_Int64;
		t.value.i64 = i;
		d->values[key] = t;
	}
}

GMLargeInteger GMConfig::getInt64(const GMConfigValue& key, GMLargeInteger defaultValue, bool* hasValue, bool* correctType)
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

	return v->value.i64;
}

void GMConfig::setFloat32(const GMConfigValue& key, GMfloat i)
{
	D(d);
	GMConfigValue* v = find(key);
	if (v)
	{
		releaseString(v);
		v->type = VT_Float32;
		v->value.f32 = i;
	}
	else
	{
		GMConfigValue t;
		t.type = VT_Float32;
		t.value.f32 = i;
		d->values[key] = t;
	}
}

GMfloat GMConfig::getFloat32(const GMConfigValue& key, GMfloat defaultValue, bool* hasValue, bool* correctType)
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

void GMConfig::setString(const GMConfigValue& key, const char* str)
{
	D(d);
	GMConfigValue* v = find(key);
	size_t len = strlen(str);
	if (v)
	{
		releaseString(v);
		v->type = VT_String;
		v->value.str = new char[len];
		strcpy_s(v->value.str, len, str);
	}
	else
	{
		GMConfigValue t;
		t.type = VT_String;
		t.value.str = new char[len];
		strcpy_s(t.value.str, len, str);
		d->values[key] = t;
	}
}

const char* GMConfig::getString(const GMConfigValue& key, const char* defaultValue, bool* hasValue, bool* correctType)
{
	GMConfigValue* v = find(key);
	if (!v)
	{
		if (hasValue)
			*hasValue = false;
		return defaultValue;
	}

	if (correctType)
		*correctType = v->type == VT_String;

	if (hasValue)
		*hasValue = true;

	return v->value.str;
}

GMConfigValue* GMConfig::find(const GMConfigValue& key)
{
	D(d);
	auto iter = d->values.find(key);
	if (iter == d->values.end())
		return nullptr;
	return &((*iter).second);
}

void GMConfig::releaseString(const GMConfigValue* value)
{
	if (value->type == VT_String && value->value.str)
	{
		delete[] value->value.str;
		value->value.str = nullptr;
	}
}

void GMConfig::initInternal()
{
	setInt32(GMConfig_BuiltInOptions::CALCULATE_BSP_FACE, 1);
	setInt32(GMConfig_BuiltInOptions::POLYGON_LINE_MODE, 0);
	setInt32(GMConfig_BuiltInOptions::DRAW_ONLY_SKY, 0);
	setInt32(GMConfig_BuiltInOptions::DRAW_NORMAL, GMConfig_BuiltInOptions::DRAW_NORMAL_OFF);
	setInt32(GMConfig_BuiltInOptions::DRAW_LIGHTMAP_ONLY, 0);
	setInt32(GMConfig_BuiltInOptions::RUN_PROFILE, 0);
	setInt32(GMConfig_BuiltInOptions::FRAMERATE_CONTROL, 1);
}