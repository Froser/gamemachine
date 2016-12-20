#ifndef __IMAGEREADER_PRIVATE_H__
#define __IMAGEREADER_PRIVATE_H__
#include "common.h"
#include <string>
BEGIN_NS

class Image;
class ImageReader_BMP
{
public:
	static bool load(const char* filename, OUT Image** img);
};

END_NS
#endif