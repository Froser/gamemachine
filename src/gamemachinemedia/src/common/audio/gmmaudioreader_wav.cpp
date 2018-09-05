#include "stdafx.h"
#include "gmmaudioreader_wav.h"
#include "alframework/cwaves.h"
#include <AL/al.h>
#include <gmtools.h>

#pragma pack(push, 4)
struct WAVEFILEHEADER
{
	char			szRIFF[4];
	unsigned long	ulRIFFSize;
	char			szWAVE[4];
};

static CWaves& getWaveLoader()
{
	static CWaves g_cwaves;
	return g_cwaves;
}

GM_PRIVATE_OBJECT(GMMAudioFile_Wav)
{
	ALuint bufferId = 0;
	WAVEID waveId = -1;
	gm::GMAudioFileInfo fileInfo;
};

class GMMAudioFile_Wav : public gm::IAudioFile
{
	GM_DECLARE_PRIVATE_NGO(GMMAudioFile_Wav)

public:
	GMMAudioFile_Wav()
	{
	}

	~GMMAudioFile_Wav()
	{
		disposeAudioFile();
	}

public:
	bool load(gm::GMBuffer& buffer)
	{
		D(d);
		CWaves& waveLoader = getWaveLoader();
		if (WAVE_SUCCEEDED(waveLoader.LoadWaveBuffer(buffer, &d->waveId)))
		{
			if ((WAVE_SUCCEEDED(waveLoader.GetWaveSize(d->waveId, (unsigned long*)&d->fileInfo.size))) &&
				(WAVE_SUCCEEDED(waveLoader.GetWaveData(d->waveId, (void**)&d->fileInfo.data))) &&
				(WAVE_SUCCEEDED(waveLoader.GetWaveFrequency(d->waveId, (unsigned long*)&d->fileInfo.frequency))) &&
				(WAVE_SUCCEEDED(waveLoader.GetWaveALBufferFormat(d->waveId, &alGetEnumValue, (unsigned long*)&d->fileInfo.format))) &&
				(WAVE_SUCCEEDED(waveLoader.GetWaveFormatExHeader(d->waveId, &d->fileInfo.waveFormatExHeader))))
			{
				alGenBuffers(1, &d->bufferId);
				alBufferData(d->bufferId, d->fileInfo.format, d->fileInfo.data, (ALsizei) d->fileInfo.size, d->fileInfo.frequency);
				disposeAudioFile();
				return true;
			}
		}
		return false;
	}

public:
	virtual bool isStream() override
	{
		return false;
	}

	virtual gm::IAudioStream* getStream() override
	{
		return nullptr;
	}

	virtual gm::GMAudioFileInfo& getFileInfo() override
	{
		D(d);
		return d->fileInfo;
	}

	virtual gm::GMuint getBufferId()
	{
		D(d);
		return d->bufferId;
	}

private:
	virtual void disposeAudioFile()
	{
		D(d);
		if (d->waveId > 0)
		{
			CWaves& waveLoader = getWaveLoader();
			waveLoader.DeleteWaveFile(d->waveId);
			d->waveId = -1;
		}
	}

};

bool GMMAudioReader_Wav::load(gm::GMBuffer& buffer, OUT gm::IAudioFile** f)
{
	GMMAudioFile_Wav* file = new GMMAudioFile_Wav();
	(*f) = file;
	return file->load(buffer);
}

bool GMMAudioReader_Wav::test(const gm::GMBuffer& buffer)
{
	WAVEFILEHEADER header;
	gm::GMMemoryStream ms(buffer.buffer, buffer.size);
	ms.read((gm::GMbyte*)&header, sizeof(header));
	return strnEqual(header.szRIFF, "RIFF", 4) && strnEqual(header.szWAVE, "WAVE", 4);
}
