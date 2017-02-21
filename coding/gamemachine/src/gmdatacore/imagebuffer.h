#ifndef __IMAGE_BUFFER_H__
#define __IMAGE_BUFFER_H__
#include "common.h"
#include "image.h"
BEGIN_NS

// 表示一个来自内存的图片
class ImageBuffer : public Image
{
public:
	ImageBuffer(GMuint width, GMuint height, GMuint bufferSize, GMbyte* buffer);

private:
	void generateData();

private:
	GMuint m_width;
	GMuint m_height;
	GMbyte* m_buffer;
};

END_NS
#endif