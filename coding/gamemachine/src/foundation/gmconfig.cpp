#include "stdafx.h"
#include "gmconfig.h"

GMConfig::GMConfig()
{
	initInternal();
}

GMConfig::~GMConfig()
{
	D(d);
	for (auto& v : d->values)
	{
		releaseString(v);
	}
}

void GMConfig::setInt32(GMint key, GMint i)
{
	D(d);
	GMConfigValue& v = d->values[key];
	releaseString(v);
	GMConfigValue t;
	t.type = VT_Int32;
	t.value.i32 = i;
	d->values[key] = t;
}

GMint GMConfig::getInt32(GMint key, bool* correctType)
{
	D(d);
	GMConfigValue& v = d->values[key];

	if (correctType)
		*correctType = v.type == VT_Int32;

	return v.value.i32;
}

void GMConfig::setInt64(GMint key, GMLargeInteger i)
{
	D(d);
	GMConfigValue& v = d->values[key];
	releaseString(v);
	GMConfigValue t;
	t.type = VT_Int64;
	t.value.i64 = i;
	d->values[key] = t;
}

GMLargeInteger GMConfig::getInt64(GMint key, bool* correctType)
{
	D(d);
	GMConfigValue& v = d->values[key];
	if (correctType)
		*correctType = v.type == VT_Int64;
	return v.value.i64;
}

void GMConfig::setFloat32(GMint key, GMfloat i)
{
	D(d);
	GMConfigValue& v = d->values[key];
	releaseString(v);
	GMConfigValue t;
	t.type = VT_Float32;
	t.value.f32 = i;
	d->values[key] = t;
}

GMfloat GMConfig::getFloat32(GMint key, bool* correctType)
{
	D(d);
	GMConfigValue& v = d->values[key];
	if (correctType)
		*correctType = v.type == VT_Float32;
	return v.value.f32;
}

void GMConfig::setString(GMint key, const char* str)
{
	D(d);
	GMConfigValue& v = d->values[key];
	size_t len = strlen(str);
	releaseString(v);
	GMConfigValue t;
	t.type = VT_String;
	t.value.str = new char[len];
	strcpy_s(t.value.str, len, str);
	d->values[key] = t;
}

const char* GMConfig::getString(GMint key, bool* correctType)
{
	D(d);
	GMConfigValue& v = d->values[key];
	if (correctType)
		*correctType = v.type == VT_String;
	return v.value.str;
}

void GMConfig::releaseString(const GMConfigValue& value)
{
	if (value.type == VT_String && value.value.str)
	{
		delete[] value.value.str;
		value.value.str = nullptr;
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
	setInt32(GMConfig_BuiltInOptions::FRAMEBUFFER_VIEWER_X, 10);
	setInt32(GMConfig_BuiltInOptions::FRAMEBUFFER_VIEWER_Y, 10);
	setInt32(GMConfig_BuiltInOptions::FRAMEBUFFER_VIEWER_WIDTH, 250);
	setInt32(GMConfig_BuiltInOptions::FRAMEBUFFER_VIEWER_HEIGHT, 250);
	setInt32(GMConfig_BuiltInOptions::FRAMEBUFFER_VIEWER_INDEX, 0);
}