#include "stdafx.h"
#include "gmimagereader_bmp.h"
#include <sstream>
#include "gmdatacore/image.h"
#include "foundation/utilities/utilities.h"

#if _WINDOWS
#include <wtypes.h>
#endif

#if _WINDOWS
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

#if _WINDOWS
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
	DEFAULT_CONSTRUCTOR(ImageBMP);

public:
	BitmapFile& getRawFile() { return m_bitmapFile; }

private:
	BitmapFile m_bitmapFile;
};

bool GMImageReader_BMP::load(const GMbyte* byte, GMuint size, OUT GMImage** img)
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

bool GMImageReader_BMP::test(const GMbyte* byte)
{
	const BitmapHeader* header = reinterpret_cast<const BitmapHeader*>(byte);
	return header->bfType == 19778;
}

void GMImageReader_BMP::writeDataToImage(BitmapFile& bitmap, GMImage* img, GMuint size)
{
	ASSERT(img);
	GMImage::Data& data = img->getData();
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