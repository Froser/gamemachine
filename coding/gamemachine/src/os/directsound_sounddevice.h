#ifndef __DIRECTSOUND_SOUNDPLAYER_H__
#define __DIRECTSOUND_SOUNDPLAYER_H__
#include "common.h"
#include "foundation/utilities/utilities.h"
#include "gmdatacore/soundreader/soundreader.h"

#ifdef _WINDOWS
#include <dsound.h>
#include <mmsystem.h>
#endif

BEGIN_NS

#ifdef _WINDOWS

struct IWindow;
class SoundPlayerDevice
{
public:
	static void createInstance(IWindow* window);
	static IDirectSound8* getInstance();

private:
	SoundPlayerDevice(IWindow* window);

private:
	ComPtr<IDirectSound8> m_cpDirectSound;
};

#endif

END_NS
#endif
