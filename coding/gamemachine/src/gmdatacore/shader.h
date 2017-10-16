#ifndef __SHADER_H__
#define __SHADER_H__
#include <gmcommon.h>
#include <linearmath.h>
#include <utilities.h>
BEGIN_NS

enum class GMLightType
{
	AMBIENT = 0,
	SPECULAR,

	// 以上类型数目
	COUNT,
};

GM_PRIVATE_OBJECT(GMLight)
{
	GMLightType type = GMLightType::AMBIENT;
	GMfloat lightPosition[3];
	GMfloat lightColor[3];
};

class GMLight : public GMObject
{
	DECLARE_PRIVATE(GMLight)

public:
	GM_DECLARE_PROPERTY(Type, type, GMLightType);

	GMLight() = default;

	GMLight(const GMLight& light)
	{
		*this = light;
	}

	GMLight(GMLightType type)
	{
		D(d);
		d->type = type;
	}

	inline GMLight& operator=(const GMLight& rhs)
	{
		D(d);
		D_OF(rhs_d, &rhs);
		*d = *rhs_d;
		setLightColor(rhs_d->lightColor);
		setLightPosition(rhs_d->lightPosition);
		return *this;
	}

	void setLightColor(GMfloat light[3])
	{
		D(d);
		d->lightColor[0] = light[0];
		d->lightColor[1] = light[1];
		d->lightColor[2] = light[2];
	}

	const GMfloat* getLightColor() const
	{
		D(d);
		return d->lightColor;
	}

	void setLightPosition(GMfloat light[3])
	{
		D(d);
		d->lightPosition[0] = light[0];
		d->lightPosition[1] = light[1];
		d->lightPosition[2] = light[2];
	}

	const GMfloat* getLightPosition() const
	{
		D(d);
		return d->lightPosition;
	}
};

enum
{
	MAX_ANIMATION_FRAME = 16,
	MAX_TEX_MOD = 3,
};

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

GM_ALIGNED_STRUCT(GMS_TextureMod)
{
	GMS_TextureModType type = GMS_TextureModType::NO_TEXTURE_MOD;
	GMfloat p1 = 0;
	GMfloat p2 = 0;
};

GM_PRIVATE_OBJECT(GMTextureFrames)
{
	ITexture* frames[MAX_ANIMATION_FRAME] = { 0 }; // 每个texture由TEXTURE_INDEX_MAX个纹理动画组成。静态纹理的纹理动画数量为1
	GMS_TextureMod texMod[MAX_TEX_MOD];
	GMint frameCount = 0;
	GMint animationMs = 1; //每一帧动画间隔 (ms)
	GMS_TextureFilter magFilter = GMS_TextureFilter::LINEAR;
	GMS_TextureFilter minFilter = GMS_TextureFilter::LINEAR_MIPMAP_LINEAR;
	GMS_Wrap wrapS = GMS_Wrap::REPEAT;
	GMS_Wrap wrapT = GMS_Wrap::REPEAT;
};

class GMTextureFrames : public GMObject
{
	DECLARE_PRIVATE(GMTextureFrames)

public:
	GM_DECLARE_PROPERTY(FrameCount, frameCount, GMint);
	GM_DECLARE_PROPERTY(AnimationMs, animationMs, GMint);
	GM_DECLARE_PROPERTY(MagFilter, magFilter, GMS_TextureFilter);
	GM_DECLARE_PROPERTY(MinFilter, minFilter, GMS_TextureFilter);
	GM_DECLARE_PROPERTY(WrapS, wrapS, GMS_Wrap);
	GM_DECLARE_PROPERTY(WrapT, wrapT, GMS_Wrap);

public:
	GMTextureFrames() = default;
	GMTextureFrames(const GMTextureFrames&) = delete;

public:
	inline GMS_TextureMod& getTexMod(GMint index)
	{
		D(d);
		return d->texMod[index];
	}

	inline void setTexMod(GMint index, const GMS_TextureMod& mod)
	{
		D(d);
		d->texMod[index] = mod;
	}

	inline ITexture* getFrameByIndex(GMint frameIndex)
	{
		D(d);
		return d->frames[frameIndex];
	}

	inline GMint addFrame(ITexture* oneFrame)
	{
		D(d);
		d->frames[d->frameCount] = oneFrame;
		++d->frameCount;
		return d->frameCount;
	}

	inline GMTextureFrames& operator=(const GMTextureFrames& rhs)
	{
		D(d);
		D_OF(rhs_d, &rhs);
		*d = *rhs.data();
		for (GMint i = 0; i < MAX_ANIMATION_FRAME; i++)
		{
			d->frames[i] = rhs_d->frames[i];
		}
		for (GMint i = 0; i < MAX_TEX_MOD; i++)
		{
			d->texMod[i] = rhs_d->texMod[i];
		}
		return *this;
	}
};

enum class GMTextureType
{
	AMBIENT,
	DIFFUSE,
	NORMALMAP,
	LIGHTMAP,
	END,
};

