#include "stdafx.h"
#include "directsound_sounddevice.h"
#include "gmdatacore/soundreader/soundreader.h"

#ifdef _WINDOWS

#ifdef _MSC_VER
#pragma comment(lib,"dsound.lib")
#endif

static SoundPlayerDevice* g_device;

SoundPlayerDevice::SoundPlayerDevice(IWindow* window)
{
	HRESULT hr = DirectSoundCreate8(NULL, &m_cpDirectSound, NULL);
	ASSERT(SUCCEEDED(hr));

	hr = m_cpDirectSound->SetCooperativeLevel(window->hwnd(), DSSCL_PRIORITY);
	ASSERT(SUCCEEDED(hr));
}

IDirectSound8* SoundPlayerDevice::getInstance()
{
	return g_device->m_cpDirectSound;
}

void SoundPlayerDevice::createInstance(IWindow* window)
{
	if (!g_device)
		g_device = new SoundPlayerDevice(window);
}

#endif