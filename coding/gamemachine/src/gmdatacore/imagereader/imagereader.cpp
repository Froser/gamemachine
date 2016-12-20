#include "stdafx.h"
#include "imagereader.h"
#include "imagereader_bmp.h"
#include "imagereader_dds.h"
#include "utilities/assert.h"

bool ImageReader::load(const char* filename, ImageType type, OUT Image** image)
{
	switch (type)
	{
	case BMP:
		return ImageReader_BMP::load(filename, image);
	case DDS:
		return ImageReader_DDS::load(filename, image);
	default:
		ASSERT(false);
		break;
	}
	return false;
}