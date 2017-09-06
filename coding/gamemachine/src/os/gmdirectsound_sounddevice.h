#ifndef __DIRECTSOUND_SOUNDPLAYER_H__
#define __DIRECTSOUND_SOUNDPLAYER_H__
#include <gmcommon.h>
#include "foundation/utilities/utilities.h"
#include "gmdatacore/soundreader/gmsoundreader.h"
#include <gmcom.h>

#if _WINDOWS
#	if _MSC_VER
#		include <dsound.h>
#	else
#		include <dsound_gm.h>
#	endif
#	include <mmsystem.h>
#endif

BEGIN_NS

GM_REGISTER_IUNKNOWN(IDirectSound8)

#if _WINDOWS

class GMUIWindow;
class GMSoundPlayerDevice
{
public:
	static void createInstance(IWindow* window);
	static IDirectSound8* getInstance();
	static void terminate();

private:
	GMSoundPlayerDevice(IWindow* window);

private:
	ComPtr<IDirectSound8> m_cpDirectSound;
};

#endif

END_NS
#endif
