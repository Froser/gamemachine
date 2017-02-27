#include "stdafx.h"
#include "gmgltexture.h"
#include "gmdatacore/imagereader/imagereader.h"
#include "shader_constants.h"
#include "gmdatacore/object.h"
#include "utilities/assert.h"

GMGLTextureShaderNames::GMGLTextureShaderNames()
{
	m_uniformNames[TEXTURE_INDEX_AMBIENT] = GMSHADER_AMBIENT_TEXTURE;
	m_uniformNames[TEXTURE_INDEX_AMBIENT_2] = GMSHADER_AMBIENT_TEXTURE_2;
	m_uniformNames[TEXTURE_INDEX_AMBIENT_3] = GMSHADER_AMBIENT_TEXTURE_3;
	m_uniformNames[TEXTURE_INDEX_DIFFUSE] = GMSHADER_DIFFUSE_TEXTURE;
	m_uniformNames[TEXTURE_INDEX_NORMAL_MAPPING] = GMSHADER_NORMAL_MAPPING_TEXTURE;
	m_uniformNames[TEXTURE_INDEX_LIGHTMAP] = GMSHADER_LIGHTMAP_TEXTURE;
}

const char* GMGLTextureShaderNames::operator [](TextureIndex t)
{
	ASSERT(m_uniformNames.find(t) != m_uniformNames.end());
	return m_uniformNames[t].c_str();
}

GMGLTexture::GMGLTexture(AUTORELEASE Image* image)
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

	GMuint level;
	const ImageData& image = m_image->getData();

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

void GMGLTexture::beginTexture(TextureInfo* ti)
{
	const ImageData& image = m_image->getData();
	glBindTexture(image.target, m_id);

	// Apply params
	if (ti->blend)
	{
		glEnable(GL_BLEND);
		GLenum factors[2];
		for (GMuint i = 0; i < 2; i++)
		{
			switch (ti->blendFactors[i])
			{
			case GMS_ZERO:
				factors[i] = GL_ZERO;
				break;
			case GMS_ONE:
				factors[i] = GL_ONE;
				break;
			default:
				ASSERT(false);
				break;
			}
		}
		glBlendFunc(factors[0], factors[1]);
	}
	else
	{
		glDisable(GL_BLEND);
	}

	glTexParameteri(image.target, GL_TEXTURE_MIN_FILTER,
		ti->minFilter == GMS_LINEAR ? GL_LINEAR :
		ti->minFilter == GMS_NEAREST ? GL_NEAREST :
		ti->minFilter == GMS_LINEAR_MIPMAP_LINEAR ? GL_LINEAR_MIPMAP_LINEAR :
		ti->minFilter == GMS_NEAREST_MIPMAP_LINEAR ? GL_NEAREST_MIPMAP_LINEAR :
		ti->minFilter == GMS_LINEAR_MIPMAP_NEAREST ? GL_LINEAR_MIPMAP_NEAREST :
		ti->minFilter == GMS_NEAREST_MIPMAP_NEAREST ? GL_NEAREST_MIPMAP_NEAREST : GL_LINEAR
	);

	glTexParameteri(image.target, GL_TEXTURE_MAG_FILTER,
		ti->magFilter == GMS_LINEAR ? GL_LINEAR :
		ti->magFilter == GMS_NEAREST ? GL_NEAREST :
		ti->magFilter == GMS_LINEAR_MIPMAP_LINEAR ? GL_LINEAR_MIPMAP_LINEAR :
		ti->magFilter == GMS_NEAREST_MIPMAP_LINEAR ? GL_NEAREST_MIPMAP_LINEAR :
		ti->magFilter == GMS_LINEAR_MIPMAP_NEAREST ? GL_LINEAR_MIPMAP_NEAREST :
		ti->magFilter == GMS_NEAREST_MIPMAP_NEAREST ? GL_NEAREST_MIPMAP_NEAREST : GL_LINEAR
	);

	glTexParameteri(image.target, GL_TEXTURE_WRAP_S, 
		ti->wrapS == GMS_REPEAT ? GL_REPEAT :
		ti->wrapS == GMS_CLAMP_TO_EDGE ? GL_CLAMP_TO_EDGE :
		ti->wrapS == GMS_CLAMP_TO_BORDER ? GL_CLAMP_TO_BORDER :
		ti->wrapS == GMS_MIRRORED_REPEAT ? GL_MIRRORED_REPEAT : GL_REPEAT
	);
	
	glTexParameteri(image.target, GL_TEXTURE_WRAP_T,
		ti->wrapT == GMS_REPEAT ? GL_REPEAT :
		ti->wrapT == GMS_CLAMP_TO_EDGE ? GL_CLAMP_TO_EDGE :
		ti->wrapT == GMS_CLAMP_TO_BORDER ? GL_CLAMP_TO_BORDER :
		ti->wrapT == GMS_MIRRORED_REPEAT ? GL_MIRRORED_REPEAT : GL_REPEAT
	);
}

void GMGLTexture::endTexture()
{
	const ImageData& image = m_image->getData();
	glBindTexture(image.target, 0);
}