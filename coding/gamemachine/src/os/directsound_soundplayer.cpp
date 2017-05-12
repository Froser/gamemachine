#include "stdafx.h"
#include "directsound_soundplayer.h"
#include "utilities/assert.h"
#include "gmengine/controllers/gamemachine.h"
#include "gmdatacore/soundreader/soundreader.h"

#ifdef _WINDOWS

#ifdef _MSC_VER
#pragma comment(lib,"dsound.lib")
#endif

DirectSound_SoundPlayer::DirectSound_SoundPlayer(IWindow* window)
{
	D(d);
	d.window = window;
	d.playing = false;

	HRESULT hr = DirectSoundCreate8(NULL, &d.cpDirectSound, NULL);
	ASSERT(SUCCEEDED(hr));

	hr = d.cpDirectSound->SetCooperativeLevel(d.window->hwnd(), DSSCL_PRIORITY);
	ASSERT(SUCCEEDED(hr));
}

DirectSound_SoundPlayer::~DirectSound_SoundPlayer()
{
	D(d);
	d.playing = false;
}

void DirectSound_SoundPlayer::play(ISoundFile* sf, PlayOptions options)
{
	D(d);
	loadSound(sf, options);
	d.playing = true;
	d.cpDirectSoundBuffer->Play(0, 0, DSBPLAY_LOOPING);
}

void DirectSound_SoundPlayer::stop()
{
	D(d);
	d.cpDirectSoundBuffer->Stop();
	d.playing = false;
}

void DirectSound_SoundPlayer::loadSound(ISoundFile* sf, PlayOptions options)
{
	D(d);
	DSBUFFERDESC dsbd = { 0 };
	dsbd.dwSize = sizeof(DSBUFFERDESC);
	dsbd.dwFlags = DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLFX | DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_GETCURRENTPOSITION2;
	dsbd.dwBufferBytes = sf->getData()->dwSize;
	dsbd.lpwfxFormat = (LPWAVEFORMATEX)sf->getWaveFormat();

	ComPtr<IDirectSoundBuffer> cpBuffer;
	HRESULT hr;
	if (FAILED(hr = d.cpDirectSound->CreateSoundBuffer(&dsbd, &cpBuffer, NULL)))
	{
		gm_error("create sound buffer error.");
		return;
	}

	if (FAILED(hr = cpBuffer->QueryInterface(IID_IDirectSoundBuffer8, (LPVOID*)&d.cpDirectSoundBuffer)))
	{
		gm_error("QueryInterface to IDirectSoundBuffer8 error");
		return;
	}

	LPVOID lpLockBuf;
	DWORD len;
	d.cpDirectSoundBuffer->Lock(0, 0, &lpLockBuf, &len, NULL, NULL, DSBLOCK_ENTIREBUFFER);
	memcpy(lpLockBuf, sf->getData()->data, len);
	d.cpDirectSoundBuffer->Unlock(lpLockBuf, len, NULL, NULL);
	d.cpDirectSoundBuffer->SetCurrentPosition(0);
}

#endif