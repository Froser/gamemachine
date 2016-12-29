#ifndef __IMAGEREADER_PRIVATE_H__
#define __IMAGEREADER_PRIVATE_H__
#include "common.h"
#include <string>
#include "imagereader.h"
BEGIN_NS

class Image;
struct BitmapFile;
class ImageReader_BMP : public IImageReader
{
public:
	virtual bool load(const char* filename, OUT Image** img) override;
	virtual bool test(const char* filename) override;

private:
	void writeDataToImage(BitmapFile& bitmap, Image* img);
};

END_NS
#endif