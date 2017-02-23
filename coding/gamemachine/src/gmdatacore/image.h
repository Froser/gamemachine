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
struct ImageData
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

	// (optional) texture params:
	GMuint minFilter;
	GMuint magFilter;
	GMuint wrapS;
	GMuint wrapT;
};

enum ImageType
{
	ImageType_AUTO,

	ImageType_Begin = 0,
	ImageType_BMP = ImageType_Begin,
	ImageType_DDS,
	ImageType_PNG,
	ImageType_TGA,
	ImageType_JPG,
	ImageType_End,
};

class Image
{
public:
	Image();
	virtual ~Image();

public:
	ImageData& getData();
	virtual void dispose();
	void flipVertically(GMuint mipId);

private:
	ImageData m_data;
};

END_NS
#endif