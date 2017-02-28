#ifndef __SHADER_H__
#define __SHADER_H__
#include "common.h"
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
};

enum GMS_BlendFunc
{
	GMS_ZERO = 0,
	GMS_ONE,
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
		memset(textures, 0, MAX_ANIMATION_FRAME * sizeof(ITexture*));
	}

	ITexture* textures[MAX_ANIMATION_FRAME];
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
		memset(this, 0, sizeof(this));
		memset(textureFrames, 0, sizeof(textureFrames));
	}

	// 每个texture由TEXTURE_INDEX_MAX个纹理动画组成。静态纹理的纹理动画数量为1
	TextureFrames textureFrames[TEXTURE_INDEX_MAX];
	GMuint autorelease : 1;
};

struct Shader
{
	Shader()
		: surfaceFlag(0)
		, cull(GMS_CULL)
		, blend(false)
		, nodraw(false)
		, nodepthmask(false)
	{
		blendFactors[0] = GMS_ZERO;
		blendFactors[1] = GMS_ZERO;
	}

	TextureInfo texture;
	GMuint surfaceFlag;
	GMS_Cull cull;
	bool blend;
	GMS_BlendFunc blendFactors[2];
	bool nodraw;
	bool nodepthmask;
};
END_NS
#endif