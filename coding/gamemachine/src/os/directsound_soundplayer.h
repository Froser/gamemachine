#ifndef __DIRECTSOUND_SOUNDPLAYER_H__
#define __DIRECTSOUND_SOUNDPLAYER_H__
#include "common.h"
#include <mmsystem.h>
#include <dsound.h>
BEGIN_NS

#ifdef _WINDOWS

struct IWindow;
struct DirectSound_SoundPlayerPrivate
{
	LPDIRECTSOUND8 directSound;
	IWindow* window;
};

class DirectSound_SoundPlayer
{
	DEFINE_PRIVATE(DirectSound_SoundPlayer)

public:
	DirectSound_SoundPlayer(IWindow* window);
};

#endif

END_NS
#endif