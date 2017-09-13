#ifndef __SOUNDREADER_MP3_H__
#define __SOUNDREADER_MP3_H__
#include <gmmcommon.h>
#include <gamemachine.h>
#include "gmmaudioreader.h"

BEGIN_MEDIA_NS

class GMMAudioReader_MP3 : public gm::GMObject, public IAudioFormatReader
{
public:
	virtual bool load(gm::GMBuffer& buffer, OUT gm::IAudioFile** f) override;
	virtual bool test(const gm::GMBuffer& buffer) override;
};

END_MEDIA_NS
#endif