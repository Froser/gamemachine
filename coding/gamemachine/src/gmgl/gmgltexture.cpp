#include "stdafx.h"
#include "gmgltexture.h"
#include "gmdatacore/imagereader/gmimagereader.h"
#include "shader_constants.h"
#include "gmdatacore/gmmodel.h"

GMGLTexture::GMGLTexture(AUTORELEASE GMImage* image)
	: m_inited(false)
{
	m_image.reset(image);
	init();
}

GMGLTexture::~GMGLTexture()
{
	glDeleteTextures(1, &m_id);
	m_inited = false;
}

void GMGLTexture::init()
{
	if (m_inited)
		return;

	GMint level;
	const GMImage::Data& image = m_image->getData();

	glGenTextures(1, &m_id);
	glBindTexture(image.target, m_id);

	switch (image.target)
	{
	case GL_TEXTURE_1D:
		glTexStorage1D(image.target,
			image.mipLevels,
			image.internalFormat,
			image.mip[0].width);
		for (level = 0; level < image.mipLevels; ++level)
		{
			glTexSubImage1D(GL_TEXTURE_1D,
				level,
				0,
				image.mip[level].width,
				image.format, image.type,
				image.mip[level].data);
		}
		break;
	case GL_TEXTURE_1D_ARRAY:
		glTexStorage2D(image.target,
			image.mipLevels,
			image.internalFormat,
			image.mip[0].width,
			image.slices);
		for (level = 0; level < image.mipLevels; ++level)
		{
			glTexSubImage2D(GL_TEXTURE_1D,
				level,
				0, 0,
				image.mip[level].width, image.slices,
				image.format, image.type,
				image.mip[level].data);
		}
		break;
	case GL_TEXTURE_2D:
		glTexStorage2D(image.target,
			image.mipLevels,
			image.internalFormat,
			image.mip[0].width,
			image.mip[0].height);
		for (level = 0; level < image.mipLevels; ++level)
		{
			glTexSubImage2D(GL_TEXTURE_2D,
				level,
				0, 0,
				image.mip[level].width, image.mip[level].height,
				image.format, image.type,
				image.mip[level].data);
		}
		break;
	case GL_TEXTURE_CUBE_MAP:
		for (level = 0; level < image.mipLevels; ++level)
		{
			GMbyte* ptr = (GMbyte *)image.mip[level].data;
			for (int face = 0; face < 6; face++)
			{
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
					level,
					image.internalFormat,
					image.mip[level].width, image.mip[level].height,
					0,
					image.format, image.type,
					ptr + image.sliceStride * face);
			}
		}
		break;
	case GL_TEXTURE_2D_ARRAY:
		glTexStorage3D(image.target,
			image.mipLevels,
			image.internalFormat,
			image.mip[0].width,
			image.mip[0].height,
			image.slices);
		for (level = 0; level < image.mipLevels; ++level)
		{
			glTexSubImage3D(GL_TEXTURE_2D_ARRAY,
				level,
				0, 0, 0,
				image.mip[level].width, image.mip[level].height, image.slices,
				image.format, image.type,
				image.mip[level].data);
		}
		break;
	case GL_TEXTURE_CUBE_MAP_ARRAY:
		glTexStorage3D(image.target,
			image.mipLevels,
			image.internalFormat,
			image.mip[0].width,
			image.mip[0].height,
			image.slices);
		break;
	case GL_TEXTURE_3D:
		glTexStorage3D(image.target,
			image.mipLevels,
			image.internalFormat,
			image.mip[0].width,
			image.mip[0].height,
			image.mip[0].depth);
		for (level = 0; level < image.mipLevels; ++level)
		{
			glTexSubImage3D(GL_TEXTURE_3D,
				level,
				0, 0, 0,
				image.mip[level].width, image.mip[level].height, image.mip[level].depth,
				image.format, image.type,
				image.mip[level].data);
		}
		break;
	default:
		break;
	}

	glTexParameteriv(image.target, GL_TEXTURE_SWIZZLE_RGBA, reinterpret_cast<const GLint *>(image.swizzle));
	glBindTexture(image.target, 0);
	m_image->dispose();
	m_inited = true;
}

void GMGLTexture::drawTexture(GMTextureFrames* frames)
{
	const GMImage::Data& image = m_image->getData();
	glBindTexture(image.target, m_id);

	// Apply params
	glTexParameteri(image.target, GL_TEXTURE_MIN_FILTER,
		frames->getMinFilter() == GMS_TextureFilter::LINEAR ? GL_LINEAR :
		frames->getMinFilter() == GMS_TextureFilter::NEAREST ? GL_NEAREST :
		frames->getMinFilter() == GMS_TextureFilter::LINEAR_MIPMAP_LINEAR ? GL_LINEAR_MIPMAP_LINEAR :
		frames->getMinFilter() == GMS_TextureFilter::NEAREST_MIPMAP_LINEAR ? GL_NEAREST_MIPMAP_LINEAR :
		frames->getMinFilter() == GMS_TextureFilter::LINEAR_MIPMAP_NEAREST ? GL_LINEAR_MIPMAP_NEAREST :
		frames->getMinFilter() == GMS_TextureFilter::NEAREST_MIPMAP_NEAREST ? GL_NEAREST_MIPMAP_NEAREST : GL_LINEAR
	);

	glTexParameteri(image.target, GL_TEXTURE_MAG_FILTER,
		frames->getMagFilter() == GMS_TextureFilter::LINEAR ? GL_LINEAR :
		frames->getMagFilter() == GMS_TextureFilter::NEAREST ? GL_NEAREST :
		frames->getMagFilter() == GMS_TextureFilter::LINEAR_MIPMAP_LINEAR ? GL_LINEAR_MIPMAP_LINEAR :
		frames->getMagFilter() == GMS_TextureFilter::NEAREST_MIPMAP_LINEAR ? GL_NEAREST_MIPMAP_LINEAR :
		frames->getMagFilter() == GMS_TextureFilter::LINEAR_MIPMAP_NEAREST ? GL_LINEAR_MIPMAP_NEAREST :
		frames->getMagFilter() == GMS_TextureFilter::NEAREST_MIPMAP_NEAREST ? GL_NEAREST_MIPMAP_NEAREST : GL_LINEAR
	);

	glTexParameteri(image.target, GL_TEXTURE_WRAP_S, 
		frames->getWrapS() == GMS_Wrap::REPEAT ? GL_REPEAT :
		frames->getWrapS() == GMS_Wrap::CLAMP_TO_EDGE ? GL_CLAMP_TO_EDGE :
		frames->getWrapS() == GMS_Wrap::CLAMP_TO_BORDER ? GL_CLAMP_TO_BORDER :
		frames->getWrapS() == GMS_Wrap::MIRRORED_REPEAT ? GL_MIRRORED_REPEAT : GL_REPEAT
	);
	
	glTexParameteri(image.target, GL_TEXTURE_WRAP_T,
		frames->getWrapT() == GMS_Wrap::REPEAT ? GL_REPEAT :
		frames->getWrapT() == GMS_Wrap::CLAMP_TO_EDGE ? GL_CLAMP_TO_EDGE :
		frames->getWrapT() == GMS_Wrap::CLAMP_TO_BORDER ? GL_CLAMP_TO_BORDER :
		frames->getWrapT() == GMS_Wrap::MIRRORED_REPEAT ? GL_MIRRORED_REPEAT : GL_REPEAT
	);
}
