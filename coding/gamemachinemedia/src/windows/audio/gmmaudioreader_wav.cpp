#include "stdafx.h"
#include <gmcom.h>
#include "gmmaudioreader_wav.h"

#if _MSC_VER
#	include <dsound.h>
#else
#	include <dsound_gm.h>
#endif

struct GMMWAVERIFF
{
	gm::GMbyte chID[4];
	DWORD dwSize;
	gm::GMbyte chData[4];
};

struct GMMWAVEFORMATEX
{
	gm::GMbyte chID[4];
	DWORD dwSize;
	WAVEFORMATEX waveFormatEx;
};

struct GMMWAVEFACT
{
	gm::GMbyte chID[4];
	DWORD dwSize;
	DWORD data;
};

struct GMMPCMDATA
{
	gm::GMbyte chID[4] = { 0 };
	DWORD dwSize = 0;
	gm::GMbyte* data = nullptr;

	enum
	{
		HEADER_SIZE = 8,
	};
};


GM_REGISTER_IUNKNOWN(IDirectSoundBuffer8)
GM_REGISTER_IUNKNOWN(IDirectSoundBuffer)

GM_PRIVATE_OBJECT(GMMAudioPlayer_Wav)
{
	gm::ComPtr<IDirectSoundBuffer8> cpDirectSoundBuffer;
	GMMWAVERIFF riff;
	GMMWAVEFORMATEX fmtex;
	GMMWAVEFACT fact;
	GMMPCMDATA pcm;
	bool playing;
};

class GMMAudioPlayer_Wav : public gm::IAudioPlayer
{
	DECLARE_PRIVATE(GMMAudioPlayer_Wav)

public:
	GMMAudioPlayer_Wav()
	{
		D(d);
		d->playing = false;
	}

	~GMMAudioPlayer_Wav()
	{
		D(d);
		if (d->pcm.data)
			delete[] d->pcm.data;
	}

	bool load(gm::GMBuffer& buffer)
	{
		D(d);
		GM_ASSERT(GM.getMachineEndianness() == gm::GameMachine::LITTLE_ENDIAN);

		gm::MemoryStream ms(buffer.buffer, buffer.size);
		ms.read((gm::GMbyte*)&d->riff, sizeof(GMMWAVERIFF));
		ms.read((gm::GMbyte*)&d->fmtex.chID, sizeof(d->fmtex.chID));
		ms.read((gm::GMbyte*)&d->fmtex.dwSize, sizeof(d->fmtex.dwSize));

		gm::GMuint pos = ms.tell();
		ms.read((gm::GMbyte*)&d->fmtex.waveFormatEx, sizeof(d->fmtex.waveFormatEx));
		ms.seek(pos + d->fmtex.dwSize);

		char maybeFact[6];
		ms.peek((gm::GMbyte*)maybeFact, 4);
		maybeFact[5] = 0;
		if (strEqual(maybeFact, "fact"))
		{
			ms.read((gm::GMbyte*)&d->fact, sizeof(GMMWAVEFACT));
		}

		ms.read(reinterpret_cast<gm::GMbyte*>(&d->pcm), GMMPCMDATA::HEADER_SIZE);
		d->pcm.data = new gm::GMbyte[d->pcm.dwSize];
		ms.read(d->pcm.data, d->pcm.dwSize);
		return true;
	}

public:
	virtual void play() override
	{
		//D(d);
		//loadSound();
		//d->playing = true;
		//HRESULT hr = d->cpDirectSoundBuffer->Play(0, 0, DSBPLAY_LOOPING);
		//GM_ASSERT(SUCCEEDED(hr));
	}

	virtual void stop() override
	{
		//D(d);
		//d->cpDirectSoundBuffer->Stop();
		//d->playing = false;
	}

private:
	/*
	void loadSound()
	{
		D(d);
		DSBUFFERDESC dsbd = { 0 };
		dsbd.dwSize = sizeof(DSBUFFERDESC);
		dsbd.dwFlags = DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLFX | DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_GETCURRENTPOSITION2;
		dsbd.dwBufferBytes = getData()->dwSize;
		dsbd.lpwfxFormat = (LPWAVEFORMATEX)getWaveFormat();

		gm::ComPtr<IDirectSoundBuffer> cpBuffer;
		HRESULT hr;
		if (FAILED(hr = GMSoundPlayerDevice::getInstance()->CreateSoundBuffer(&dsbd, &cpBuffer, NULL)))
		{
			gm_error(_L("create sound buffer error."));
			return;
		}

		if (FAILED(hr = cpBuffer->QueryInterface(IID_IDirectSoundBuffer8, (LPVOID*)&d->cpDirectSoundBuffer)))
		{
			gm_error(_L("QueryInterface to IDirectSoundBuffer8 error"));
			return;
		}

		LPVOID lpLockBuf;
		DWORD len;
		d->cpDirectSoundBuffer->Lock(0, 0, &lpLockBuf, &len, NULL, NULL, DSBLOCK_ENTIREBUFFER);
		memcpy(lpLockBuf, getData()->data, len);
		d->cpDirectSoundBuffer->Unlock(lpLockBuf, len, NULL, NULL);
		d->cpDirectSoundBuffer->SetCurrentPosition(0);
	}
	*/
};

bool GMMAudioReader_Wav::load(gm::GMBuffer& buffer, OUT gm::IAudioPlayer** sf)
{
	GMMAudioPlayer_Wav* player = new GMMAudioPlayer_Wav();
	(*sf) = player;
	return player->load(buffer);
}

bool GMMAudioReader_Wav::test(const gm::GMBuffer& buffer)
{
	gm::MemoryStream ms(buffer.buffer, buffer.size);
	GMMWAVERIFF riff;
	ms.read((gm::GMbyte*)&riff, sizeof(GMMWAVERIFF));
	GM_ASSERT(GM.getMachineEndianness() == gm::GameMachine::LITTLE_ENDIAN);
	return riff.chData[0] == 'W' && riff.chData[1] == 'A' && riff.chData[2] == 'V' && riff.chData[3] == 'E';
}
