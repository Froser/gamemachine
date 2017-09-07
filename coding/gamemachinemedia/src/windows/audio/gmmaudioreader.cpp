#include "stdafx.h"
#include "gmmaudioreader.h"

class GMAudioFormatReaders
{
public:
	GMAudioFormatReaders()
	{
		//m_readers[GMMAudioReader::AudioType::Wav] = new GMSoundReader_Wav();
		//m_readers[GMMAudioReader::AudioType::MP3] = new GMSoundReader_MP3();
	}

	~GMAudioFormatReaders()
	{
		for (auto iter = m_readers.begin(); iter != m_readers.end(); iter++)
		{
			delete iter->second;
		}
	}

	IAudioFormatReader* getReader(GMMAudioReader::AudioType type)
	{
		GM_ASSERT(m_readers.find(type) != m_readers.end());
		return m_readers[type];
	}

private:
	Map<GMMAudioReader::AudioType, IAudioFormatReader*> m_readers;
};

IAudioFormatReader* GMMAudioReader::getReader(AudioType type)
{
	static GMAudioFormatReaders readers;
	return readers.getReader(type);
}

GMMAudioReader::AudioType GMMAudioReader::test(const gm::GMBuffer& buffer)
{
	GM_FOREACH_ENUM(i, AudioType::Wav, AudioType::End)
	{
		if (getReader(i)->test(buffer))
			return i;
	}
	return AudioType::End;
}

bool GMMAudioReader::load(gm::GMBuffer& buffer, OUT gm::IAudioPlayer** player)
{
	auto type = test(buffer);
	if (type == AudioType::End)
		return false;

	return getReader(type)->load(buffer, player);
}