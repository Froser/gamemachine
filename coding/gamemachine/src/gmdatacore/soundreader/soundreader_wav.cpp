#include "stdafx.h"
#include "soundreader_wav.h"
#include "utilities/utilities.h"
#include "gmdatacore/gamepackage.h"
#include "soundreader.h"

#ifdef _WINDOWS
#include <dsound.h>
#include "os/directsound_sounddevice.h"
#endif

struct GM_WAVERIFF
{
	GMbyte chID[4];
	DWORD dwSize;
	GMbyte chData[4];
};

struct GM_WAVEFORMATEX
{
	GMbyte chID[4];
	DWORD dwSize;
	WAVEFORMATEX waveFormatEx;
};

struct GM_WAVEFACT
{
	GMbyte chID[4];
	DWORD dwSize;
	DWORD data;
};

#ifdef _WINDOWS
struct WavSoundFilePrivate
{
	ComPtr<IDirectSoundBuffer8> cpDirectSoundBuffer;
	bool playing;
};

class WavSoundFile : public SoundFile
{
	DEFINE_PRIVATE(WavSoundFile)

	typedef SoundFile Base;

public:
	WavSoundFile(const WAVEFORMATEX& fmt, AUTORELEASE WaveData* waveData)
		: Base(fmt, waveData)
	{
		D(d);
		d.playing = false;
	}

public:
	virtual void play() override
	{
		D(d);
		loadSound();
		d.playing = true;
		HRESULT hr = d.cpDirectSoundBuffer->Play(0, 0, DSBPLAY_LOOPING);
		ASSERT(SUCCEEDED(hr));
	}

	virtual void stop() override
	{
		D(d);
		d.cpDirectSoundBuffer->Stop();
		d.playing = false;
	}

private:
	void loadSound()
	{
		D(d);
		DSBUFFERDESC dsbd = { 0 };
		dsbd.dwSize = sizeof(DSBUFFERDESC);
		dsbd.dwFlags = DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLFX | DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_GETCURRENTPOSITION2;
		dsbd.dwBufferBytes = getData()->dwSize;
		dsbd.lpwfxFormat = (LPWAVEFORMATEX)getWaveFormat();

		ComPtr<IDirectSoundBuffer> cpBuffer;
		HRESULT hr;
		if (FAILED(hr = SoundPlayerDevice::getInstance()->CreateSoundBuffer(&dsbd, &cpBuffer, NULL)))
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
		memcpy(lpLockBuf, getData()->data, len);
		d.cpDirectSoundBuffer->Unlock(lpLockBuf, len, NULL, NULL);
		d.cpDirectSoundBuffer->SetCurrentPosition(0);
	}
};
#endif

bool SoundReader_Wav::load(GamePackageBuffer& buffer, OUT ISoundFile** sf)
{
#ifdef _WINDOWS
	// 假设都是小端模式
	GM_WAVERIFF riff;
	GM_WAVEFORMATEX format;
	GM_WAVEFACT fact;

	MemoryStream ms(buffer.buffer, buffer.size);
	ms.read((GMbyte*)&riff, sizeof(GM_WAVERIFF));
	ms.read((GMbyte*)&format.chID, sizeof(format.chID));
	ms.read((GMbyte*)&format.dwSize, sizeof(format.dwSize));

	GMuint pos = ms.tell();
	ms.read((GMbyte*)&format.waveFormatEx, sizeof(format.waveFormatEx));
	ms.seek(pos + format.dwSize);

	char maybeFact[6];
	ms.peek((GMbyte*)maybeFact, 4);
	maybeFact[5] = 0;
	if (strEqual(maybeFact, "fact"))
	{
		ms.read((GMbyte*)&fact, sizeof(GM_WAVEFACT));
	}

	WaveData* data;
	WaveData::newWaveData(&data);

	ms.read((GMbyte*)data, WaveData::HEADER_SIZE);
	data->data = new GMbyte[data->dwSize];
	ms.read(data->data, data->dwSize);

	SoundFile* _sf = new WavSoundFile(format.waveFormatEx, data);
	*sf = _sf;
	return true;
#else
	ASSERT(false);
	return false;
#endif
}

bool SoundReader_Wav::test(const GamePackageBuffer& buffer)
{
	MemoryStream ms(buffer.buffer, buffer.size);
	GM_WAVERIFF riff;
	ms.read((GMbyte*)&riff, sizeof(GM_WAVERIFF));
	return riff.chData[0] == 'W' && riff.chData[1] == 'A' && riff.chData[2] == 'V' && riff.chData[3] == 'E';
}
