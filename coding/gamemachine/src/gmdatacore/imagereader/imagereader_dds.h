#ifndef __IMAGEREADER_DDS_H
#define __IMAGEREADER_DDS_H
#include "common.h"
#include "imagereader.h"

BEGIN_NS
class Image;
class ImageReader_DDS : public IImageReader
{
public:
	virtual bool load(const char* filename, OUT Image** img) override;
	virtual bool test(const char* filename) override;
};
END_NS
#endif