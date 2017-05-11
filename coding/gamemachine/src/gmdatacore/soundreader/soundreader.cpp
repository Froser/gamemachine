#include "stdafx.h"
#include "soundreader.h"
#include "utilities/assert.h"
#include "soundreader_wav.h"

class SoundReaderContainer
{
public:
	SoundReaderContainer()
	{
		m_readers[SoundType_Wav] = new SoundReader_Wav();
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

bool SoundReader::load(GamePackageBuffer& buffer, OUT Object** image)
{
	return load(buffer, SoundType_AUTO, image);
}

bool SoundReader::load(GamePackageBuffer& buffer, SoundType type, OUT Object** object)
{
	if (type == SoundType_AUTO)
		type = test(buffer);

	if (type == SoundType_End)
		return false;

	return getReader(type)->load(buffer, object);
}
