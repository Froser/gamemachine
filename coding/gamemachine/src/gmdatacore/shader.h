#ifndef __SHADER_H__
#define __SHADER_H__
#include "common.h"
#include "foundation/linearmath.h"
BEGIN_NS

struct ITexture;
// 表示一套纹理，包括普通纹理、漫反射纹理、法线贴图、光照贴图，以后可能还有高光贴图等
enum TextureIndex
{
	TEXTURE_INDEX_AMBIENT,
	TEXTURE_INDEX_AMBIENT_2,
	TEXTURE_INDEX_AMBIENT_3,
	TEXTURE_INDEX_DIFFUSE,
	TEXTURE_INDEX_NORMAL_MAPPING,
	TEXTURE_INDEX_LIGHTMAP,
	TEXTURE_INDEX_MAX,
};

enum
{
	MAX_ANIMATION_FRAME = 16,
	MAX_TEX_MOD = 3,
};

enum GMS_BlendFunc
{
	GMS_ZERO = 0,
	GMS_ONE,
	GMS_DST_COLOR,
	GMS_SRC_ALPHA,
	GMS_ONE_MINUS_SRC_ALPHA,
};

enum GMS_Cull
{
	GMS_CULL = 0,
	GMS_NONE,
};

enum GMS_TextureFilter
{
	GMS_LINEAR = 0,
	GMS_NEAREST,
	GMS_LINEAR_MIPMAP_LINEAR,
	GMS_NEAREST_MIPMAP_LINEAR,
	GMS_LINEAR_MIPMAP_NEAREST,
	GMS_NEAREST_MIPMAP_NEAREST,
};

enum GMS_Wrap
{
	GMS_REPEAT = 0,
	GMS_CLAMP_TO_EDGE,
	GMS_CLAMP_TO_BORDER,
	GMS_MIRRORED_REPEAT,
};

enum GMS_TextureModType
{
	GMS_NO_TEXTURE_MOD = 0,
	GMS_SCROLL,
	GMS_SCALE,
};

struct GMS_TextureMod
{
	GMS_TextureModType type;
	GMfloat p1;
	GMfloat p2;
};

// 表示一系列动画帧
struct TextureFrames
{
	TextureFrames()
		: frameCount(0)
		, animationMs(1)
		, magFilter(GMS_LINEAR)
		, minFilter(GMS_LINEAR_MIPMAP_LINEAR)
		, wrapS(GMS_REPEAT)
		, wrapT(GMS_REPEAT)
	{
		memset(frames, 0, MAX_ANIMATION_FRAME * sizeof(ITexture*));
		memset(texMod, 0, MAX_TEX_MOD * sizeof(GMS_TextureMod));
	}

	ITexture* frames[MAX_ANIMATION_FRAME];
	GMS_TextureMod texMod[MAX_TEX_MOD];
	GMint frameCount;
	GMint animationMs; //每一帧动画间隔 (ms)
	GMS_TextureFilter magFilter;
	GMS_TextureFilter minFilter;
	GMS_Wrap wrapS;
	GMS_Wrap wrapT;
};

struct TextureInfo
{
	TextureInfo()
	{
		autorelease = 0;
		memset(textures, 0, sizeof(textures));
	}

	// 每个texture由TEXTURE_INDEX_MAX个纹理动画组成。静态纹理的纹理动画数量为1
	TextureFrames textures[TEXTURE_INDEX_MAX];
	GMuint autorelease : 1;
};

#define ARG_OFFSET(arg, size) arg+size
enum LightArgs
{
	LA_KA = 0,
	LA_KD = LA_KA,
	LA_KS = ARG_OFFSET(LA_KD, 3),
	LA_SHINESS = ARG_OFFSET(LA_KS, 3),
	LA_END,
};

struct LightInfo
{
	bool on;
	bool useGlobalLightColor; // true表示使用全局的光的颜色
	linear_math::Vector3 lightPosition;
	linear_math::Vector3 lightColor;
	GMfloat args[LA_END];
};

enum LightType
{
	LT_BEGIN = 0,
	LT_AMBIENT = 0,
	LT_SPECULAR,
	LT_END,
};

struct Shader
{
	Shader()
		: surfaceFlag(0)
		, cull(GMS_CULL)
		, blend(false)
		, nodraw(false)
		, noDepthTest(false)
	{
		blendFactors[0] = GMS_ZERO;
		blendFactors[1] = GMS_ZERO;
		memset(&lights, 0, sizeof(lights));
	}

	TextureInfo texture;
	GMuint surfaceFlag;
	GMS_Cull cull;
	bool blend;
	GMS_BlendFunc blendFactors[2];
	bool nodraw;
	bool noDepthTest;
	LightInfo lights[LT_END];
};
END_NS
#endif