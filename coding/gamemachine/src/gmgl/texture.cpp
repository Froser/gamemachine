#include "stdafx.h"
#include "texture.h"
#include "gmdatacore/imagereader/imagereader.h"

GMGLTexture::~GMGLTexture()
{
	glDeleteTextures(1, &m_id);
}

void GMGLTexture::loadTexture(const char* filename, GMGLTexture& texture)
{
	Image* image;
	ImageReader::load(filename, DDS, &image);
	texture.m_image.reset(image);
	texture.m_id = image->loadTexture();
}