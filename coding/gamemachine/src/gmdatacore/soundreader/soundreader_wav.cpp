#include "stdafx.h"
#include "soundreader_wav.h"
#include "utilities/memorystream.h"
#include "gmdatacore/gamepackage.h"

struct Chunk
{
	Chunk()
		: data(nullptr)
	{

	}

	~Chunk()
	{
		if (data)
			delete[] data;
	}

	GMbyte fmt[4];
	GMlong size;
	GMbyte* data;
};

struct FormatChunk
{
	GMbyte fmt[4];
	GMlong size;
};

bool SoundReader_Wav::load(GamePackageBuffer& buffer, OUT Object** object)
{
	// 假设都是小端模式
	MemoryStream ms(buffer.buffer, buffer.size);
	Chunk riff;
	ms.read(riff.fmt, 4);
	ms.read((GMbyte*)&riff.size, 4);
	riff.data = new GMbyte[4];
	ms.read(riff.data, 4);

	return 0;
}

bool SoundReader_Wav::test(const GamePackageBuffer& buffer)
{
	MemoryStream ms(buffer.buffer, buffer.size);
	GMbyte fmt[4];
	ms.read(fmt, 4);
	return fmt[0] == 'R' && fmt[1] == 'I' && fmt[2] == 'F' && fmt[3] == 'F';
}
