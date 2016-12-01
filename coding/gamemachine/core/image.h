#ifndef __IMAGE_H__
#define __IMAGE_H__
#include "common.h"
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

struct ImageRGB
{
	GMint r;
	GMint g;
	GMint b;
};

class ImagePrivate
{
	friend class Image;
public:
	ImagePrivate();
	~ImagePrivate();

private:
	BitmapFile m_bitmapFile;
};

class Image
{
	DEFINE_PRIVATE(Image)

public:
	Image();

public:
	BitmapFile& getRawFile();
	ImageRGB getRGB(long x, long y);
	long getWidth();
	long getHeight();
	FByte* asTexture();
};

END_NS
#endif