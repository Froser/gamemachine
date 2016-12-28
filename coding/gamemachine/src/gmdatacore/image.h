#ifndef __IMAGE_H__
#define __IMAGE_H__
#include "common.h"
BEGIN_NS

#define MAX_MIP_CNT 14

struct ImageMipData
{
	GLsizei width;
	GLsizei height;
	GLsizei depth;
	GLsizeiptr mipStride;
	GLvoid* data;
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
};

enum ImageType
{
	BMP,
	DDS,
};

class Image
{
public:
	Image();
	virtual ~Image();

public:
	ImageData& getData();
	virtual void dispose();

private:
	ImageData m_data;
};

END_NS
#endif