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

class ImageBMP : public Image
{
public:
	ImageBMP() {}

public:
	void dispose() override
	{
		if (m_bitmapFile.buffer)
			delete[] m_bitmapFile.buffer;
	}

	unsigned int loadTexture() override
	{
		return 0;
	}

	BitmapFile& getRawFile() { return m_bitmapFile; }

private:
	BitmapFile m_bitmapFile;
};

bool ImageReader_BMP::load(const char* filename, OUT Image** img)
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
		return true;
	}
	return false;
}