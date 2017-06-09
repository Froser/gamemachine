#include "stdafx.h"
#include "gmdirectsound_sounddevice.h"
#include "gmdatacore/soundreader/gmsoundreader.h"
#include "gmui/gmui.h"

#if _WINDOWS

#ifdef _MSC_VER
#pragma comment(lib,"dsound.lib")
#endif

static GMSoundPlayerDevice* g_device;

GMSoundPlayerDevice::GMSoundPlayerDevice(GMUIWindow* window)
{
	HRESULT hr = DirectSoundCreate8(NULL, &m_cpDirectSound, NULL);
	ASSERT(SUCCEEDED(hr));

	hr = m_cpDirectSound->SetCooperativeLevel(window->getWindowHandle(), DSSCL_PRIORITY);
	ASSERT(SUCCEEDED(hr));
}

IDirectSound8* GMSoundPlayerDevice::getInstance()
{
	return g_device->m_cpDirectSound;
}

void GMSoundPlayerDevice::createInstance(GMUIWindow* window)
{
	if (!g_device)
		g_device = new GMSoundPlayerDevice(window);
}

#endif