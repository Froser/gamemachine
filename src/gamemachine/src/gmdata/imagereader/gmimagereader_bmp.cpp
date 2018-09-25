#include "stdafx.h"
#include "gmimagereader_bmp.h"
#include <sstream>
#include <gmimage.h>
#include "foundation/utilities/tools.h"
#include <GL/glew.h>

#if GM_WINDOWS
#include <wtypes.h>
#endif

#if GM_WINDOWS
#include <pshpack2.h>
#endif

BEGIN_NS

struct BitmapHeader
{
	GMWord bfType;
	GMDWord  bfSize;
	GMWord   bfReserved1;
	GMWord   bfReserved2;
	GMDWord  bfOffBits;
};

#if GM_WINDOWS
#include <poppack.h>
#endif

struct BitmapInfoHeader
{
	GMDWord biSize;
	GMlong  biWidth;
	GMlong  biHeight;
	GMWord  biPlanes;
	GMWord  biBitCount;
	GMDWord biCompression;
	GMDWord biSizeImage;
	GMlong  biXPelsPerMeter;
	GMlong  biYPelsPerMeter;
	GMDWord biClrUsed;
	GMDWord biClrImportant;
};

struct PaletteEntry
{
	GMbyte peRed;
	GMbyte peGreen;
	GMbyte peBlue;
	GMbyte peFlags;
};

struct BitmapFile
{
	BitmapHeader bitmapHeader;
	BitmapInfoHeader bitmapInfoHeader;
	PaletteEntry palette[256];
	GMbyte *buffer;
};

struct ImageRGB
{
	GMint32 r;
	GMint32 g;
	GMint32 b;
};
END_NS

class ImageBMP : public GMImage
{
public:
	ImageBMP() = default;

public:
	BitmapFile& getRawFile() { return m_bitmapFile; }

private:
	BitmapFile m_bitmapFile;
};

bool GMImageReader_BMP::load(const GMbyte* byte, GMsize_t size, OUT GMImage** img)
{
	ImageBMP* image = nullptr;
	if (img)
	{
		*img = new ImageBMP();
		image = static_cast<ImageBMP*>(*img);
	}
	else
	{
		return false;
	}

	GMMemoryStream ms(byte, size);
	BitmapFile& bitmapFile = image->getRawFile();

	ms.read(reinterpret_cast<GMbyte*>(&bitmapFile.bitmapHeader), sizeof(BitmapHeader));
	ms.read(reinterpret_cast<GMbyte*>(&bitmapFile.bitmapInfoHeader), sizeof(BitmapInfoHeader));

	long paletteLen = bitmapFile.bitmapHeader.bfOffBits - sizeof(BitmapHeader)-sizeof(BitmapInfoHeader);
	ms.read(reinterpret_cast<GMbyte*>(&bitmapFile.palette), paletteLen);

	GMuint32 bytePerPixel = bitmapFile.bitmapInfoHeader.biBitCount >> 3;
	GMuint32 skip = 4 - ((bitmapFile.bitmapInfoHeader.biWidth * bytePerPixel) & 3);
	if (bitmapFile.bitmapInfoHeader.biBitCount == 24
		&& bitmapFile.bitmapInfoHeader.biCompression == 0)
	{
		GMlong pixels = bitmapFile.bitmapInfoHeader.biWidth * bitmapFile.bitmapInfoHeader.biHeight;
		GMlong size = pixels * GMImageReader::DefaultChannels;
		bitmapFile.buffer = new GMbyte[size];
		GMbyte* dataPtr = bitmapFile.buffer;

		for (GMlong h = 0; h < bitmapFile.bitmapInfoHeader.biHeight; ++h)
		{
			for (GMlong w = 0; w < bitmapFile.bitmapInfoHeader.biWidth; ++w)
			{
				ms.read(dataPtr, 3);
				dataPtr += 4;
				*(dataPtr - 1) = 0xFF;
			}
			ms.read(nullptr, skip);
		}
		GM_ASSERT(dataPtr - bitmapFile.buffer == size);

		if (bitmapFile.bitmapInfoHeader.biHeight > 0)
		{
			flipVertically(
				bitmapFile.buffer,
				bitmapFile.bitmapInfoHeader.biWidth,
				bitmapFile.bitmapInfoHeader.biHeight
			);
		}

		writeDataToImage(bitmapFile, image, size);
		return true;
	}
	else
	{
		GM_ASSERT(false);
		return false;
	}
}

bool GMImageReader_BMP::test(const GMbyte* byte)
{
	const BitmapHeader* header = reinterpret_cast<const BitmapHeader*>(byte);
	return header && header->bfType == 19778;
}

void GMImageReader_BMP::writeDataToImage(BitmapFile& bitmap, GMImage* img, GMuint32 size)
{
	GM_ASSERT(img);
	GMImage::Data& data = img->getData();
	data.target = GMImageTarget::Texture2D;
	data.mipLevels = 1;
	data.internalFormat = GMImageInternalFormat::RGBA8;
	data.format = GMImageFormat::BGRA;
	data.type = GMImageDataType::UnsignedByte;
	data.mip[0].height = Fabs(bitmap.bitmapInfoHeader.biHeight);
	data.mip[0].width = Fabs(bitmap.bitmapInfoHeader.biWidth);
	// Buffer 移交给 Image 管理
	data.mip[0].data = bitmap.buffer;
	data.size = size;
}

void GMImageReader_BMP::flipVertically(GMbyte* data, GMuint32 width, GMuint32 height)
{
	const GMuint32 bytePerPixel = 4;
	GMuint32 rowsToSwap = height % 2 == 1 ? (height - 1) / 2 : height / 2;
	GMbyte* tempRow = new GMbyte[width * bytePerPixel];
	for (GMuint32 i = 0; i < rowsToSwap; ++i)
	{
		memcpy(tempRow, &data[i * width * bytePerPixel], width * bytePerPixel);
		memcpy(&data[i * width * bytePerPixel], &data[(height - i - 1) * width * bytePerPixel], width * bytePerPixel);
		memcpy(&data[(height - i - 1) * width * bytePerPixel], tempRow, width * bytePerPixel);
	}

	GM_delete_array(tempRow);
}