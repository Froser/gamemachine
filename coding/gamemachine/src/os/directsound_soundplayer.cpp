#include "stdafx.h"
#include "directsound_soundplayer.h"
#include "utilities/assert.h"
#include "gmengine/controllers/gamemachine.h"
#include "gmdatacore/soundreader/soundreader.h"

#ifdef _WINDOWS

#ifdef _MSC_VER
#pragma comment(lib,"dsound.lib")
#endif

static const GMlong AUDIO_BUF_CNT = 4;
static const GMlong BUF_NOTIFY_SZ = 1920;
static DSBPOSITIONNOTIFY posNotify[AUDIO_BUF_CNT];
static HANDLE events[AUDIO_BUF_CNT];

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

void DirectSound_SoundPlayer::play(ISoundFile* sf)
{
	D(d);
	loadSound(sf);
	d.playing = true;
	//DWORD res;
	//while (d.playing)
	//{
	//	res = WaitForMultipleObjects(AUDIO_BUF_CNT, events, FALSE, INFINITE);
	//	if (res > WAIT_OBJECT_0)
	//		processBuffer();
	//}
}

void DirectSound_SoundPlayer::stop()
{
	D(d);
	d.cpDirectSoundBuffer->Stop();
	d.playing = false;
}

void DirectSound_SoundPlayer::loadSound(ISoundFile* sf)
{
	D(d);
	DSBUFFERDESC dsbd = { 0 };
	dsbd.dwSize = sizeof(DSBUFFERDESC);
	dsbd.dwFlags = DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLFX | DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_GETCURRENTPOSITION2;
	dsbd.dwBufferBytes = sf->getData()->dwSize;//AUDIO_BUF_CNT * BUF_NOTIFY_SZ;
	dsbd.lpwfxFormat = (LPWAVEFORMATEX)sf->getWaveFormat();

	ComPtr<IDirectSoundBuffer> cpBuffer;
	HRESULT hr;
	if (FAILED(hr = d.cpDirectSound->CreateSoundBuffer(&dsbd, &cpBuffer, NULL)))
		return;

	if (FAILED(hr = cpBuffer->QueryInterface(IID_IDirectSoundBuffer8, (LPVOID*)&d.cpDirectSoundBuffer)))
		return;

	LPVOID lpLockBuf;
	DWORD len;
	d.cpDirectSoundBuffer->Lock(0, 0, &lpLockBuf, &len, NULL, NULL, DSBLOCK_ENTIREBUFFER);
	memcpy(lpLockBuf, sf->getData()->data, len);
	d.cpDirectSoundBuffer->Unlock(lpLockBuf, len, NULL, NULL);
	d.cpDirectSoundBuffer->SetCurrentPosition(0);
	d.cpDirectSoundBuffer->Play(0, 0, DSBPLAY_LOOPING);

	//ComPtr<IDirectSoundNotify8> cpNotify;
	//if (FAILED(hr = cpBuffer->QueryInterface(IID_IDirectSoundNotify, (LPVOID*)&cpNotify)))
	//	return;
	//
	//for (GMint i = 0; i < AUDIO_BUF_CNT; i++)
	//{
	//	posNotify[i].dwOffset = i* BUF_NOTIFY_SZ;
	//	posNotify[i].hEventNotify = events[i];
	//}
	//cpNotify->SetNotificationPositions(AUDIO_BUF_CNT, posNotify);
}

void DirectSound_SoundPlayer::processBuffer()
{

}

#endif