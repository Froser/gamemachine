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

GM_ALIGNED_STRUCT(GMS_TextureTransform)
{
	GMS_TextureTransformType type = GMS_TextureTransformType::NoTextureTransform;
	GMfloat p1 = 0;
	GMfloat p2 = 0;
};

GM_PRIVATE_OBJECT(GMTextureSampler)
{
	Array<GMTextureAsset, MAX_ANIMATION_FRAME> frames; // 每个texture由TEXTURE_INDEX_MAX个纹理动画组成。静态纹理的纹理动画数量为1
	Array<GMS_TextureTransform, MAX_TEX_TRANS> texTransform;
	GMsize_t frameCount = 0;
	GMint32 animationMs = 1; //每一帧动画间隔 (ms)
	GMS_TextureFilter magFilter = GMS_TextureFilter::Linear;
	GMS_TextureFilter minFilter = GMS_TextureFilter::LinearMipmapLinear;
	GMS_Wrap wrapS = GMS_Wrap::Repeat;
	GMS_Wrap wrapT = GMS_Wrap::Repeat;
};

class GMTextureSampler
{
	GM_DECLARE_PRIVATE_NGO(GMTextureSampler)

public:
	GM_DECLARE_PROPERTY(FrameCount, frameCount);
	GM_DECLARE_PROPERTY(AnimationMs, animationMs);
	GM_DECLARE_PROPERTY(MagFilter, magFilter);
	GM_DECLARE_PROPERTY(MinFilter, minFilter);
	GM_DECLARE_PROPERTY(WrapS, wrapS);
	GM_DECLARE_PROPERTY(WrapT, wrapT);

public:
	GMTextureSampler() = default;

public:
	GMS_TextureTransform& getTextureTransform(GMsize_t index);
	void setTextureTransform(GMsize_t index, const GMS_TextureTransform& mod);
	GMTextureAsset getFrameByIndex(GMsize_t frameIndex);
	GMsize_t addFrame(GMTextureAsset texture);
	bool setTexture(GMsize_t frameIndex, GMTextureAsset texture);
	void applyTexMode(GMfloat timeSeconds, std::function<void(GMS_TextureTransformType, Pair<GMfloat, GMfloat>&&)> callback);
	GMTextureSampler& operator=(const GMTextureSampler& rhs);
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

GM_PRIVATE_OBJECT(GMTextureList)
{
	GMTextureSampler ambientMap;
	GMTextureSampler diffuseMap;
	GMTextureSampler normalMap;
	GMTextureSampler lightMap;
	GMTextureSampler specularMap;
	GMTextureSampler albedoMap;
	GMTextureSampler metallicRoughnessAOMap;
	GMTextureSampler cubeMap;
};

GM_ALIGNED_16(class) GMTextureList
{
	GM_DECLARE_PRIVATE_NGO(GMTextureList)
	GM_DECLARE_ALIGNED_ALLOCATOR()

public:
	GMTextureList() = default;

public:
	inline GMTextureSampler& getTextureSampler(GMTextureType type)
	{
		D(d);
		switch (type)
		{
		case GMTextureType::Ambient:
			return d->ambientMap;
		case GMTextureType::Diffuse:
			return d->diffuseMap;
		case GMTextureType::NormalMap:
			return d->normalMap;
		case GMTextureType::Lightmap:
			return d->lightMap;
		case GMTextureType::Specular:
			return d->specularMap;
		case GMTextureType::Albedo:
			return d->albedoMap;
		case GMTextureType::MetallicRoughnessAO:
			return d->metallicRoughnessAOMap;
		case GMTextureType::CubeMap:
			return d->cubeMap;
		default:
			GM_ASSERT(false);
			return d->ambientMap;
		}
	}

	inline const GMTextureSampler& getTextureSampler(GMTextureType type) const
	{
		return const_cast<GMTextureList*>(this)->getTextureSampler(type);
	}

	inline GMTextureList& operator=(const GMTextureList& rhs)
	{
		D(d);
		D_OF(rhs_d, &rhs);

		GM_FOREACH_ENUM_CLASS(type, GMTextureType::Ambient, GMTextureType::EndOfCommonTexture)
		{
			getTextureSampler(type) = rhs.getTextureSampler(type);
		}

		return *this;
	}
};

enum class GMIlluminationModel
{
	Invalid = 0,
	Phong,
	CookTorranceBRDF,
};

// 光照参数
GM_PRIVATE_OBJECT(GMMaterial)
{
	GMfloat shininess = 1;
	GMfloat refractivity = 0;
	GMVec3 ambient = GMVec3(0);
	GMVec3 specular = GMVec3(1);
	GMVec3 diffuse = GMVec3(1);
	GMVec3 f0 = GMVec3(0.04f); //!< 基础反射率，用于PBR(BRDF)模型的渲染。对于金属，这个值为0.04。
	HashMap<GMString, GMVariant, GMStringHashFunctor> customMaterials;
};

GM_ALIGNED_16(class) GMMaterial
{
	GM_DECLARE_PRIVATE_NGO(GMMaterial)
	GM_DECLARE_ALIGNED_ALLOCATOR()
	GM_DECLARE_PROPERTY(Shininess, shininess)
	GM_DECLARE_PROPERTY(Refractivity, refractivity)
	GM_DECLARE_PROPERTY(Ambient, ambient)
	GM_DECLARE_PROPERTY(Specular, specular)
	GM_DECLARE_PROPERTY(Diffuse, diffuse)
	GM_DECLARE_PROPERTY(F0, f0)

public:
	const GMVariant& getCustomMaterial(const GMString& name) const;
	void setCustomMaterial(const GMString& name, const GMVariant& value);
};

GM_PRIVATE_OBJECT(GMShader)
{
	GMIlluminationModel illuminationModel = GMIlluminationModel::Phong;
	GMuint32 surfaceFlag = 0;
	GMS_Cull cull = GMS_Cull::Cull;
	GMS_FrontFace frontFace = GMS_FrontFace::Closewise;
	GMS_BlendFunc blendFactorSrcRGB = GMS_BlendFunc::Zero;
	GMS_BlendFunc blendFactorDestRGB = GMS_BlendFunc::Zero;
	GMS_BlendFunc blendFactorSrcAlpha = GMS_BlendFunc::Zero;
	GMS_BlendFunc blendFactorDestAlpha = GMS_BlendFunc::Zero;
	GMS_BlendOp blendOpRGB = GMS_BlendOp::Add;
	GMS_BlendOp blendOpAlpha = GMS_BlendOp::Add;
	GMS_VertexColorOp vertexColorOp = GMS_VertexColorOp::DoNotUseVertexColor;
	bool blend = false;
	bool discard = false;
	bool noDepthTest = false;
	bool drawBorder = false;
	GMVec3 lineColor = GMVec3(0);
	GMTextureList textureList;
	GMMaterial material;
};

GM_ALIGNED_16(class) GMShader : public IDestroyObject
{
	GM_DECLARE_PRIVATE_NGO(GMShader)
	GM_DECLARE_ALIGNED_ALLOCATOR()
	GM_DEFAULT_DESTORY_IMPL()

public:
	GMShader() = default;

public:
	GM_DECLARE_PROPERTY(SurfaceFlag, surfaceFlag);
	GM_DECLARE_PROPERTY(Cull, cull);
	GM_DECLARE_PROPERTY(FrontFace, frontFace);
	GM_DECLARE_PROPERTY(BlendFactorSourceRGB, blendFactorSrcRGB);
	GM_DECLARE_PROPERTY(BlendFactorDestRGB, blendFactorDestRGB);
	GM_DECLARE_PROPERTY(BlendOpRGB, blendOpRGB);
	GM_DECLARE_PROPERTY(BlendFactorSourceAlpha, blendFactorSrcAlpha);
	GM_DECLARE_PROPERTY(BlendFactorDestAlpha, blendFactorDestAlpha);
	GM_DECLARE_PROPERTY(BlendOpAlpha, blendOpAlpha);
	GM_DECLARE_PROPERTY(Blend, blend);
	GM_DECLARE_PROPERTY(Discard, discard);
	GM_DECLARE_PROPERTY(NoDepthTest, noDepthTest);
	GM_DECLARE_PROPERTY(TextureList, textureList);
	GM_DECLARE_PROPERTY(LineColor, lineColor);
	GM_DECLARE_PROPERTY(Material, material);
	GM_DECLARE_PROPERTY(IlluminationModel, illuminationModel);
	GM_DECLARE_PROPERTY(VertexColorOp, vertexColorOp)

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
};
END_NS
#endif