#include "stdafx.h"
#include "gmimagebuffer.h"
#include <GL/glew.h>
#include "gmdata/gmimage_p.h"

BEGIN_NS

GM_PRIVATE_OBJECT_UNALIGNED(GMImageBuffer)
{
	GMuint32 width;
	GMuint32 height;
	GMsize_t size = 0;
	GMbyte* buffer = nullptr;
};

GMImageBuffer::GMImageBuffer(GMImageFormat format, GMuint32 width, GMuint32 height, GMsize_t bufferSize, GMbyte* buffer)
{
	GM_CREATE_DATA();

	D(d);
	d->width = width;
	d->height = height;
	d->size = bufferSize / 3 * 4;

	if (format == GMImageFormat::RGB)
	{
		d->buffer = new GMbyte[d->size];
		GMuint32 writePtr = 0;
		for (GMuint32 readPtr = 0; readPtr < bufferSize;)
		{
			d->buffer[writePtr++] = buffer[readPtr++];
			if (readPtr % 3 == 0)
				d->buffer[writePtr++] = 0xFF; //ALPHA
		}
	}
	else
	{
		if (buffer > 0)
		{
			d->buffer = new GMbyte[d->size];
			memcpy(d->buffer, buffer, sizeof(GMbyte) * bufferSize);
		}
	}
	getData().format = GMImageFormat::RGBA;

	generateData();
}

GMImageBuffer::~GMImageBuffer()
{

}

void GMImageBuffer::generateData()
{
	D(d);
	GMImage::Data& data = getData();

	data.target = GMImageTarget::Texture2D;
	data.mipLevels = 1;
	data.internalFormat = GMImageInternalFormat::RGBA8;
	data.type = GMImageDataType::UnsignedByte;
	data.mip[0].height = d->height;
	data.mip[0].width = d->width;
	// Buffer 移交给 Image 管理
	data.mip[0].data = d->buffer;
	data.size = d->size;
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

	GM_ASSERT(
		posX.getData().format == negX.getData().format &&
		posX.getData().format == posY.getData().format &&
		posX.getData().format == negY.getData().format &&
		posX.getData().format == posZ.getData().format &&
		posX.getData().format == negZ.getData().format
	);

	data.target = GMImageTarget::CubeMap;
	data.mipLevels = 1;
	data.internalFormat = GMImageInternalFormat::RGBA8;
	data.format = posX.getData().format;
	data.type = GMImageDataType::UnsignedByte;
	data.mip[0].height = posX.getWidth();
	data.mip[0].width = posY.getHeight();

	GMsize_t totalSize = posX.getData().size +
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
		&posY,
		&negY,
		&posZ,
		&negZ
	};
	GMbyte* ptr = data.mip[0].data;
	for (GMint32 i = 0; i < GM_array_size(slices); ++i)
	{
		GMsize_t sz = slices[i]->getData().size;
		memcpy(ptr, slices[i]->getData().mip[0].data, data.sliceStride);
		ptr += sz;
	}
}

END_NS