#include "stdafx.h"
#include "gmsoundreader.h"
#include "gmsoundreader_wav.h"
#include "gmsoundreader_mp3.h"

class GMSoundReaderContainer
{
public:
	GMSoundReaderContainer()
	{
		m_readers[GMSoundReader::SoundType_Wav] = new GMSoundReader_Wav();
		m_readers[GMSoundReader::SoundType_MP3] = new GMSoundReader_MP3();
	}

	~GMSoundReaderContainer()
	{
		for (auto iter = m_readers.begin(); iter != m_readers.end(); iter++)
		{
			delete iter->second;
		}
	}

	ISoundReader* getReader(GMSoundReader::SoundType type)
	{
		GM_ASSERT(m_readers.find(type) != m_readers.end());
		return m_readers[type];
	}

private:
	std::map<GMSoundReader::SoundType, ISoundReader*> m_readers;
};

GMSoundFile::GMSoundFile(const WAVEFORMATEX& fmt, AUTORELEASE GMWaveData* waveData)
	: m_waveFormat(fmt)
	, m_waveData(waveData)
{
}

GMSoundFile::~GMSoundFile()
{
	if (m_waveData)
		delete m_waveData;
}

WAVEFORMATEX* GMSoundFile::getWaveFormat()
{
	return &m_waveFormat;
}

GMWaveData* GMSoundFile::getData()
{
	return m_waveData;
}

ISoundReader* GMSoundReader::getReader(SoundType type)
{
	static GMSoundReaderContainer readers;
	return readers.getReader(type);
}

GMSoundReader::SoundType GMSoundReader::test(const GMBuffer& buffer)
{
	for (SoundType i = SoundType_Begin; i < SoundType_End; i = (SoundType)((GMuint)i + 1))
	{
		if (getReader(i)->test(buffer))
			return i;
	}
	return SoundType_End;
}

bool GMSoundReader::load(GMBuffer& buffer, OUT ISoundFile** sf)
{
	return load(buffer, SoundType_AUTO, sf);
}

bool GMSoundReader::load(GMBuffer& buffer, SoundType type, OUT ISoundFile** sf)
{
	if (type == SoundType_AUTO)
		type = test(buffer);

	if (type == SoundType_End)
		return false;

	return getReader(type)->load(buffer, sf);
}
