#include "stdafx.h"
#include "gmgltexture.h"
#include "gmdata/imagereader/gmimagereader.h"
#include "shader_constants.h"
#include "gmdata/gmmodel.h"

GMGLTexture::GMGLTexture(const GMImage* image)
{
	D(d);
	d->target = image->getData().target;
	init(image);
}

GMGLTexture::~GMGLTexture()
{
	D(d);
	glDeleteTextures(1, &d->id);
	d->inited = false;
}

void GMGLTexture::init(const GMImage* image)
{
	D(d);
	if (d->inited)
		return;

	GMint level;
	const GMImage::Data& imgData = image->getData();

	glGenTextures(1, &d->id);
	glBindTexture(imgData.target, d->id);

	switch (imgData.target)
	{
	case GL_TEXTURE_1D:
		glTexStorage1D(imgData.target,
			imgData.mipLevels,
			imgData.internalFormat,
			imgData.mip[0].width);
		for (level = 0; level < imgData.mipLevels; ++level)
		{
			glTexSubImage1D(GL_TEXTURE_1D,
				level,
				0,
				imgData.mip[level].width,
				imgData.format, imgData.type,
				imgData.mip[level].data);
		}
		break;
	case GL_TEXTURE_1D_ARRAY:
		glTexStorage2D(imgData.target,
			imgData.mipLevels,
			imgData.internalFormat,
			imgData.mip[0].width,
			imgData.slices);
		for (level = 0; level < imgData.mipLevels; ++level)
		{
			glTexSubImage2D(GL_TEXTURE_1D,
				level,
				0, 0,
				imgData.mip[level].width, imgData.slices,
				imgData.format, imgData.type,
				imgData.mip[level].data);
		}
		break;
	case GL_TEXTURE_2D:
		glTexStorage2D(imgData.target,
			imgData.mipLevels,
			imgData.internalFormat,
			imgData.mip[0].width,
			imgData.mip[0].height);
		for (level = 0; level < imgData.mipLevels; ++level)
		{
			glTexSubImage2D(GL_TEXTURE_2D,
				level,
				0, 0,
				imgData.mip[level].width, imgData.mip[level].height,
				imgData.format, imgData.type,
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
					imgData.internalFormat,
					imgData.mip[level].width, imgData.mip[level].height,
					0,
					imgData.format, imgData.type,
					ptr + imgData.sliceStride * face);
			}
		}
		break;
	case GL_TEXTURE_2D_ARRAY:
		glTexStorage3D(imgData.target,
			imgData.mipLevels,
			imgData.internalFormat,
			imgData.mip[0].width,
			imgData.mip[0].height,
			imgData.slices);
		for (level = 0; level < imgData.mipLevels; ++level)
		{
			glTexSubImage3D(GL_TEXTURE_2D_ARRAY,
				level,
				0, 0, 0,
				imgData.mip[level].width, imgData.mip[level].height, imgData.slices,
				imgData.format, imgData.type,
				imgData.mip[level].data);
		}
		break;
	case GL_TEXTURE_CUBE_MAP_ARRAY:
		glTexStorage3D(imgData.target,
			imgData.mipLevels,
			imgData.internalFormat,
			imgData.mip[0].width,
			imgData.mip[0].height,
			imgData.slices);
		break;
	case GL_TEXTURE_3D:
		glTexStorage3D(imgData.target,
			imgData.mipLevels,
			imgData.internalFormat,
			imgData.mip[0].width,
			imgData.mip[0].height,
			imgData.mip[0].depth);
		for (level = 0; level < imgData.mipLevels; ++level)
		{
			glTexSubImage3D(GL_TEXTURE_3D,
				level,
				0, 0, 0,
				imgData.mip[level].width, imgData.mip[level].height, imgData.mip[level].depth,
				imgData.format, imgData.type,
				imgData.mip[level].data);
		}
		break;
	default:
		break;
	}

	glTexParameteriv(imgData.target, GL_TEXTURE_SWIZZLE_RGBA, reinterpret_cast<const GLint *>(imgData.swizzle));
	glBindTexture(imgData.target, 0);
	d->inited = true;
}

void GMGLTexture::drawTexture(GMTextureFrames* frames)
{
	D(d);
	glBindTexture(d->target, d->id);

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
}
