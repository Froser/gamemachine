#ifndef __GMMAUDIOREADER_H__
#define __GMMAUDIOREADER_H__
#include <gmmcommon.h>
#include <gminterfaces.h>
#include <gmobject.h>
#include <gmbuffer.h>

BEGIN_MEDIA_NS

GM_INTERFACE(IAudioFormatReader)
{
	virtual bool load(gm::GMBuffer& buffer, OUT gm::IAudioFile** f) = 0;
	virtual bool test(const gm::GMBuffer& buffer) = 0;
};

class GMMAudioReader : public gm::IAudioReader
{
public:
	enum class AudioType
	{
		Wav,
		MP3,
		End,
	};

public:
	virtual bool load(gm::GMBuffer& buffer, OUT gm::IAudioFile** f) override;

private:
	IAudioFormatReader* getReader(AudioType type);
	AudioType test(const gm::GMBuffer& buffer);
};

END_MEDIA_NS

#endif