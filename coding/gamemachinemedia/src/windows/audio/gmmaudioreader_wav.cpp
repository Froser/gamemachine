#include "stdafx.h"
#include <gmcom.h>
#include "gmmaudioreader_wav.h"
#include "alframework/cwaves.h"
#include <al.h>

static CWaves& getWaveLoader()
{
	static CWaves g_cwaves;
	return g_cwaves;
}

GM_PRIVATE_OBJECT(GMMAudioPlayer_Wav)
{
	WAVEID waveId = 0;
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
	}

public:
	bool load(gm::GMBuffer& buffer)
	{
		D(d);
		CWaves& waveLoader = getWaveLoader();
		if (WAVE_SUCCEEDED(waveLoader.LoadWaveFile("", &d->waveId)))
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
	struct GMMWAVERIFF
	{
		gm::GMbyte chID[4];
		DWORD dwSize;
		gm::GMbyte chData[4];
	} riff;
	gm::MemoryStream ms(buffer.buffer, buffer.size);
	ms.read((gm::GMbyte*)&riff, sizeof(GMMWAVERIFF));
	GM_ASSERT(GM.getMachineEndianness() == gm::GameMachine::LITTLE_ENDIAN);
	return riff.chData[0] == 'W' && riff.chData[1] == 'A' && riff.chData[2] == 'V' && riff.chData[3] == 'E';
}
