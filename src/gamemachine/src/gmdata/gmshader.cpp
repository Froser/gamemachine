#include "stdafx.h"
#include "gmshader.h"

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

ITexture* GMTextureSampler::getFrameByIndex(GMsize_t frameIndex)
{
	D(d);
	GM_ASSERT(!d->frames[frameIndex].isEmpty());
	GM_ASSERT(d->frames[frameIndex].getType() == GMAssetType::Texture);
	return d->frames[frameIndex].getTexture();
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
	for (GMint i = 0; i < MAX_ANIMATION_FRAME; i++)
	{
		d->frames[i] = rhs_d->frames[i];
	}
	for (GMint i = 0; i < MAX_TEX_TRANS; i++)
	{
		d->texTransform[i] = rhs_d->texTransform[i];
	}
	return *this;
}

void GMTextureSampler::applyTexMode(GMfloat timeSeconds, std::function<void(GMS_TextureTransformType, Pair<GMfloat, GMfloat>&&)> callback)
{
	GMuint n = 0;
	const GMS_TextureTransform& tc = getTextureTransform(n);
	while (n < MAX_TEX_TRANS && tc.type != GMS_TextureTransformType::NoTextureTransform)
	{
		switch (tc.type)
		{
		case GMS_TextureTransformType::Scroll:
		{
			GMfloat s = timeSeconds * tc.p1, t = timeSeconds * tc.p2;
			callback(tc.type, { s, t });
		}
		break;
		case GMS_TextureTransformType::Scale:
		{
			GMfloat s = tc.p1, t = tc.p2;
			callback(tc.type, { s, t });
			break;
		}
		default:
			break;
		}
		n++;
	}
}