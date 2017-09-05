#include "stdafx.h"
#include <gmimage.h>

GMImage::GMImage()
{
}

GMImage::~GMImage()
{
	dispose();
}

GMImage::Data& GMImage::getData()
{
	D(d);
	return *d;
}

void GMImage::dispose()
{
	D(d);
	if (d->mip[0].data)
	{
		delete[] reinterpret_cast<GMbyte *>(d->mip[0].data);
		d->mip[0].data = nullptr;
	}
}

void GMImage::flipVertically(GMuint mipId)
{
	D(d);
	ImageMipData* mip = &d->mip[mipId];
	GMint width = mip->width,
		height = mip->height;
	GMuint rowsToSwap = height % 2 == 1 ? (height - 1) / 2 : height / 2;

	GMbyte* tempRow = new GMbyte[width * mip->depth / 8];
	for (GMuint i = 0; i < rowsToSwap; ++i)
	{
		memcpy(tempRow, &mip->data[i * width * mip->depth / 8], width * mip->depth / 8);
		memcpy(&mip->data[i * width * mip->depth / 8], &mip->data[(height - i - 1) * width * mip->depth / 8], width * mip->depth / 8);
		memcpy(&mip->data[(height - i - 1) * width * mip->depth / 8], tempRow, width * mip->depth / 8);
	}

	if (tempRow)
		delete[] tempRow;
}
