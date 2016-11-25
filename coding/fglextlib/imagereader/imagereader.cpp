#include "stdafx.h"
#include "imagereader.h"

ImageReader::ImageReader(const char* filename)
{
	dataRef().setFilename(filename);
}

void ImageReader::load()
{
	dataRef().load();
}