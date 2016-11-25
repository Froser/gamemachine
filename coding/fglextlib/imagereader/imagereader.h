#ifndef __IMAGEREADER_H__
#include "common.h"
#include "imagereaderprivate.h"
BEGIN_NS

class ImageReader
{
	DEFINE_PRIVATE(ImageReader)

public:
	ImageReader(const char* filename);

public:
	void load();
};

END_NS
#endif