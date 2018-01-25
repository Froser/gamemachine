#include "stdafx.h"
#include "gmimagebuffer.h"
#include <GL/glew.h>

GMImageBuffer::GMImageBuffer(GMuint width, GMuint height, GMuint bufferSize, GMbyte* buffer)
{
	D(d);
	d->width = width;
	d->height = height;
	d->buffer = new GMbyte[bufferSize];
	memcpy(d->buffer, buffer, sizeof(GMbyte) * bufferSize);

	generateData();
}

void GMImageBuffer::generateData()
{
	D(d);
	GMImage::Data& data = getData();

#if GM_USE_OPENGL
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
	GM_ASSERT(false);
#endif
}

GMCubeMapBuffer::GMCubeMapBuffer(
	const GMImage& posX,
	const GMImage& negX,
	const GMImage& posY,
	const GMImage& negY,
	const GMImage& posZ,
	const GMImage& negZ)
{
	D_BASE(d, GMImage);
	GMImage::Data& data = getData();

#if GM_USE_OPENGL
	data.target = GL_TEXTURE_CUBE_MAP;
	data.mipLevels = 1;
	data.internalFormat = GL_RGBA8;
	data.format = GL_RGB;
	data.swizzle[0] = GL_RED;
	data.swizzle[1] = GL_GREEN;
	data.swizzle[2] = GL_BLUE;
	data.swizzle[3] = GL_ALPHA;
	data.type = GL_UNSIGNED_BYTE;
	data.mip[0].height = posX.getWidth();
	data.mip[0].width = posY.getHeight();
#else
	GM_ASSERT(false);
#endif
	size_t totalSize = posX.getData().size +
		posY.getData().size +
		posZ.getData().size +
		negX.getData().size +
		negY.getData().size +
		negZ.getData().size;

	// Buffer 移交给 Image 管理
	data.mip[0].data = new GMbyte[totalSize];
	data.slices = 6;
	data.sliceStride = posX.getData().size;

	const GMImage* slices[] = {
		&posX,
		&negX,
		&negY, // 由于uv坐标和gl坐标y方向相反，所以我们特意把negY（天空）和posY（地面）颠倒一下
		&posY,
		&posZ,
		&negZ
	};
	GMbyte* ptr = data.mip[0].data;
	for (GMint i = 0; i < GM_array_size(slices); ++i, ptr+=data.sliceStride)
	{
		memcpy(ptr, slices[i]->getData().mip[0].data, data.sliceStride);
	}
}