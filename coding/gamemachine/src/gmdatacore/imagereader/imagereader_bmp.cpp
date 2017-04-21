#include "stdafx.h"
#include "imagereader_bmp.h"
#include "utilities/assert.h"
#include <fstream>
#include "gmdatacore/image.h"

#ifdef _WINDOWS
#include <wtypes.h>
#endif

#ifdef _WINDOWS
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

#ifdef _WINDOWS
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

class ImageBMP : public Image
{
public:
	ImageBMP() {}

public:
	BitmapFile& getRawFile() { return m_bitmapFile; }

private:
	BitmapFile m_bitmapFile;
};

bool ImageReader_BMP::load(const GMbyte* byte, OUT Image** img)
{
	ImageBMP* image;
	if (img)
	{
		*img = new ImageBMP();
		image = static_cast<ImageBMP*>(*img);
	}
	else
	{
		return false;
	}

	std::ifstream file;
	file.open(filename, std::ios::in | std::ios::binary);
	if (file.good())
	{
		BitmapFile& bitmapFile = image->getRawFile();

		file.read(reinterpret_cast<char*>(&bitmapFile.bitmapHeader), sizeof(BitmapHeader));
		file.read(reinterpret_cast<char*>(&bitmapFile.bitmapInfoHeader), sizeof(BitmapInfoHeader));

		long paletteLen = bitmapFile.bitmapHeader.bfOffBits - sizeof(BitmapHeader) - sizeof(BitmapInfoHeader);
		file.read(reinterpret_cast<char*>(&bitmapFile.palette), paletteLen);

		ASSERT(bitmapFile.bitmapInfoHeader.biHeight > 0
			&& bitmapFile.bitmapInfoHeader.biBitCount == 24
			&& bitmapFile.bitmapInfoHeader.biCompression == 0);

		long cnt = bitmapFile.bitmapInfoHeader.biWidth * bitmapFile.bitmapInfoHeader.biHeight * 3;
		bitmapFile.buffer = new BYTE[cnt];
		file.read(reinterpret_cast<char*>(bitmapFile.buffer), cnt);

		writeDataToImage(bitmapFile, image);

		file.close();
		return true;
	}
	file.close();
	return false;
}

bool ImageReader_BMP::test(const GMbyte* byte)
{
	BitmapHeader* header = reinterpret_cast<BitmapHeader*>(&header);
	return header->bfType == 19778;
}

void ImageReader_BMP::writeDataToImage(BitmapFile& bitmap, Image* img)
{
	ASSERT(img);
	ImageData& data = img->getData();
#ifdef USE_OPENGL
	data.target = GL_TEXTURE_2D;
	data.mipLevels = 1;
	data.internalFormat = GL_RGB16;
	data.format = GL_BGR;
	data.swizzle[0] = GL_RED;
	data.swizzle[1] = GL_GREEN;
	data.swizzle[2] = GL_BLUE;
	data.swizzle[3] = GL_ALPHA;
	data.type = GL_UNSIGNED_BYTE;
	data.mip[0].height = bitmap.bitmapInfoHeader.biHeight;
	data.mip[0].width = bitmap.bitmapInfoHeader.biWidth;
	// Buffer 移交给 Image 管理
	data.mip[0].data = bitmap.buffer;
#else
	ASSERT(false);
#endif
}