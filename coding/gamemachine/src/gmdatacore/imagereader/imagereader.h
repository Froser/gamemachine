#ifndef __IMAGEREADER_H__
#define __IMAGEREADER_H__
#include "common.h"
#include "imagereaderprivate.h"
BEGIN_NS

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