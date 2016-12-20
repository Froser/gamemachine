#ifndef __IMAGE_H__
#define __IMAGE_H__
#include "common.h"
BEGIN_NS

enum ImageType
{
	BMP,
	DDS,
};

class Image
{
public:
	virtual ~Image();
	virtual unsigned int loadTexture() = 0;

protected:
	virtual void dispose();
};

END_NS
#endif