#ifndef __DIRECTSOUND_SOUNDPLAYER_H__
#define __DIRECTSOUND_SOUNDPLAYER_H__
#include "common.h"
#include <mmsystem.h>
#include <dsound.h>
BEGIN_NS

#ifndef _WINDOWS
#error need windows env
#endif

struct DirectSound_SoundPlayerPrivate
{
	LPDIRECTSOUND8 directSound;
};

class DirectSound_SoundPlayer
{
	DEFINE_PRIVATE(DirectSound_SoundPlayer)

public:
	DirectSound_SoundPlayer();
};

END_NS
#endif