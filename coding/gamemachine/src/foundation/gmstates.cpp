#include "stdafx.h"
#include "gmstates.h"
#include "assert.h"

GMStateItem::~GMStateItem()
{
	D(d);
	for (auto& v : d->values)
	{
		releaseString(v);
	}
}

void GMStateItem::setInt32(GMint key, GMint i)
{
	D(d);
	GMStateItemValue& v = d->values[key];
	releaseString(v);
	GMStateItemValue t;
	t.type = VT_Int32;
	t.value.i32 = i;
	d->values[key] = t;
}

GMint GMStateItem::getInt32(GMint key)
{
	D(d);
	GMStateItemValue& v = d->values[key];

	ASSERT(v.type == VT_Int32);

	return v.value.i32;
}

void GMStateItem::setInt64(GMint key, GMLargeInteger i)
{
	D(d);
	GMStateItemValue& v = d->values[key];
	releaseString(v);
	GMStateItemValue t;
	t.type = VT_Int64;
	t.value.i64 = i;
	d->values[key] = t;
}

GMLargeInteger GMStateItem::getInt64(GMint key)
{
	D(d);
	GMStateItemValue& v = d->values[key];
	ASSERT(v.type == VT_Int64);
	return v.value.i64;
}

void GMStateItem::setFloat32(GMint key, GMfloat i)
{
	D(d);
	GMStateItemValue& v = d->values[key];
	releaseString(v);
	GMStateItemValue t;
	t.type = VT_Float32;
	t.value.f32 = i;
	d->values[key] = t;
}

GMfloat GMStateItem::getFloat32(GMint key)
{
	D(d);
	GMStateItemValue& v = d->values[key];
	ASSERT(v.type == VT_Float32);
	return v.value.f32;
}

void GMStateItem::setString(GMint key, const char* str)
{
	D(d);
	GMStateItemValue& v = d->values[key];
	size_t len = strlen(str);
	releaseString(v);
	GMStateItemValue t;
	t.type = VT_String;
	t.value.str = new char[len];
	strcpy_s(t.value.str, len, str);
	d->values[key] = t;
}

const char* GMStateItem::getString(GMint key)
{
	D(d);
	GMStateItemValue& v = d->values[key];
	ASSERT(v.type == VT_String);
	return v.value.str;
}

void GMStateItem::releaseString(const GMStateItemValue& value)
{
	if (value.type == VT_String && value.value.str)
	{
		delete[] value.value.str;
		value.value.str = nullptr;
	}
}

GMStates::GMStates()
{
	initInternal();
}

void GMStates::initInternal()
{
	D(d);
	d->debugStates.setInt32(GMStates_DebugOptions::CALCULATE_BSP_FACE, 1);
	d->debugStates.setInt32(GMStates_DebugOptions::POLYGON_LINE_MODE, 0);
	d->debugStates.setInt32(GMStates_DebugOptions::DRAW_ONLY_SKY, 0);
	d->debugStates.setInt32(GMStates_DebugOptions::DRAW_NORMAL, GMStates_DebugOptions::DRAW_NORMAL_OFF);
	d->debugStates.setInt32(GMStates_DebugOptions::DRAW_LIGHTMAP_ONLY, 0);
	d->debugStates.setInt32(GMStates_DebugOptions::RUN_PROFILE, 0);
	d->debugStates.setInt32(GMStates_DebugOptions::FRAMERATE_CONTROL, 1);
	d->debugStates.setInt32(GMStates_DebugOptions::FRAMEBUFFER_VIEWER_X, 10);
	d->debugStates.setInt32(GMStates_DebugOptions::FRAMEBUFFER_VIEWER_Y, 10);
	d->debugStates.setInt32(GMStates_DebugOptions::FRAMEBUFFER_VIEWER_WIDTH, 250);
	d->debugStates.setInt32(GMStates_DebugOptions::FRAMEBUFFER_VIEWER_HEIGHT, 250);
	d->debugStates.setInt32(GMStates_DebugOptions::FRAMEBUFFER_VIEWER_INDEX, 0);

	d->renderStates.setInt32(GMStates_RenderOptions::EFFECTS, GMEffects::None);
	d->renderStates.setFloat32(GMStates_RenderOptions::BLUR_SAMPLE_OFFSET_X, GMStates_RenderOptions::AUTO_SAMPLE_OFFSET);
	d->renderStates.setFloat32(GMStates_RenderOptions::BLUR_SAMPLE_OFFSET_Y, GMStates_RenderOptions::AUTO_SAMPLE_OFFSET);
	d->renderStates.setInt32(GMStates_RenderOptions::RESOLUTION_X, GMStates_RenderOptions::AUTO_RESOLUTION);
	d->renderStates.setInt32(GMStates_RenderOptions::RESOLUTION_Y, GMStates_RenderOptions::AUTO_RESOLUTION);
}
