#ifndef __SOUNDREADER_MP3_H__
#define __SOUNDREADER_MP3_H__
#include <gmcommon.h>
#include "gmsoundreader.h"
#include "foundation/vector.h"
BEGIN_NS

GM_PRIVATE_OBJECT(GMSoundReader_MP3)
{
};

class GMSoundReader_MP3 : public GMObject, public ISoundReader
{
	DECLARE_PRIVATE(GMSoundReader_MP3)

public:
	virtual bool load(GMBuffer& buffer, OUT ISoundFile** sf) override;
	virtual bool test(const GMBuffer& buffer) override;
};

END_NS
#endif
