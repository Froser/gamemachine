#ifndef __GMM_H__
#define __GMM_H__
#include <gmmcommon.h>

#include <gmmaudioplayer.h>
#include <gmmaudioreader.h>

BEGIN_MEDIA_NS

class GMMFactory
{
public:
	static gm::IAudioReader* getAudioReader();
	static gm::IAudioPlayer* getAudioPlayer();
};

END_MEDIA_NS

#endif