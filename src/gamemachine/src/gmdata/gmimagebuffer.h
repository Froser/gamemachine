#ifndef __IMAGE_BUFFER_H__
#define __IMAGE_BUFFER_H__
#include <gmcommon.h>
#include <gmimage.h>
#include <gmassets.h>
BEGIN_NS

// 表示一个来自内存的图片
GM_PRIVATE_CLASS(GMImageBuffer);
class GMImageBuffer : public GMImage
{
	GM_DECLARE_PRIVATE(GMImageBuffer)
	GM_DECLARE_BASE(GMImage)

public:
	GMImageBuffer(GMImageFormat format, GMuint32 width, GMuint32 height, GMsize_t bufferSize, GMbyte* buffer);
	~GMImageBuffer();

private:
	void generateData();
};

// 表示一个mipmap level=0的Cubemap
class GM_EXPORT GMCubeMapBuffer : public GMImage
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