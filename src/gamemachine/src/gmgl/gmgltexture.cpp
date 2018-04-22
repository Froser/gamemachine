#include "stdafx.h"
#include "gmgltexture.h"
#include "gmdata/imagereader/gmimagereader.h"
#include "shader_constants.h"
#include "gmdata/gmmodel.h"
#include <GL/glew.h>

namespace
{
	inline GLenum toGLTarget(GMImageTarget target)
	{
		switch (target)
		{
		case GMImageTarget::Texture1D:
			return GL_TEXTURE_1D;
		case GMImageTarget::Texture1DArray:
			return GL_TEXTURE_1D_ARRAY;
		case GMImageTarget::Texture2D:
			return GL_TEXTURE_2D;
		case GMImageTarget::Texture2DArray:
			return GL_TEXTURE_2D_ARRAY;
		case GMImageTarget::Texture3D:
			return GL_TEXTURE_3D;
		case GMImageTarget::CubeMap:
			return GL_TEXTURE_CUBE_MAP;
		case GMImageTarget::CubeMapArray:
			return GL_TEXTURE_CUBE_MAP_ARRAY;
		default:
			GM_ASSERT(false);
			return GL_NONE;
		}
	}

	inline GLenum toGLFormat(GMImageFormat format)
	{
		switch (format)
		{
		case GMImageFormat::RGB:
			return GL_RGB;
		case GMImageFormat::RGBA:
			return GL_RGBA;
		case GMImageFormat::BGRA:
			return GL_BGRA;
		default:
			GM_ASSERT(false);
			return GL_NONE;
		}
	}

	inline GLenum toGLInternalFormat(GMImageInternalFormat internalFormat)
	{
		switch (internalFormat)
		{
		case GMImageInternalFormat::RGB8:
			return GL_RGB8;
		case GMImageInternalFormat::RGBA8:
			return GL_RGBA8;
		default:
			GM_ASSERT(false);
			return GL_NONE;
		}
	}

	inline GLenum toGLImageDataType(GMImageDataType type)
	{
		switch (type)
		{
		case GMImageDataType::UnsignedByte:
			return GL_UNSIGNED_BYTE;
		case GMImageDataType::Float:
			return GL_FLOAT;
		default:
			GM_ASSERT(false);
			return GL_NONE;
		}
	}
}

GMGLTexture::GMGLTexture(const GMImage* image)
{
	D(d);
	if (image)
	{
		d->target = toGLTarget(image->getData().target);
		d->format = toGLFormat(image->getData().format);
		d->dataType = toGLImageDataType(image->getData().type);
		d->internalFormat = toGLInternalFormat(image->getData().internalFormat);
		d->image = image;
	}
}

GMGLTexture::~GMGLTexture()
{
	D(d);
	glDeleteTextures(1, &d->id);
	d->inited = false;
}

void GMGLTexture::init()
{
	D(d);
	if (d->inited)
		return;

	if (!d->image)
	{
		GM_ASSERT(false);
		return;
	}

	GMint level;
	const GMImage::Data& imgData = d->image->getData();

	glGenTextures(1, &d->id);
	glBindTexture(d->target, d->id);

	switch (d->target)
	{
	case GL_TEXTURE_2D:
		glTexStorage2D(d->target,
			imgData.mipLevels,
			d->internalFormat,
			imgData.mip[0].width,
			imgData.mip[0].height);
		for (level = 0; level < imgData.mipLevels; ++level)
		{
			glTexSubImage2D(GL_TEXTURE_2D,
				level,
				0, 0,
				imgData.mip[level].width, imgData.mip[level].height,
				d->format, d->dataType,
				imgData.mip[level].data);
		}
		break;
	case GL_TEXTURE_CUBE_MAP:
		for (level = 0; level < imgData.mipLevels; ++level)
		{
			GMbyte* ptr = (GMbyte *)imgData.mip[level].data;
			for (int face = 0; face < 6; face++)
			{
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
					level,
					d->internalFormat,
					imgData.mip[level].width, imgData.mip[level].height,
					0,
					d->format, d->dataType,
					ptr + imgData.sliceStride * face);
			}
		}
		break;
	default:
		break;
	}

	glTexParameteriv(d->target, GL_TEXTURE_SWIZZLE_RGBA, reinterpret_cast<const GLint *>(imgData.swizzle));
	glBindTexture(d->target, 0);
	d->inited = true;
}

