#ifndef __IMAGEREADER_PNG_H__
#define __IMAGEREADER_PNG_H__
#include "common.h"
#include "imagereader.h"
BEGIN_NS

struct PngData
{
	int width, height;
	int bit_depth;
	bool hasAlpha;
	unsigned char *rgba;
};

class Image;
class ImageReader_PNG : public IImageReader
{
public:
	virtual bool load(const GMbyte* data, OUT Image** img) override;
	virtual bool test(const GMbyte* data) override;

private:
	void writeDataToImage(PngData& png, Image* img);
};

END_NS
#endif