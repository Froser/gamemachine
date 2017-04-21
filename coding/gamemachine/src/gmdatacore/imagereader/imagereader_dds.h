#ifndef __IMAGEREADER_DDS_H__
#define __IMAGEREADER_DDS_H__
#include "common.h"
#include "imagereader.h"

BEGIN_NS
class Image;
class ImageReader_DDS : public IImageReader
{
public:
	virtual bool load(const GMbyte* data, OUT Image** img) override;
	virtual bool test(const GMbyte* data) override;
};
END_NS
#endif