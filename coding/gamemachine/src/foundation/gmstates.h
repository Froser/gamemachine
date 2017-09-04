#ifndef __GMCONFIG_H__
#define __GMCONFIG_H__
#include "foundation/gmobject.h"
BEGIN_NS

#define GMDebugStates (GameMachine::instance().getStatesManager()->getDebugStates())
#define GMGetDebugState(i) (GMDebugStates.getInt32(GMStates_DebugOptions::i))
#define GMSetDebugState(i, value) (GMDebugStates.setInt32(GMStates_DebugOptions::i, value))

#define GMRenderStates (GameMachine::instance().getStatesManager()->getRenderStates())
#define GMGetRenderState(i) (GMRenderStates.getInt32(GMStates_RenderOptions::i))
#define GMSetRenderState(i, value) (GMRenderStates.setInt32(GMStates_RenderOptions::i, value))
#define GMGetRenderStateF(i) (GMRenderStates.getFloat32(GMStates_RenderOptions::i))
#define GMSetRenderStateF(i, value) (GMRenderStates.setFloat32(GMStates_RenderOptions::i, value))

constexpr GMint MAX_SUPPORTED_KEY_NUM = 64;

// 关于内部（如调试）的一些配置
struct GMStates_DebugOptions
{
	enum
	{
		BUILTIN_OPTIONS_INVALID = -1,
		
		CALCULATE_BSP_FACE = 0,
		POLYGON_LINE_MODE,
		DRAW_ONLY_SKY,
		DRAW_NORMAL,
		DRAW_LIGHTMAP_ONLY,
		RUN_PROFILE,
		FRAMERATE_CONTROL, //把最大帧率控制在60FPS

		FRAMEBUFFER_VIEWER_X,
		FRAMEBUFFER_VIEWER_Y,
		FRAMEBUFFER_VIEWER_HEIGHT,
		FRAMEBUFFER_VIEWER_WIDTH,
		FRAMEBUFFER_VIEWER_INDEX,

		OPTIONS_END,
	};

	enum // DRAW_NORMAL
	{
		DRAW_NORMAL_OFF,
		DRAW_NORMAL_EYESPACE,
		DRAW_NORMAL_CAMERASPACE,
		DRAW_NORMAL_END
	};
};

struct GMStates_RenderOptions
{
	enum
	{
		EFFECTS,
		BLUR_SAMPLE_OFFSET_X,
		BLUR_SAMPLE_OFFSET_Y,
		RESOLUTION_X,
		RESOLUTION_Y,

		OPTIONS_END
	};

	enum
	{
		AUTO_SAMPLE_OFFSET = -1,
		AUTO_RESOLUTION = -1,
	};

	struct GMEffects_ns
	{
		enum GMEffects
		{
			None = 0x00000000,
			Inversion = 0x00000001,
			Sharpen = 0x00000002,
			Blur = 0x00000004,
			Grayscale = 0x00000008,
			EdgeDetect = 0x00000010,

			LastEffectsTag,
			EndOfEffects = (LastEffectsTag - 1) << 2
		};
	};
};
using GMEffects = GMStates_RenderOptions::GMEffects_ns::GMEffects;

union GMStateItemValueStruct
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

struct GMStateItemValue : public GMAlignmentObject
{
	GMStateItemValue() = default;

	GMStateItemValue(const char* str)
	{
		type = VT_String;
		size_t len = strlen(str);
		value.str = new char[len];
		strcpy_s(value.str, len, str);
	}

	GMStateItemValue(GMint i)
	{
		type = VT_Int32;
		value.i32 = i;
	}

	GMStateItemValue(GMLargeInteger i)
	{
		type = VT_Int64;
		value.i64 = i;
	}

	GMStateItemValue(GMfloat f)
	{
		type = VT_Float32;
		value.f32 = f;
	}

	mutable GMStateItemValueStruct value;
	GMConfigValueType type;
};

inline bool operator < (const GMStateItemValue& lhs, const GMStateItemValue& rhs)
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

GM_PRIVATE_OBJECT(GMStateItem)
{
	Array<GMStateItemValue, MAX_SUPPORTED_KEY_NUM> values;
};

class GMStateItem : public GMObject
{
	DECLARE_PRIVATE(GMStateItem)

	friend class GameMachine;

public:
	~GMStateItem();

public:
	void setInt32(GMint key, GMint i);
	GMint getInt32(GMint key);
	void setInt64(GMint key, GMLargeInteger i);
	GMLargeInteger getInt64(GMint key);
	void setFloat32(GMint key, GMfloat i);
	GMfloat getFloat32(GMint key);
	void setString(GMint key, const char* str);
	const char* getString(GMint key);

private:
	void releaseString(const GMStateItemValue& value);
};

GM_PRIVATE_OBJECT(GMStates)
{
	GMStateItem debugStates;
	GMStateItem renderStates;
};

class GMStates : public GMObject
{
	DECLARE_PRIVATE(GMStates)

public:
	GMStates();

public:
	GMStateItem& getDebugStates() { D(d); return d->debugStates; }
	GMStateItem& getRenderStates() { D(d); return d->renderStates; }

private:
	void initInternal();
};

END_NS
#endif