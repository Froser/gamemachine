#ifndef __SHADER_H__
#define __SHADER_H__
#include <gmcommon.h>
#include <gmenums.h>
#include <linearmath.h>
#include <gmtools.h>
#include <gmassets.h>
BEGIN_NS

enum
{
	MAX_ANIMATION_FRAME = 16,
	MAX_TEX_TRANS = 3,
};

struct GMS_TextureTransform
{
	GMS_TextureTransformType type = GMS_TextureTransformType::NoTextureTransform;
	GMfloat p1 = 0;
	GMfloat p2 = 0;
};

struct GMTextureAttributeBank;
GM_PRIVATE_CLASS(GMTextureSampler);
class GM_EXPORT GMTextureSampler
{
	GM_DECLARE_PRIVATE(GMTextureSampler)

public:
	GM_DECLARE_PROPERTY(GMsize_t, FrameCount);
	GM_DECLARE_PROPERTY(GMint32, AnimationMs);
	GM_DECLARE_PROPERTY(GMS_TextureFilter, MagFilter);
	GM_DECLARE_PROPERTY(GMS_TextureFilter, MinFilter);
	GM_DECLARE_PROPERTY(GMS_Wrap, WrapS);
	GM_DECLARE_PROPERTY(GMS_Wrap, WrapT);

public:
	GMTextureSampler();
	GMTextureSampler& operator=(const GMTextureSampler& rhs);

public:
	GMS_TextureTransform& getTextureTransform(GMsize_t index);
	void setTextureTransform(GMsize_t index, const GMS_TextureTransform& mod);
	GMTextureAsset getFrameByIndex(GMsize_t frameIndex);
	GMsize_t addFrame(GMTextureAsset texture);
	bool setTexture(GMsize_t frameIndex, GMTextureAsset texture);
	void applyTexMode(GMfloat timeSeconds, std::function<void(const GMTextureAttributeBank*, GMS_TextureTransformType, Pair<GMfloat, GMfloat>&&)> callback, const GMTextureAttributeBank* bank);
};

enum class GMTextureType
{
	BeginOfEnum,

	Ambient,
	Diffuse,
	Specular,
	NormalMap,
	Lightmap,
	Albedo,
	MetallicRoughnessAO,
	EndOfCommonTexture, // 一般纹理从这里结束

	// Special type
	CubeMap,
	ShadowMap,
	GeometryPasses,
	EndOfEnum,
};

static constexpr GMuint32 GMMaxTextureCount(GMTextureType type)
{
	return type == GMTextureType::BeginOfEnum || type == GMTextureType::EndOfCommonTexture || type == GMTextureType::EndOfEnum ? 0 :
		type == GMTextureType::GeometryPasses ? 8 : 1;
}

template <GMTextureType Type>
struct GMTextureRegisterQuery
{
	enum
	{
		Value = GMMaxTextureCount((GMTextureType)((GMint32)Type - 1)) +
			GMTextureRegisterQuery<(GMTextureType)((GMint32)Type - 1)>::Value
	};
};

template <>
struct GMTextureRegisterQuery<GMTextureType::BeginOfEnum>
{
	enum { Value = 0 };
};

GM_PRIVATE_CLASS(GMTextureList);
class GMTextureList
{
	GM_DECLARE_PRIVATE(GMTextureList)

public:
	GMTextureList();

public:
	GMTextureSampler& getTextureSampler(GMTextureType type);
	const GMTextureSampler& getTextureSampler(GMTextureType type) const;
	GMTextureList& operator=(const GMTextureList& rhs);
};

enum class GMIlluminationModel
{
	Invalid = 0,
	Phong,
	CookTorranceBRDF,
};

// 光照参数
GM_PRIVATE_CLASS(GMMaterial);
class GMMaterial : public IDestroyObject
{
	GM_DECLARE_PRIVATE(GMMaterial)
	GM_DECLARE_PROPERTY(GMfloat, Shininess)
	GM_DECLARE_PROPERTY(GMfloat, Refractivity)
	GM_DECLARE_PROPERTY(GMVec3, Ambient)
	GM_DECLARE_PROPERTY(GMVec3, Specular)
	GM_DECLARE_PROPERTY(GMVec3, Diffuse)
	GM_DECLARE_PROPERTY(GMVec3, F0)

public:
	GMMaterial();
	GMMaterial(const GMMaterial&);
	GMMaterial(GMMaterial&&) GM_NOEXCEPT;
	GMMaterial& operator=(const GMMaterial& rhs);
	GMMaterial& operator=(GMMaterial&& rhs) GM_NOEXCEPT;

public:
	const GMVariant& getCustomMaterial(const GMString& name) const;
	void setCustomMaterial(const GMString& name, const GMVariant& value);
};

GM_PRIVATE_CLASS(GMShader);
class GMShader : public IDestroyObject
{
	GM_DECLARE_PRIVATE(GMShader)
	GM_FRIEND_CLASS(GMGameObject)

public:
	GMShader();
	~GMShader();
	GMShader(const GMShader&);
	GMShader(GMShader&&) GM_NOEXCEPT;
	GMShader& operator=(const GMShader& rhs);
	GMShader& operator=(GMShader&& rhs) GM_NOEXCEPT;

public:
	GM_DECLARE_PROPERTY(GMint32, Flag); // to be deprecated
	GM_DECLARE_PROPERTY(GMS_Cull, Cull);
	GM_DECLARE_PROPERTY(GMS_FrontFace, FrontFace);
	GM_DECLARE_PROPERTY(GMS_BlendFunc, BlendFactorSourceRGB);
	GM_DECLARE_PROPERTY(GMS_BlendFunc, BlendFactorDestRGB);
	GM_DECLARE_PROPERTY(GMS_BlendOp, BlendOpRGB);
	GM_DECLARE_PROPERTY(GMS_BlendFunc, BlendFactorSourceAlpha);
	GM_DECLARE_PROPERTY(GMS_BlendFunc, BlendFactorDestAlpha);
	GM_DECLARE_PROPERTY(GMS_BlendOp, BlendOpAlpha);
	GM_DECLARE_PROPERTY(bool, Blend);
	GM_DECLARE_PROPERTY(bool, Visible);
	GM_DECLARE_PROPERTY(bool, NoDepthTest);
	GM_DECLARE_PROPERTY(GMTextureList, TextureList);
	GM_DECLARE_PROPERTY(GMVec3, LineColor);
	GM_DECLARE_PROPERTY(GMMaterial, Material);
	GM_DECLARE_PROPERTY(GMIlluminationModel, IlluminationModel);
	GM_DECLARE_PROPERTY(GMS_VertexColorOp, VertexColorOp)

public:
	inline void setBlendFactorSource(GMS_BlendFunc func)
	{
		setBlendFactorSourceRGB(func);
		setBlendFactorSourceAlpha(func);
	}

	inline void setBlendFactorDest(GMS_BlendFunc func)
	{
		setBlendFactorDestRGB(func);
		setBlendFactorDestAlpha(func);
	}

	inline void setBlendOp(GMS_BlendOp op)
	{
		setBlendOpRGB(op);
		setBlendOpAlpha(op);
	}

	// GMGameObject:
private:
	void setCulled(bool culled);
	bool isCulled();
};
END_NS
#endif