void GMGLTexture::drawTexture(GMTextureFrames* frames, GMint textureIndex)
{
	D(d);
	glActiveTexture(GL_TEXTURE0 + textureIndex);
	glBindTexture(d->target, d->id);

	if (!d->texParamsSet)
	{
		// Apply params
		glTexParameteri(d->target, GL_TEXTURE_MIN_FILTER,
			frames->getMinFilter() == GMS_TextureFilter::LINEAR ? GL_LINEAR :
			frames->getMinFilter() == GMS_TextureFilter::NEAREST ? GL_NEAREST :
			frames->getMinFilter() == GMS_TextureFilter::LINEAR_MIPMAP_LINEAR ? GL_LINEAR_MIPMAP_LINEAR :
			frames->getMinFilter() == GMS_TextureFilter::NEAREST_MIPMAP_LINEAR ? GL_NEAREST_MIPMAP_LINEAR :
			frames->getMinFilter() == GMS_TextureFilter::LINEAR_MIPMAP_NEAREST ? GL_LINEAR_MIPMAP_NEAREST :
			frames->getMinFilter() == GMS_TextureFilter::NEAREST_MIPMAP_NEAREST ? GL_NEAREST_MIPMAP_NEAREST : GL_LINEAR
		);

		glTexParameteri(d->target, GL_TEXTURE_MAG_FILTER,
			frames->getMagFilter() == GMS_TextureFilter::LINEAR ? GL_LINEAR :
			frames->getMagFilter() == GMS_TextureFilter::NEAREST ? GL_NEAREST :
			frames->getMagFilter() == GMS_TextureFilter::LINEAR_MIPMAP_LINEAR ? GL_LINEAR_MIPMAP_LINEAR :
			frames->getMagFilter() == GMS_TextureFilter::NEAREST_MIPMAP_LINEAR ? GL_NEAREST_MIPMAP_LINEAR :
			frames->getMagFilter() == GMS_TextureFilter::LINEAR_MIPMAP_NEAREST ? GL_LINEAR_MIPMAP_NEAREST :
			frames->getMagFilter() == GMS_TextureFilter::NEAREST_MIPMAP_NEAREST ? GL_NEAREST_MIPMAP_NEAREST : GL_LINEAR
		);

		glTexParameteri(d->target, GL_TEXTURE_WRAP_S,
			frames->getWrapS() == GMS_Wrap::REPEAT ? GL_REPEAT :
			frames->getWrapS() == GMS_Wrap::CLAMP_TO_EDGE ? GL_CLAMP_TO_EDGE :
			frames->getWrapS() == GMS_Wrap::CLAMP_TO_BORDER ? GL_CLAMP_TO_BORDER :
			frames->getWrapS() == GMS_Wrap::MIRRORED_REPEAT ? GL_MIRRORED_REPEAT : GL_REPEAT
		);

		glTexParameteri(d->target, GL_TEXTURE_WRAP_T,
			frames->getWrapT() == GMS_Wrap::REPEAT ? GL_REPEAT :
			frames->getWrapT() == GMS_Wrap::CLAMP_TO_EDGE ? GL_CLAMP_TO_EDGE :
			frames->getWrapT() == GMS_Wrap::CLAMP_TO_BORDER ? GL_CLAMP_TO_BORDER :
			frames->getWrapT() == GMS_Wrap::MIRRORED_REPEAT ? GL_MIRRORED_REPEAT : GL_REPEAT
		);

		if (d->target == GL_TEXTURE_CUBE_MAP)
		{
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		}
		d->texParamsSet = true;
	}
}
