#include "stdafx.h"
#include "imagebuffer.h"

ImageBuffer::ImageBuffer(GMuint width, GMuint height, GMuint bufferSize, GMbyte* buffer)
	: m_width(width)
	, m_height(height)
{
	m_buffer = new GMbyte[bufferSize];
	memcpy(m_buffer, buffer, sizeof(GMbyte) * bufferSize);

	generateData();
}

void ImageBuffer::generateData()
{
	ImageData& data = getData();

#ifdef USE_OPENGL
	data.target = GL_TEXTURE_2D;
	data.mipLevels = 1;
	data.internalFormat = GL_RGBA8;
	data.format = GL_RGB;
	data.swizzle[0] = GL_RED;
	data.swizzle[1] = GL_GREEN;
	data.swizzle[2] = GL_BLUE;
	data.swizzle[3] = GL_ALPHA;
	data.type = GL_UNSIGNED_BYTE;
	data.mip[0].height = m_height;
	data.mip[0].width = m_width;
	// Buffer 移交给 Image 管理
	data.mip[0].data = m_buffer;
#else
	ASSERT(false);
#endif
}