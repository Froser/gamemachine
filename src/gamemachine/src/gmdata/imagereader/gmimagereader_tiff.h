#ifndef __IMAGEREADER_TIFF_H__
#define __IMAGEREADER_TIFF_H__
#include <gmcommon.h>
#include "gmimagereader.h"
BEGIN_NS

class GMImageReader_TIFF : public IImageReader
{
	virtual bool load(const GMbyte* data, GMsize_t size, OUT GMImage** image) override;
	virtual bool test(const GMbyte* data, GMsize_t size) override;
};

END_NS
#endif