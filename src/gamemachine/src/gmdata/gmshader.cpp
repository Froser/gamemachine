#include "stdafx.h"
#include "gmshader.h"

BEGIN_NS

GM_PRIVATE_OBJECT_ALIGNED(GMMaterial)
{
	GMfloat shininess = 1;
	GMfloat refractivity = 0;
	GMVec3 ambient = GMVec3(0);
	GMVec3 specular = GMVec3(1);
	GMVec3 diffuse = GMVec3(1);
	GMVec3 f0 = GMVec3(0.04f); //!< 基础反射率，用于PBR(BRDF)模型的渲染。对于金属，这个值为0.04。
	HashMap<GMString, GMVariant, GMStringHashFunctor> customMaterials;
};

GM_DEFINE_PROPERTY(GMMaterial, GMfloat, Shininess, shininess)
GM_DEFINE_PROPERTY(GMMaterial, GMfloat, Refractivity, refractivity)
GM_DEFINE_PROPERTY(GMMaterial, GMVec3, Ambient, ambient)
GM_DEFINE_PROPERTY(GMMaterial, GMVec3, Specular, specular)
GM_DEFINE_PROPERTY(GMMaterial, GMVec3, Diffuse, diffuse)
GM_DEFINE_PROPERTY(GMMaterial, GMVec3, F0, f0)
GMMaterial::GMMaterial()
{
	GM_CREATE_DATA();
}

GMMaterial::GMMaterial(const GMMaterial& rhs)
{
	*this = rhs;
}

GMMaterial::GMMaterial(GMMaterial&& rhs) GM_NOEXCEPT
{
	*this = std::move(rhs);
}

GMMaterial& GMMaterial::operator=(const GMMaterial& rhs)
{
	GM_COPY(rhs);
	return *this;
}

GMMaterial& GMMaterial::operator=(GMMaterial&& rhs) GM_NOEXCEPT
{
	GM_MOVE(rhs);
	return *this;
}

const GMVariant& GMMaterial::getCustomMaterial(const GMString& name) const
{
	D(d);
	return d->customMaterials[name];
}

void GMMaterial::setCustomMaterial(const GMString& name, const GMVariant& value)
{
	D(d);
	d->customMaterials[name] = value;
}

GM_PRIVATE_OBJECT_UNALIGNED(GMTextureSampler)
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

GM_DEFINE_PROPERTY(GMTextureSampler, GMsize_t, FrameCount, frameCount);
GM_DEFINE_PROPERTY(GMTextureSampler, GMint32, AnimationMs, animationMs);
GM_DEFINE_PROPERTY(GMTextureSampler, GMS_TextureFilter, MagFilter, magFilter);
GM_DEFINE_PROPERTY(GMTextureSampler, GMS_TextureFilter, MinFilter, minFilter);
GM_DEFINE_PROPERTY(GMTextureSampler, GMS_Wrap, WrapS, wrapS);
GM_DEFINE_PROPERTY(GMTextureSampler, GMS_Wrap, WrapT, wrapT);

GMTextureSampler::GMTextureSampler()
{

}

GMS_TextureTransform& GMTextureSampler::getTextureTransform(GMsize_t index)
{
	D(d);
	return d->texTransform[index];
}

void GMTextureSampler::setTextureTransform(GMsize_t index, const GMS_TextureTransform& trans)
{
	D(d);
	d->texTransform[index] = trans;
}

GMTextureAsset GMTextureSampler::getFrameByIndex(GMsize_t frameIndex)
{
	D(d);
	GM_ASSERT(!d->frames[frameIndex].isEmpty());
	GM_ASSERT(d->frames[frameIndex].getType() == GMAssetType::Texture);
	return d->frames[frameIndex];
}

GMsize_t GMTextureSampler::addFrame(GMTextureAsset texture)
{
	D(d);
	GM_ASSERT(texture.getType() == GMAssetType::Texture);
	d->frames[d->frameCount] = texture;
	++d->frameCount;
	return d->frameCount;
}

bool GMTextureSampler::setTexture(GMsize_t frameIndex, GMTextureAsset texture)
{
	D(d);
	GM_ASSERT(texture.getType() == GMAssetType::Texture);
	if (frameIndex >= d->frameCount)
		return false;

	d->frames[frameIndex] = texture;
	return true;
}

GMTextureSampler& GMTextureSampler::operator=(const GMTextureSampler& rhs)
{
	D(d);
	D_OF(rhs_d, &rhs);
	*d = *rhs.data();
	for (GMint32 i = 0; i < MAX_ANIMATION_FRAME; i++)
	{
		d->frames[i] = rhs_d->frames[i];
	}
	for (GMint32 i = 0; i < MAX_TEX_TRANS; i++)
	{
		d->texTransform[i] = rhs_d->texTransform[i];
	}
	return *this;
}

