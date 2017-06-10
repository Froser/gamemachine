#ifndef __IMAGEREADER_DDS_H__
#define __IMAGEREADER_DDS_H__
#include "common.h"
#include "gmimagereader.h"

BEGIN_NS
class GMImage;
class GMImageReader_DDS : public IImageReader
{
public:
	virtual bool load(const GMbyte* data, GMuint size, OUT GMImage** img) override;
	virtual bool test(const GMbyte* data) override;
};
END_NS
#endif