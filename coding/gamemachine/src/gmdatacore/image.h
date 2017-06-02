#ifndef __IMAGE_H__
#define __IMAGE_H__
#include "common.h"
BEGIN_NS

#define MAX_MIP_CNT 14

struct ImageMipData
{
	GMint width;
	GMint height;
	GMint depth;
	GLsizeiptr mipStride;
	GMbyte* data;
};

// This is the main image data structure. It contains all the parameters needed
// to place texture data into a texture object using OpenGL.
GM_PRIVATE_OBJECT(Image)
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

class Image : public GMObject
{
	DECLARE_PRIVATE(Image)

public:
	Image();
	virtual ~Image();

public:
	Data& getData();
	virtual void dispose();
	void flipVertically(GMuint mipId);
};

END_NS
#endif