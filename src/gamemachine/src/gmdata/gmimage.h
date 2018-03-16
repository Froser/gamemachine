#ifndef __GMIMAGE_H__
#define __GMIMAGE_H__
#include <gmcommon.h>
BEGIN_NS

#define MAX_MIP_CNT 14

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
	BGR,
	BGRA,
};

enum class GMImageInternalFormat
{
	RGB8,
	RGBA8,
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
	GMint depth;
	GMsizeiptr mipStride;
	GMbyte* data = nullptr;
};

GM_PRIVATE_OBJECT(GMImage)
{
	GMImageTarget target = GMImageTarget::Invalid;
	GMImageInternalFormat internalFormat;
	GMImageFormat format;
	GMImageDataType type;
	GMuint swizzle[4];
	GMsizei mipLevels;
	GMsizei slices = 1;
	GMsizeiptr sliceStride = 0;
	GMsizeiptr totalDataSize;
	ImageMipData mip[MAX_MIP_CNT];
	GMuint size;
};

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
	void flipVertically(GMuint mipId);

public:
	inline GMint getWidth(GMint mipLevel = 0) const { return getData().mip[mipLevel].width; }
	inline GMint getHeight(GMint mipLevel = 0) const { return getData().mip[mipLevel].height; }
};

END_NS
#endif