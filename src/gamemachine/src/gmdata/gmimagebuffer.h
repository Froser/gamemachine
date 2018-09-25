#ifndef __IMAGE_BUFFER_H__
#define __IMAGE_BUFFER_H__
#include <gmcommon.h>
#include <gmimage.h>
#include <gmassets.h>
BEGIN_NS

GM_PRIVATE_OBJECT(GMImageBuffer)
{
	GMuint32 width;
	GMuint32 height;
	GMsize_t size = 0;
	GMbyte* buffer = nullptr;
};

// 表示一个来自内存的图片
class GMImageBuffer : public GMImage
{
	GM_DECLARE_PRIVATE_AND_BASE(GMImageBuffer, GMImage)

public:
	GMImageBuffer(GMImageFormat format, GMuint32 width, GMuint32 height, GMsize_t bufferSize, GMbyte* buffer);

private:
	void generateData();
};

// 表示一个mipmap level=0的Cubemap
class GMCubeMapBuffer : public GMImage
{
public:
	GMCubeMapBuffer(
		const GMImage& posX,
		const GMImage& negX,
		const GMImage& posY,
		const GMImage& negY,
		const GMImage& posZ,
		const GMImage& negZ);
};

END_NS
#endif