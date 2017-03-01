#include "stdafx.h"
#include "image.h"

Image::Image()
{
	memset(&m_data, 0, sizeof(m_data));
}

Image::~Image()
{
	dispose();
}

ImageData& Image::getData()
{
	return m_data;
}

void Image::dispose()
{
	if (m_data.mip[0].data)
	{
		delete[] reinterpret_cast<GMbyte *>(m_data.mip[0].data);
		m_data.mip[0].data = nullptr;
	}
}

void Image::flipVertically(GMuint mipId)
{
	ImageMipData* mip = &m_data.mip[mipId];
	GMint width = mip->width,
		height = mip->height;
	GMbyte* buffer = mip->data;
	GMuint rowsToSwap = height % 2 == 1 ? (height - 1) / 2 : height / 2;

	GMbyte* tempRow = new GMbyte[width * mip->depth / 8];
	for (int i = 0; i < rowsToSwap; ++i)
	{
		memcpy(tempRow, &mip->data[i * width * mip->depth / 8], width * mip->depth / 8);
		memcpy(&mip->data[i * width * mip->depth / 8], &mip->data[(height - i - 1) * width * mip->depth / 8], width * mip->depth / 8);
		memcpy(&mip->data[(height - i - 1) * width * mip->depth / 8], tempRow, width * mip->depth / 8);
	}

	if (tempRow)
		delete[] tempRow;
}