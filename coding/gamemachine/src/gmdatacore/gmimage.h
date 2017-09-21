#ifndef __GMIMAGE_H__
#define __GMIMAGE_H__
#include <gmcommon.h>
BEGIN_NS

#define MAX_MIP_CNT 14

struct ImageMipData
{
	GMint width;
	GMint height;
	GMint depth;
	GLsizeiptr mipStride;
	GMbyte* data = nullptr;
};

// This is the main image data structure. It contains all the parameters needed
// to place texture data into a texture object using OpenGL.
GM_PRIVATE_OBJECT(GMImage)
{
	GMuint target;
	GMuint internalFormat;
	GMuint format;
	GMuint type;
	GMuint swizzle[4];
	GMsizei mipLevels;
	GMsizei slices;
	GMsizeiptr sliceStride;
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
	Data& getData();
	const Data& getData() const;
	virtual void dispose();
	void flipVertically(GMuint mipId);
};

END_NS
#endif