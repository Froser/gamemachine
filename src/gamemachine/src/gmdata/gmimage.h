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
	ARGB,
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
	GMint32 width;
	GMint32 height;
	GMptrdiff mipStride;
	GMbyte* data = nullptr;
};

typedef void(*GMImageDataDeleter)(void*);

GM_PRIVATE_CLASS(GMImage);
//! 表示一张或一系列图片。
/*!
  图片数据一般为32位形式保存，有RGBA共计4个通道。<BR>
  一个图片对象中，可能会存有多个MipMap，这通常出现在DDS等格式中。
*/
class GM_EXPORT GMImage : public IDestroyObject
{
	GM_DECLARE_PRIVATE(GMImage)
	GM_DISABLE_COPY_ASSIGN(GMImage)

public:
	GMImage();
	~GMImage();

public:
	Data& getData();
	const Data& getData() const;

	virtual void dispose();

public:
	GMint32 getWidth(GMint32 mipLevel = 0) const;
	GMint32 getHeight(GMint32 mipLevel = 0) const;
	void setGenerateMipmap(bool b);
};

END_NS
#endif