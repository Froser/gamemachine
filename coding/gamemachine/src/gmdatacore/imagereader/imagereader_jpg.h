#ifndef __IMAGEREADER_JPG_H__
#define __IMAGEREADER_JPG_H__
#include "common.h"
#include "imagereader.h"
BEGIN_NS

class ImageReader_JPG : public IImageReader
{
	virtual bool load(const char* filename, OUT Image** image) override;
	virtual bool test(const char* filename) override;
};

END_NS
#endif