#ifndef __GMM_H__
#define __GMM_H__
#include <gmmcommon.h>

#if _WINDOWS
#include "../windows/audio/gmmaudioreader.h"
#endif

#include <gmmaudioplayer.h>

BEGIN_MEDIA_NS

class GMMFactory
{
public:
	static void createAudioReader(OUT gm::IAudioReader** reader);
	static void createAudioPlayer(OUT gm::IAudioPlayer** player);
};

END_MEDIA_NS

#endif