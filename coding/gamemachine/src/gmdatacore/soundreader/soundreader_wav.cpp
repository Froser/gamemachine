#include "stdafx.h"
#include "soundreader_wav.h"
#include "utilities/memorystream.h"
#include "gmdatacore/gamepackage.h"

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

bool SoundReader_Wav::load(GamePackageBuffer& buffer, OUT ISoundFile** sf)
{
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

	WAVEFORMATEX f = format.waveFormatEx;
	SoundFile* _sf = new SoundFile(f, data);
	*sf = _sf;
	return true;
}

bool SoundReader_Wav::test(const GamePackageBuffer& buffer)
{
	MemoryStream ms(buffer.buffer, buffer.size);
	GM_WAVERIFF riff;
	ms.read((GMbyte*)&riff, sizeof(GM_WAVERIFF));
	return riff.chData[0] == 'W' && riff.chData[1] == 'A' && riff.chData[2] == 'V' && riff.chData[3] == 'E';
}
