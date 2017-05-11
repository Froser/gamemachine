#include "stdafx.h"
#include "directsound_soundplayer.h"
#include "utilities/assert.h"
#include "gmengine/controllers/gamemachine.h"

#ifdef _WINDOWS

DirectSound_SoundPlayer::DirectSound_SoundPlayer(IWindow* window)
{
	D(d);
	d.window = window;

	HRESULT hr = DirectSoundCreate8(NULL, &d.directSound, NULL);
	ASSERT(SUCCEEDED(hr));

	hr = d.directSound->SetCooperativeLevel(d.window->hwnd(), DSSCL_PRIORITY);
	ASSERT(SUCCEEDED(hr));

	//d.directSound->CreateSoundBuffer()
}

#endif