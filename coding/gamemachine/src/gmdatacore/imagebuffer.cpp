#include "stdafx.h"
#include "imagebuffer.h"

ImageBuffer::ImageBuffer(GMuint width, GMuint height, GMuint bufferSize, GMbyte* buffer)
{
	D(d);
	d->width = width;
	d->height = height;
	d->buffer = new GMbyte[bufferSize];
	memcpy(d->buffer, buffer, sizeof(GMbyte) * bufferSize);

	generateData();
}

void ImageBuffer::generateData()
{
	D(d);
	Image::Data& data = getData();

#if USE_OPENGL
	data.target = GL_TEXTURE_2D;
	data.mipLevels = 1;
	data.internalFormat = GL_RGBA8;
	data.format = GL_RGB;
	data.swizzle[0] = GL_RED;
	data.swizzle[1] = GL_GREEN;
	data.swizzle[2] = GL_BLUE;
	data.swizzle[3] = GL_ALPHA;
	data.type = GL_UNSIGNED_BYTE;
	data.mip[0].height = d->height;
	data.mip[0].width = d->width;
	// Buffer 移交给 Image 管理
	data.mip[0].data = d->buffer;
#else
	ASSERT(false);
#endif
}