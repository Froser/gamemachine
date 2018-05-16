#ifndef __GMIMAGE_H__
#define __GMIMAGE_H__
#include <gmcommon.h>
BEGIN_NS

#define MAX_MIP_CNT 14
#define GM_IMAGE_DEFAULT_CHANNELS 4

enum class GMImageTarget
{
	Invalid,
	Texture1D,
	Texture1DArray,
	Texture2D,
	Texture2DArray,
	Texture3D,
	CubeMap,
	CubeMapArray,
};

enum class GMImageFormat
{
	RGB,
	RGBA,
	BGRA,
	RED,
};

enum class GMImageInternalFormat
{
	RGB8,
	RGBA8,
	RED8,
};

enum class GMImageDataType
{
	UnsignedByte,
	Float,
};

struct ImageMipData
{
	GMint width;
	GMint height;
	GMptrdiff mipStride;
	GMbyte* data = nullptr;
};

GM_PRIVATE_OBJECT(GMImage)
{
	GMImageTarget target = GMImageTarget::Invalid;
	GMImageInternalFormat internalFormat;
	GMImageFormat format;
	GMImageDataType type;
	GMint mipLevels;
	GMint slices = 1;
	GMptrdiff sliceStride = 0;
	ImageMipData mip[MAX_MIP_CNT];
	GMsize_t size = 0;
	GMuint channels = GM_IMAGE_DEFAULT_CHANNELS;
};

//! 表示一张或一系列图片。
/*!
  图片数据一般为32位形式保存，有RGBA共计4个通道。<BR>
  一个图片对象中，可能会存有多个MipMap，这通常出现在DDS等格式中。
*/
class GMImage : public GMObject
{
	DECLARE_PRIVATE(GMImage)

public:
	GMImage() = default;
	virtual ~GMImage();

public:
	inline Data& getData() { D(d); return *d; }
	const Data& getData() const;
	virtual void dispose();

public:
	inline GMint getWidth(GMint mipLevel = 0) const { return getData().mip[mipLevel].width; }
	inline GMint getHeight(GMint mipLevel = 0) const { return getData().mip[mipLevel].height; }
};

END_NS
#endif