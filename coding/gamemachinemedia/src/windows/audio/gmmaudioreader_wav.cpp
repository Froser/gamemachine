#include "stdafx.h"
#include <gmcom.h>
#include "gmmaudioreader_wav.h"
#include "alframework/cwaves.h"
#include <al.h>

#pragma pack(push, 4)
struct WAVEFILEHEADER
{
	char			szRIFF[4];
	unsigned long	ulRIFFSize;
	char			szWAVE[4];
};

static bool strnEqual(char* a, char* b, gm::GMint n)
{
	gm::GMint i = 0;
	do
	{
		if (a[i] != b[i])
			return false;
	}
	while (++i < n);
	return true;
}

static CWaves& getWaveLoader()
{
	static CWaves g_cwaves;
	return g_cwaves;
}

GM_PRIVATE_OBJECT(GMMAudioPlayer_Wav)
{
	WAVEID waveId = -1;
	gm::GMAudioFileInfo fileInfo;
};

class GMMAudioPlayer_Wav : public gm::IAudioFile
{
	DECLARE_PRIVATE(GMMAudioPlayer_Wav)

public:
	GMMAudioPlayer_Wav()
	{
	}

	~GMMAudioPlayer_Wav()
	{
		disposeAudioFile();
	}

public:
	bool load(gm::GMBuffer& buffer)
	{
		D(d);
		CWaves& waveLoader = getWaveLoader();
		if (WAVE_SUCCEEDED(waveLoader.LoadWaveFile("D:\\gmpk\\audio\\footsteps.wav", &d->waveId)))
		{
			if ((WAVE_SUCCEEDED(waveLoader.GetWaveSize(d->waveId, (unsigned long*)&d->fileInfo.size))) &&
				(WAVE_SUCCEEDED(waveLoader.GetWaveData(d->waveId, (void**)&d->fileInfo.data))) &&
				(WAVE_SUCCEEDED(waveLoader.GetWaveFrequency(d->waveId, (unsigned long*)&d->fileInfo.frequency))) &&
				(WAVE_SUCCEEDED(waveLoader.GetWaveALBufferFormat(d->waveId, &alGetEnumValue, (unsigned long*)&d->fileInfo.format))))
			return true;
		}
		return false;
	}

public:
	virtual void disposeAudioFile() override
	{
		D(d);
		if (d->waveId > 0)
		{
			CWaves& waveLoader = getWaveLoader();
			waveLoader.DeleteWaveFile(d->waveId);
			d->waveId = -1;
		}
	}

	virtual gm::GMAudioFileInfo& getFileInfo() override
	{
		D(d);
		return d->fileInfo;
	}
};

bool GMMAudioReader_Wav::load(gm::GMBuffer& buffer, OUT gm::IAudioFile** f)
{
	GMMAudioPlayer_Wav* file = new GMMAudioPlayer_Wav();
	(*f) = file;
	return file->load(buffer);
}

bool GMMAudioReader_Wav::test(const gm::GMBuffer& buffer)
{
	WAVEFILEHEADER header;
	gm::MemoryStream ms(buffer.buffer, buffer.size);
	ms.read((gm::GMbyte*)&header, sizeof(header));
	GM_ASSERT(GM.getMachineEndianness() == gm::GameMachine::LITTLE_ENDIAN);
	return strnEqual(header.szRIFF, "RIFF", 4) && strnEqual(header.szWAVE, "WAVE", 4);
}
