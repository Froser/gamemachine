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
	WORD  bfType;
	DWORD bfSize;
	WORD  bfReserved1;
	WORD  bfReserved2;
	DWORD bfOffBits;
};

#if GM_WINDOWS
#include <poppack.h>
#endif

struct BitmapInfoHeader
{
	DWORD biSize;
	LONG  biWidth;
	LONG  biHeight;
	WORD  biPlanes;
	WORD  biBitCount;
	DWORD biCompression;
	DWORD biSizeImage;
	LONG  biXPelsPerMeter;
	LONG  biYPelsPerMeter;
	DWORD biClrUsed;
	DWORD biClrImportant;
};

struct PaletteEntry
{
	BYTE peRed;
	BYTE peGreen;
	BYTE peBlue;
	BYTE peFlags;
};

struct BitmapFile
{
	BitmapHeader bitmapHeader;
	BitmapInfoHeader bitmapInfoHeader;
	PaletteEntry palette[256];
	BYTE *buffer;
};

struct ImageRGB
{
	GMint r;
	GMint g;
	GMint b;
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

bool GMImageReader_BMP::load(const GMbyte* byte, GMuint size, OUT GMImage** img)
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

	GMuint bytePerPixel = bitmapFile.bitmapInfoHeader.biBitCount >> 3;
	GMuint skip = 4 - ((bitmapFile.bitmapInfoHeader.biWidth * bytePerPixel) & 3);
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

void GMImageReader_BMP::writeDataToImage(BitmapFile& bitmap, GMImage* img, GMuint size)
{
	GM_ASSERT(img);
	GMImage::Data& data = img->getData();
	data.target = GMImageTarget::Texture2D;
	data.mipLevels = 1;
	data.internalFormat = GMImageInternalFormat::RGBA8;
	data.format = GMImageFormat::BGRA;
	data.swizzle[0] = GL_RED;
	data.swizzle[1] = GL_GREEN;
	data.swizzle[2] = GL_BLUE;
	data.swizzle[3] = GL_ALPHA;
	data.type = GMImageDataType::UnsignedByte;
	data.mip[0].height = Fabs(bitmap.bitmapInfoHeader.biHeight);
	data.mip[0].width = Fabs(bitmap.bitmapInfoHeader.biWidth);
	// Buffer 移交给 Image 管理
	data.mip[0].data = bitmap.buffer;
	data.size = size;
}

void GMImageReader_BMP::flipVertically(GMbyte* data, GMuint width, GMuint height)
{
	const GMuint bytePerPixel = 4;
	GMuint rowsToSwap = height % 2 == 1 ? (height - 1) / 2 : height / 2;
	GMbyte* tempRow = new GMbyte[width * bytePerPixel];
	for (GMuint i = 0; i < rowsToSwap; ++i)
	{
		memcpy(tempRow, &data[i * width * bytePerPixel], width * bytePerPixel);
		memcpy(&data[i * width * bytePerPixel], &data[(height - i - 1) * width * bytePerPixel], width * bytePerPixel);
		memcpy(&data[(height - i - 1) * width * bytePerPixel], tempRow, width * bytePerPixel);
	}

	GM_delete_array(tempRow);
}