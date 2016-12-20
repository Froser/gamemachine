#ifndef __IMAGEREADER_DDS_H
#define __IMAGEREADER_DDS_H
#include "common.h"
BEGIN_NS
class Image;
class ImageReader_DDS
{
public:
	static bool load(const char* filename, OUT Image** img);
};
END_NS
#endif