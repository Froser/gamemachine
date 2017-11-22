#ifndef __GMENUMS_H__
#define __GMENUMS_H__
#include <gmcommon.h>
BEGIN_NS

enum class GMS_BlendFunc
{
	ZERO = 0,
	ONE,
	SRC_COLOR,
	DST_COLOR,
	SRC_ALPHA,
	DST_ALPHA,
	ONE_MINUS_SRC_ALPHA,
	ONE_MINUS_DST_COLOR,
	ONE_MINUS_DST_ALPHA,
};

enum class GMS_Cull
{
	CULL = 0,
	NONE,
};

enum class GMS_FrontFace
{
	CLOCKWISE,
	COUNTER_CLOCKWISE,
};

enum class GMS_TextureFilter
{
	LINEAR = 0,
	NEAREST,
	LINEAR_MIPMAP_LINEAR,
	NEAREST_MIPMAP_LINEAR,
	LINEAR_MIPMAP_NEAREST,
	NEAREST_MIPMAP_NEAREST,
};

enum class GMS_Wrap
{
	REPEAT = 0,
	CLAMP_TO_EDGE,
	CLAMP_TO_BORDER,
	MIRRORED_REPEAT,
};

enum class GMS_TextureModType
{
	NO_TEXTURE_MOD = 0,
	SCROLL,
	SCALE,
};

END_NS
#endif