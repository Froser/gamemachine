#ifndef __GMIMAGE_P_H__
#define __GMIMAGE_P_H__
#include <gmcommon.h>
BEGIN_NS

GM_PRIVATE_OBJECT_UNALIGNED(GMImage)
{
	GMImageTarget target = GMImageTarget::Invalid;
	GMImageInternalFormat internalFormat;
	GMImageFormat format;
	GMImageDataType type;
	GMint32 mipLevels;
	GMint32 slices = 1;
	GMptrdiff sliceStride = 0;
	ImageMipData mip[MAX_MIP_CNT];
	GMsize_t size = 0;
	GMuint32 channels = GM_IMAGE_DEFAULT_CHANNELS;
	GMImageDataDeleter deleter = nullptr;
	bool generateMipmap = true;
};

END_NS
#endif