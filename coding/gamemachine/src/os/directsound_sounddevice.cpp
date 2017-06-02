#include "stdafx.h"
#include "directsound_sounddevice.h"
#include "gmdatacore/soundreader/gmsoundreader.h"

#ifdef _WINDOWS

#ifdef _MSC_VER
#pragma comment(lib,"dsound.lib")
#endif

static GMSoundPlayerDevice* g_device;

GMSoundPlayerDevice::GMSoundPlayerDevice(IWindow* window)
{
	HRESULT hr = DirectSoundCreate8(NULL, &m_cpDirectSound, NULL);
	ASSERT(SUCCEEDED(hr));

	hr = m_cpDirectSound->SetCooperativeLevel(window->hwnd(), DSSCL_PRIORITY);
	ASSERT(SUCCEEDED(hr));
}

IDirectSound8* GMSoundPlayerDevice::getInstance()
{
	return g_device->m_cpDirectSound;
}

void GMSoundPlayerDevice::createInstance(IWindow* window)
{
	if (!g_device)
		g_device = new GMSoundPlayerDevice(window);
}

#endif