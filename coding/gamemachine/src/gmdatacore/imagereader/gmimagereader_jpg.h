#ifndef __IMAGEREADER_JPG_H__
#define __IMAGEREADER_JPG_H__
#include <gmcommon.h>
#include "gmimagereader.h"
BEGIN_NS

class GMImageReader_JPG : public IImageReader
{
	virtual bool load(const GMbyte* data, GMuint size, OUT GMImage** image) override;
	virtual bool test(const GMbyte* data) override;
};

END_NS
#endif