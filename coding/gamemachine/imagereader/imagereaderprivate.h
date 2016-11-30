#ifndef __IMAGEREADER_PRIVATE_H__
#define __IMAGEREADER_PRIVATE_H__
#include "common.h"
#include <string>
BEGIN_NS

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

class ImagePrivate
{
	friend class Image;
public:
	~ImagePrivate();

private:
	BitmapFile m_bitmapFile;
};

class ImageReaderPrivate
{
	friend class ImageReader;

private:
	bool load(const char* filename, Image* img);
};

END_NS
#endif