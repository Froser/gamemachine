#ifndef __IMAGEREADER_H__
#define __IMAGEREADER_H__
#include <gmcommon.h>
#include <gmimage.h>
BEGIN_NS

class GMImage;
struct IImageReader
{
	virtual ~IImageReader() {};
	virtual bool load(const GMbyte* data, GMsize_t size, OUT GMImage** image) = 0;
	virtual bool test(const GMbyte* data, GMsize_t size) = 0;
};

class GM_EXPORT GMImageReader
{
public:
	enum ImageType
	{
		ImageType_AUTO,

		ImageType_Begin = 0,
		ImageType_JPG = ImageType_Begin,
		ImageType_PNG,
		ImageType_BMP,
		ImageType_TIFF,
		ImageType_TGA, // TGA头无magic number，放在最后解析
		ImageType_End,
	};

	enum
	{
		DefaultChannels = GM_IMAGE_DEFAULT_CHANNELS
	};

public:
	static bool load(const GMbyte* data, GMsize_t size, OUT GMImage** image);
	static bool load(const GMbyte* data, GMsize_t size, ImageType type, OUT GMImage** image);
	static IImageReader* getReader(ImageType type);
};

END_NS
#endif
