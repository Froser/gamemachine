#include "stdafx.h"
#include <gmimage.h>

BEGIN_NS

GMImage::GMImage()
{
	GM_CREATE_DATA(GMImage);
}

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
		if (!d->deleter)
			delete[] reinterpret_cast<GMbyte *>(d->mip[0].data);
		else
			d->deleter(d->mip[0].data);
		d->mip[0].data = nullptr;
	}
}

void GMImage::setGenerateMipmap(bool b)
{
	getData().generateMipmap = b;
}

GMint32 GMImage::getHeight(GMint32 mipLevel /*= 0*/) const
{
	return getData().mip[mipLevel].height;
}

GMint32 GMImage::getWidth(GMint32 mipLevel /*= 0*/) const
{
	return getData().mip[mipLevel].width;
}

END_NS