static constexpr GMint GMMaxTextureCount(GMTextureType type)
{
	return
		type == GMTextureType::AMBIENT ? 3 :
		type == GMTextureType::DIFFUSE ? 3 :
		type == GMTextureType::NORMALMAP ? 1 :
		type == GMTextureType::LIGHTMAP ? 1 : 0;
}

GM_PRIVATE_OBJECT(GMTexture)
{
	GMTextureFrames ambients[GMMaxTextureCount(GMTextureType::AMBIENT)];
	GMTextureFrames diffuse[GMMaxTextureCount(GMTextureType::DIFFUSE)];
	GMTextureFrames normalMap[GMMaxTextureCount(GMTextureType::NORMALMAP)];
	GMTextureFrames lightMap[GMMaxTextureCount(GMTextureType::LIGHTMAP)];
};

class GMTexture : public GMObject
{
	DECLARE_PRIVATE(GMTexture)

public:
	GMTexture() = default;
	GMTexture(const GMTexture& texture) = delete;

public:
	inline GMTextureFrames& getTextureFrames(GMTextureType type, GMint index)
	{
		GM_ASSERT(index < GMMaxTextureCount(type));

		D(d);
		switch (type)
		{
		case GMTextureType::AMBIENT:
			return d->ambients[index];
		case GMTextureType::DIFFUSE:
			return d->diffuse[index];
		case GMTextureType::NORMALMAP:
			return d->normalMap[index];
		case GMTextureType::LIGHTMAP:
			return d->lightMap[index];
		default:
			GM_ASSERT(false);
			return d->ambients[0];
		}
	}

	inline const GMTextureFrames& getTextureFrames(GMTextureType type, GMint index) const
	{
		return const_cast<GMTexture*>(this)->getTextureFrames(type, index);
	}

	inline GMTexture& operator=(const GMTexture& rhs)
	{
		D(d);
		D_OF(rhs_d, &rhs);

		GM_FOREACH_ENUM_CLASS(type, GMTextureType::AMBIENT, GMTextureType::END)
		{
			GMint count = GMMaxTextureCount(type);
			for (GMint i = 0; i < count; i++)
			{
				getTextureFrames(type, i) = rhs.getTextureFrames(type, i);
			}
		}

		return *this;
	}
};

// 光照参数
GM_ALIGNED_STRUCT(GMMaterial)
{
	GMfloat shininess = 0;
	linear_math::Vector3 ka = 1;
	linear_math::Vector3 ks = 0;
	linear_math::Vector3 kd = 0;
};

GM_PRIVATE_OBJECT(Shader)
{
	GMuint surfaceFlag = 0;
	GMS_Cull cull = GMS_Cull::CULL;
	GMS_FrontFace frontFace = GMS_FrontFace::COUNTER_CLOCKWISE;
	GMS_BlendFunc blendFactorSrc = GMS_BlendFunc::ZERO;
	GMS_BlendFunc blendFactorDest = GMS_BlendFunc::ZERO;
	bool blend = false;
	bool nodraw = false;
	bool noDepthTest = false;
	bool drawBorder = false;
	GMfloat lineWidth = 1;
	linear_math::Vector3 lineColor = 0;
	GMTexture texture;
	GMMaterial material;
};

class Shader : public GMObject, public GMAlignmentObject
{
	DECLARE_PRIVATE(Shader)

public:
	Shader() = default;
	Shader(const Shader& shader)
	{
		*this = shader;
	}

public:
	GM_DECLARE_PROPERTY(SurfaceFlag, surfaceFlag, GMuint);
	GM_DECLARE_PROPERTY(Cull, cull, GMS_Cull);
	GM_DECLARE_PROPERTY(FrontFace, frontFace, GMS_FrontFace);
	GM_DECLARE_PROPERTY(BlendFactorSource, blendFactorSrc, GMS_BlendFunc);
	GM_DECLARE_PROPERTY(BlendFactorDest, blendFactorDest, GMS_BlendFunc);
	GM_DECLARE_PROPERTY(Blend, blend, bool);
	GM_DECLARE_PROPERTY(Nodraw, nodraw, bool);
	GM_DECLARE_PROPERTY(NoDepthTest, noDepthTest, bool);
	GM_DECLARE_PROPERTY(Texture, texture, GMTexture);
	GM_DECLARE_PROPERTY(LineWidth, lineWidth, GMfloat);
	GM_DECLARE_PROPERTY(LineColor, lineColor, linear_math::Vector3);
	GM_DECLARE_PROPERTY(DrawBorder, drawBorder, bool);
	GM_DECLARE_PROPERTY(Material, material, GMMaterial);

	inline Shader& operator=(const Shader& rhs)
	{
		D(d);
		*d = *rhs.data();
		return *this;
	}

	inline void stash()
	{
		D(d);
		Shader* s = new Shader();
		*s = *this;
		m_stash.reset(s);
	}

	void pop()
	{
		D(d);
		if (!m_stash)
			return;
		*this = *m_stash;
	}

	AutoPtr<Shader> m_stash;
};
END_NS
#endif