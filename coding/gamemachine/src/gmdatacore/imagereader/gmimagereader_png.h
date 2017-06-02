#ifndef __IMAGEREADER_PNG_H__
#define __IMAGEREADER_PNG_H__
#include "common.h"
#include "gmimagereader.h"
BEGIN_NS

struct PngData
{
	int width, height;
	int bit_depth;
	bool hasAlpha;
	unsigned char *rgba;
};

class Image;
class GMImageReader_PNG : public IImageReader
{
public:
	virtual bool load(const GMbyte* data, GMuint size, OUT Image** img) override;
	virtual bool test(const GMbyte* data) override;

private:
	void writeDataToImage(PngData& png, Image* img, GMuint size);
};

END_NS
#endif