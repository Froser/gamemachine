#include "stdafx.h"
#include "soundreader.h"
#include "soundreader_wav.h"
#include "soundreader_mp3.h"

class SoundReaderContainer
{
public:
	SoundReaderContainer()
	{
		m_readers[SoundType_Wav] = new SoundReader_Wav();
		m_readers[SoundType_MP3] = new SoundReader_MP3();
	}

	~SoundReaderContainer()
	{
		for (auto iter = m_readers.begin(); iter != m_readers.end(); iter++)
		{
			delete iter->second;
		}
	}

	ISoundReader* getReader(SoundType type)
	{
		ASSERT(m_readers.find(type) != m_readers.end());
		return m_readers[type];
	}

private:
	std::map<SoundType, ISoundReader*> m_readers;
};

SoundFile::SoundFile(const WAVEFORMATEX& fmt, AUTORELEASE WaveData* waveData)
	: m_waveFormat(fmt)
	, m_waveData(waveData)
{
}

SoundFile::~SoundFile()
{
	if (m_waveData)
		delete m_waveData;
}

WAVEFORMATEX* SoundFile::getWaveFormat()
{
	return &m_waveFormat;
}

WaveData* SoundFile::getData()
{
	return m_waveData;
}

ISoundReader* SoundReader::getReader(SoundType type)
{
	static SoundReaderContainer readers;
	return readers.getReader(type);
}

SoundType SoundReader::test(const GamePackageBuffer& buffer)
{
	for (SoundType i = SoundType_Begin; i < SoundType_End; i = (SoundType)((GMuint)i + 1))
	{
		if (getReader(i)->test(buffer))
			return i;
	}
	return SoundType_End;
}

bool SoundReader::load(GamePackageBuffer& buffer, OUT ISoundFile** sf)
{
	return load(buffer, SoundType_AUTO, sf);
}

bool SoundReader::load(GamePackageBuffer& buffer, SoundType type, OUT ISoundFile** sf)
{
	if (type == SoundType_AUTO)
		type = test(buffer);

	if (type == SoundType_End)
		return false;

	return getReader(type)->load(buffer, sf);
}
