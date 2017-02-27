#ifndef __SHADER_H__
#define __SHADER_H__
#include "common.h"
BEGIN_NS

struct ITexture;
// 表示一套纹理，包括普通纹理、漫反射纹理、法线贴图、光照贴图，以后可能还有高光贴图等
enum TextureIndex
{
	TEXTURE_INDEX_AMBIENT,
	TEXTURE_INDEX_DIFFUSE,
	TEXTURE_INDEX_NORMAL_MAPPING,
	TEXTURE_INDEX_LIGHTMAP,

	TEXTURE_INDEX_MAX,
};

enum
{
	MAX_ANIMATION_FRAME = 16,
};

struct TextureInfo
{
	ITexture* texture[TEXTURE_INDEX_MAX];
	GMuint autorelease : 1;
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

struct Shader
{
	Shader()
		: surfaceFlag(0)
		, cull(GMS_CULL)
		, animationMs(1)
	{
		blendFactors[0] = GMS_ZERO;
		blendFactors[1] = GMS_ZERO;
		memset(textures, 0, sizeof(textures));
		memset(this, 0, sizeof(this));
	}

	bool blend;
	GMS_BlendFunc blendFactors[2];
	TextureInfo textures[MAX_ANIMATION_FRAME]; // 表示一个动画帧
	GMint animationMs; //每一帧动画间隔 (ms)
	GMuint frameCount;
	GMuint surfaceFlag;
	GMS_Cull cull;
};
END_NS
#endif