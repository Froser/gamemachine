#ifndef __IMAGEREADER_PRIVATE_H__
#define __IMAGEREADER_PRIVATE_H__
#include "common.h"
#include <string>
BEGIN_NS

class Image;
class ImageReaderPrivate
{
	friend class ImageReader;

private:
	bool load(const char* filename, Image* img);
};

END_NS
#endif