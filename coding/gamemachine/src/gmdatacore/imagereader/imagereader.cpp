#include "stdafx.h"
#include "imagereader.h"

ImageReader::ImageReader()
{
}

bool ImageReader::load(const char* filename, Image* img)
{
	return dataRef().load(filename, img);
}
