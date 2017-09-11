#ifndef __GMMAUDIOREADER_H__
#define __GMMAUDIOREADER_H__
#include <gmmcommon.h>
#include <gamemachine.h>
#include "../gmmdef.h"

static bool strnEqual(char* a, char* b, gm::GMint n)
{
	gm::GMint i = 0;
	do
	{
		if (a[i] != b[i])
			return false;
	} while (++i < n);
	return true;
}

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