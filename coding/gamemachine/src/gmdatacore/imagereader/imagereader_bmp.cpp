#include "stdafx.h"
#include "imagereader_bmp.h"
#include <sstream>
#include "gmdatacore/image.h"
#include "foundation/utilities/utilities.h"

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

bool ImageReader_BMP::load(const GMbyte* byte, GMuint size, OUT Image** img)
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

	MemoryStream ms(byte, size);
	BitmapFile& bitmapFile = image->getRawFile();

	ms.read(reinterpret_cast<GMbyte*>(&bitmapFile.bitmapHeader), sizeof(BitmapHeader));
	ms.read(reinterpret_cast<GMbyte*>(&bitmapFile.bitmapInfoHeader), sizeof(BitmapInfoHeader));

	long paletteLen = bitmapFile.bitmapHeader.bfOffBits - sizeof(BitmapHeader)-sizeof(BitmapInfoHeader);
	ms.read(reinterpret_cast<GMbyte*>(&bitmapFile.palette), paletteLen);

	ASSERT(bitmapFile.bitmapInfoHeader.biHeight > 0
		&& bitmapFile.bitmapInfoHeader.biBitCount == 24
		&& bitmapFile.bitmapInfoHeader.biCompression == 0);

	long cnt = bitmapFile.bitmapInfoHeader.biWidth * bitmapFile.bitmapInfoHeader.biHeight * 3;
	bitmapFile.buffer = new BYTE[cnt];
	ms.read(reinterpret_cast<GMbyte*>(bitmapFile.buffer), cnt);

	writeDataToImage(bitmapFile, image, cnt);

	return true;
}

bool ImageReader_BMP::test(const GMbyte* byte)
{
	const BitmapHeader* header = reinterpret_cast<const BitmapHeader*>(byte);
	return header->bfType == 19778;
}

void ImageReader_BMP::writeDataToImage(BitmapFile& bitmap, Image* img, GMuint size)
{
	ASSERT(img);
	Image::Data& data = img->getData();
#if USE_OPENGL
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
	data.size = size;
#else
	ASSERT(false);
#endif
}