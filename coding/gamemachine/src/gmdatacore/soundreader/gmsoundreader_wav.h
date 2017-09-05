#ifndef __SOUNDREADER_WAV_H__
#define __SOUNDREADER_WAV_H__
#include <gmcommon.h>
#include "gmsoundreader.h"
BEGIN_NS

class GMSoundReader_Wav : public GMObject, public ISoundReader
{
public:
	virtual bool load(GMBuffer& buffer, OUT ISoundFile** sf) override;
	virtual bool test(const GMBuffer& buffer) override;
};

END_NS
#endif