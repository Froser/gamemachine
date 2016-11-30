#ifndef __IMAGEREADER_H__
#define __IMAGEREADER_H__
#include "common.h"
#include "imagereaderprivate.h"
BEGIN_NS

struct ImageRGB
{
	Fint r;
	Fint g;
	Fint b;
};

struct BitmapFile;
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

class ImageReader
{
	DEFINE_PRIVATE(ImageReader)

public:
	ImageReader();

public:
	bool load(const char* filename, Image* img);
};

END_NS
#endif