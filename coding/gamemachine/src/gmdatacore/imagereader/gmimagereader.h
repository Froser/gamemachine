#ifndef __IMAGEREADER_H__
#define __IMAGEREADER_H__
#include <gmcommon.h>
#include "gmdatacore/image.h"
BEGIN_NS

class GMImage;
struct IImageReader
{
	virtual ~IImageReader() {};
	virtual bool load(const GMbyte* data, GMuint size, OUT GMImage** image) = 0;
	virtual bool test(const GMbyte* data) = 0;
};

class GMImageReader
{
public:
	enum ImageType
	{
		ImageType_AUTO,

		ImageType_Begin = 0,
		ImageType_JPG = ImageType_Begin,
		ImageType_PNG,
		ImageType_TGA,
		ImageType_BMP,
		ImageType_DDS,
		ImageType_End,
	};

public:
	static bool load(const GMbyte* data, GMuint size, OUT GMImage** image);
	static bool load(const GMbyte* data, GMuint size, ImageType type, OUT GMImage** image);
	static IImageReader* getReader(ImageType type);

private:
	static ImageType test(const GMbyte* data);
};

END_NS
#endif
