#ifndef __IMAGE_BUFFER_H__
#define __IMAGE_BUFFER_H__
#include <gmcommon.h>
#include <gmimage.h>
#include <gmassets.h>
BEGIN_NS

GM_PRIVATE_OBJECT(GMImageBuffer)
{
	GMuint width;
	GMuint height;
	GMbyte* buffer;
};

// 表示一个来自内存的图片
class GMImageBuffer : public GMImage
{
	DECLARE_PRIVATE(GMImageBuffer)

public:
	GMImageBuffer(GMuint width, GMuint height, GMuint bufferSize, GMbyte* buffer);

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