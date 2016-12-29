#ifndef __IMAGEREADER_H__
#define __IMAGEREADER_H__
#include "common.h"
#include "gmdatacore/image.h"
BEGIN_NS

class Image;
struct IImageReader
{
	virtual bool load(const char* filename, OUT Image** image) = 0;
	virtual bool test(const char* filename) = 0;
};

class ImageReader
{
public:
	static bool load(const char* filename, OUT Image** image);
	static bool load(const char* filename, ImageType type, OUT Image** image);
	static IImageReader* getReader(ImageType type);

private:
	static ImageType test(const char* filename);
};

END_NS
#endif