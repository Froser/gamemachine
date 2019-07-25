#include "stdafx.h"
#include "gmimagereader_tiff.h"
#include "foundation/utilities/tools.h"
#include <libtiff/tiffio.hxx>
#include <sstream>

namespace
{
	void deleter(void* buf)
	{
		_TIFFfree(buf);
	}
}

BEGIN_NS

bool GMImageReader_TIFF::load(const GMbyte* data, size_t size, OUT GMImage** image)
{
	GMImage* img = new GMImage();
	*image = img;
	GMImage::Data& imgData = img->getData();

	std::string binary((const char*)data, size);
	std::istringstream bs(binary);
	TIFF* tif = TIFFStreamOpen("MemTIFF", &bs);

	// Size
	GMuint32 width, height;
	GMshort bitsPerChannel, channelPerPixel;
	TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
	TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);
	TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bitsPerChannel);
	TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &channelPerPixel);
	
	imgData.target = GMImageTarget::Texture2D;
	imgData.mipLevels = 1;
	imgData.type = GMImageDataType::UnsignedByte;
	imgData.channels = channelPerPixel;
	imgData.mip[0].width = width;
	imgData.mip[0].height = height;
	imgData.deleter = deleter;
	if (imgData.channels == 1)
	{
		imgData.internalFormat = GMImageInternalFormat::RED8;
		imgData.format = GMImageFormat::RED;
	}
	else if (imgData.channels == 3)
	{
		imgData.internalFormat = GMImageInternalFormat::RGB8;
		imgData.format = GMImageFormat::RGB;
	}
	else if (imgData.channels == 4)
	{
		imgData.internalFormat = GMImageInternalFormat::RGBA8;
		imgData.format = GMImageFormat::RGBA;
	}
	else
	{
		gm_warning(gm_dbg_wrap("Channel number unsupport."));
		GM_ASSERT(false);
		imgData.internalFormat = GMImageInternalFormat::RGBA8;
		imgData.format = GMImageFormat::RGBA;
	}

	GMsize_t sizePerPixel = (channelPerPixel * bitsPerChannel / 8);
	GMsize_t bufferSize = width * height * sizePerPixel;
	imgData.mip[0].data = static_cast<GMbyte*>(_TIFFmalloc(bufferSize));
	imgData.size = bufferSize;
	TIFFReadRGBAImage(tif, width, height, reinterpret_cast<GMuint32*>(imgData.mip[0].data));
	TIFFClose(tif);
	return false;
}

bool GMImageReader_TIFF::test(const GMbyte* data, GMsize_t size)
{
	if (size >= 2)
	{
		return (data[0] == 0x4d && data[1] == 0x4d) || (data[0] == 0x49 && data[1] == 0x49);
	}

	return false;
}

END_NS