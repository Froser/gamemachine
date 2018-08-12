#include "stdafx.h"
#include "gmshader.h"

GMS_TextureMod& GMTextureSampler::getTexMod(GMsize_t index)
{
	D(d);
	return d->texMod[index];
}

void GMTextureSampler::setTexMod(GMsize_t index, const GMS_TextureMod& mod)
{
	D(d);
	d->texMod[index] = mod;
}

ITexture* GMTextureSampler::getFrameByIndex(GMsize_t frameIndex)
{
	D(d);
	return d->frames[frameIndex];
}

GMsize_t GMTextureSampler::addFrame(ITexture* oneFrame)
{
	D(d);
	d->frames[d->frameCount] = oneFrame;
	++d->frameCount;
	return d->frameCount;
}

bool GMTextureSampler::setTexture(GMsize_t frameIndex, ITexture* texture)
{
	D(d);
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
	for (GMint i = 0; i < MAX_TEX_MOD; i++)
	{
		d->texMod[i] = rhs_d->texMod[i];
	}
	return *this;
}

void GMTextureSampler::applyTexMode(GMfloat timeSeconds, std::function<void(GMS_TextureTransformType, Pair<GMfloat, GMfloat>&&)> callback)
{
	GMuint n = 0;
	const GMS_TextureMod& tc = getTexMod(n);
	while (n < MAX_TEX_MOD && tc.type != GMS_TextureTransformType::NoTextureTransform)
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