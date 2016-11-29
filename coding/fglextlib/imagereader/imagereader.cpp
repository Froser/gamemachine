#include "stdafx.h"
#include "imagereader.h"

Image::Image()
{

}

BitmapFile& Image::getRawFile()
{
	return dataRef().m_bitmapFile;
}

ImageReader::ImageReader()
{
}

bool ImageReader::load(const char* filename, Image* img)
{
	return dataRef().load(filename, img);
}

ImageRGB Image::getRGB(long x, long y)
{
	BYTE* buf = dataRef().m_bitmapFile.buffer;
	LONG width = dataRef().m_bitmapFile.bitmapInfoHeader.biWidth;
	LONG offset = (x + y * width) * 3;
	BYTE* d = buf + offset;
	ImageRGB ret = { *(d + 2), *(d + 1), *d};
	return ret;
}

FByte* Image::asTexture()
{
	return (FByte*) getRawFile().buffer;
}

long Image::getWidth()
{
	BitmapInfoHeader& h = dataRef().m_bitmapFile.bitmapInfoHeader;
	return h.biWidth;
}

long Image::getHeight()
{
	BitmapInfoHeader& h = dataRef().m_bitmapFile.bitmapInfoHeader;
	return h.biHeight;
}