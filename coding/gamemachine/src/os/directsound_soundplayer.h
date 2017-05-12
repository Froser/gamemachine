#ifndef __DIRECTSOUND_SOUNDPLAYER_H__
#define __DIRECTSOUND_SOUNDPLAYER_H__
#include "common.h"
#include "utilities/comptr.h"
#include <dsound.h>
#include <mmsystem.h>
#include "gmdatacore/soundreader/soundreader.h"

BEGIN_NS

#ifdef _WINDOWS

struct IWindow;
struct DirectSound_SoundPlayerPrivate
{
	ComPtr<IDirectSound8> cpDirectSound;
	ComPtr<IDirectSoundBuffer8> cpDirectSoundBuffer;
	IWindow* window;
	bool playing;
};

struct ISoundFile;
class DirectSound_SoundPlayer : public ISoundPlayer
{
	DEFINE_PRIVATE(DirectSound_SoundPlayer)

public:
	typedef DirectSound_SoundPlayerPrivate Data;

public:
	DirectSound_SoundPlayer(IWindow* window);
	~DirectSound_SoundPlayer();

public:
	virtual void play(ISoundFile* sf, PlayOptions options) override;
	virtual void stop() override;

private:
	void loadSound(ISoundFile* sf, PlayOptions options);
};

#endif

END_NS
#endif