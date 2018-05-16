#ifndef __IMAGEREADER_PNG_H__
#define __IMAGEREADER_PNG_H__
#include <gmcommon.h>
#include "gmimagereader.h"
BEGIN_NS

struct PngData
{
	GMint width, height;
	GMint bit_depth;
	bool hasAlpha;
	GMint channels;
	GMbyte *rgba = nullptr;
};

class GMImage;
class GMImageReader_PNG : public IImageReader
{
public:
	virtual bool load(const GMbyte* data, GMsize_t size, OUT GMImage** img) override;
	virtual bool test(const GMbyte* data) override;

private:
	void writeDataToImage(PngData& png, GMImage* img, GMsize_t size);
};

END_NS
#endif