void GMTextureSampler::applyTexMode(GMfloat timeSeconds, std::function<void(const GMTextureAttributeBank*, GMS_TextureTransformType, Pair<GMfloat, GMfloat>&&)> callback, const GMTextureAttributeBank* bank)
{
	GMuint32 n = 0;
	const GMS_TextureTransform& tc = getTextureTransform(n);
	while (n < MAX_TEX_TRANS && tc.type != GMS_TextureTransformType::NoTextureTransform)
	{
		switch (tc.type)
		{
		case GMS_TextureTransformType::Scroll:
		{
			GMfloat s = timeSeconds * tc.p1, t = timeSeconds * tc.p2;
			callback(bank, tc.type, { s, t });
		}
		break;
		case GMS_TextureTransformType::Scale:
		{
			GMfloat s = tc.p1, t = tc.p2;
			callback(bank, tc.type, { s, t });
			break;
		}
		default:
			break;
		}
		n++;
	}
}

GM_PRIVATE_OBJECT_UNALIGNED(GMTextureList)
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

GMTextureList::GMTextureList()
{
	GM_CREATE_DATA();
}

GMTextureList& GMTextureList::operator=(const GMTextureList& rhs)
{
	D(d);
	D_OF(rhs_d, &rhs);

	GM_FOREACH_ENUM_CLASS(type, GMTextureType::Ambient, GMTextureType::EndOfCommonTexture)
	{
		getTextureSampler(type) = rhs.getTextureSampler(type);
	}

	return *this;
}

const GMTextureSampler& GMTextureList::getTextureSampler(GMTextureType type) const
{
	return const_cast<GMTextureList*>(this)->getTextureSampler(type);
}

GMTextureSampler& GMTextureList::getTextureSampler(GMTextureType type)
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

GM_PRIVATE_OBJECT_ALIGNED(GMShader)
{
	GMIlluminationModel illuminationModel = GMIlluminationModel::Phong;
	GMint32 flag = 0;
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
	bool visible = true;
	bool culled = false;
	bool noDepthTest = false;
	GMVec3 lineColor = GMVec3(0);
	GMTextureList textureList;
	GMMaterial material;
};

GM_DEFINE_PROPERTY(GMShader, GMint32, Flag, flag);
GM_DEFINE_PROPERTY(GMShader, GMS_Cull, Cull, cull);
GM_DEFINE_PROPERTY(GMShader, GMS_FrontFace, FrontFace, frontFace);
GM_DEFINE_PROPERTY(GMShader, GMS_BlendFunc, BlendFactorSourceRGB, blendFactorSrcRGB);
GM_DEFINE_PROPERTY(GMShader, GMS_BlendFunc, BlendFactorDestRGB, blendFactorDestRGB);
GM_DEFINE_PROPERTY(GMShader, GMS_BlendOp, BlendOpRGB, blendOpRGB);
GM_DEFINE_PROPERTY(GMShader, GMS_BlendFunc, BlendFactorSourceAlpha, blendFactorSrcAlpha);
GM_DEFINE_PROPERTY(GMShader, GMS_BlendFunc, BlendFactorDestAlpha, blendFactorDestAlpha);
GM_DEFINE_PROPERTY(GMShader, GMS_BlendOp, BlendOpAlpha, blendOpAlpha);
GM_DEFINE_PROPERTY(GMShader, bool, Blend, blend);
GM_DEFINE_PROPERTY(GMShader, bool, Visible, visible);
GM_DEFINE_PROPERTY(GMShader, bool, NoDepthTest, noDepthTest);
GM_DEFINE_PROPERTY(GMShader, GMTextureList, TextureList, textureList);
GM_DEFINE_PROPERTY(GMShader, GMVec3, LineColor, lineColor);
GM_DEFINE_PROPERTY(GMShader, GMMaterial, Material, material);
GM_DEFINE_PROPERTY(GMShader, GMIlluminationModel, IlluminationModel, illuminationModel);
GM_DEFINE_PROPERTY(GMShader, GMS_VertexColorOp, VertexColorOp, vertexColorOp)

GMShader::GMShader()
{
	GM_CREATE_DATA();
}

GMShader::~GMShader()
{

}

GMShader::GMShader(const GMShader& rhs)
{
	*this = rhs;
}

GMShader::GMShader(GMShader&& rhs) GM_NOEXCEPT
{
	*this = std::move(rhs);
}

GMShader& GMShader::operator=(const GMShader& rhs)
{
	GM_COPY(rhs);
	return *this;
}

GMShader& GMShader::operator=(GMShader&& rhs) GM_NOEXCEPT
{
	GM_MOVE(rhs);
	return *this;
}

bool GMShader::isCulled()
{
	D(d);
	return d->culled;
}

void GMShader::setCulled(bool culled)
{
	// 设置一个物体是否被裁剪，如果为true，则表示它不在frustum内，一般情况下不进行绘制
	D(d);
	d->culled = culled;
}

END_NS
