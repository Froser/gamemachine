#ifndef __IMAGEREADER_BMP_H__
#define __IMAGEREADER_BMP_H__
#include "common.h"
#include <string>
#include "gmimagereader.h"
BEGIN_NS

class Image;
struct BitmapFile;
class GMImageReader_BMP : public IImageReader
{
public:
	virtual bool load(const GMbyte* byte, GMuint size, OUT Image** img) override;
	virtual bool test(const GMbyte* byte) override;

private:
	void writeDataToImage(BitmapFile& bitmap, Image* img, GMuint size);
};

END_NS
#endif