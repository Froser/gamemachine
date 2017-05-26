#ifndef __IMAGE_BUFFER_H__
#define __IMAGE_BUFFER_H__
#include "common.h"
#include "image.h"
BEGIN_NS

GM_PRIVATE_OBJECT(ImageBuffer)
{
	GMuint width;
	GMuint height;
	GMbyte* buffer;
};

// 表示一个来自内存的图片
class ImageBuffer : public Image
{
	DECLARE_PRIVATE(ImageBuffer)

public:
	ImageBuffer(GMuint width, GMuint height, GMuint bufferSize, GMbyte* buffer);

private:
	void generateData();
};

END_NS
#endif