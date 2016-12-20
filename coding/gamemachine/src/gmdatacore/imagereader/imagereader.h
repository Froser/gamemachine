#ifndef __IMAGEREADER_H__
#define __IMAGEREADER_H__
#include "common.h"
#include "gmdatacore/image.h"

BEGIN_NS

class Image;
class ImageReader
{
public:
	static bool load(const char* filename, ImageType type, OUT Image** image);
};

END_NS
#endif