#ifndef __SOUNDREADER_WAV_H__
#define __SOUNDREADER_WAV_H__
#include <gmmcommon.h>
#include <gminterfaces.h>
#include <gmobject.h>
#include "gmmaudioreader.h"

BEGIN_MEDIA_NS

class GMMAudioReader_Wav : public gm::GMObject, public IAudioFormatReader
{
public:
	virtual bool load(gm::GMBuffer& buffer, OUT gm::IAudioFile** f) override;
	virtual bool test(const gm::GMBuffer& buffer) override;
};

END_MEDIA_NS
#endif