#include "stdafx.h"
#include <gmimage.h>

GMImage::~GMImage()
{
	dispose();
}

const GMImage::Data& GMImage::getData() const
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