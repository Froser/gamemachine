#include "stdafx.h"
#include "input.h"
#ifdef _WINDOWS
#include <dinput.h>
#include "gmengine\controllers\gameloop.h"
#include "gmgl\gmglwindow.h"

#define PANIC(hr, expr) if (FAILED(hr)) (expr);

enum
{
	MAX_DEVICES = 8
};

GMuint s_currentDeviceIndex = 0;
GMuint s_enumAxesCallbackIndex = 0;
LPDIRECTINPUT8 s_input;
struct
{
	DWORD type;
	DWORD subType;
	DIDEVCAPS caps;
	DIDEVICEINSTANCE info;
	LPDIRECTINPUTDEVICE8 device;
} s_devices[MAX_DEVICES];

// DIDEVICEINSTANCE s_pdidi;
// DIDEVICEINSTANCE s_info;
// DIJOYSTATE s_state;

BOOL CALLBACK enumCallback(const DIDEVICEINSTANCE* instance, VOID* context)
{
	if (SUCCEEDED(s_input->CreateDevice(instance->guidInstance, &s_devices[s_currentDeviceIndex].device, NULL)))
	{
		s_devices[s_currentDeviceIndex].type = GET_DIDEVICE_TYPE(instance->dwDevType);
		s_devices[s_currentDeviceIndex].subType = GET_DIDEVICE_SUBTYPE(instance->dwDevType);
		s_currentDeviceIndex++;
	}
	else
	{
		s_devices[s_currentDeviceIndex].device = NULL;
	}

	return DIENUM_CONTINUE; 
}

BOOL CALLBACK enumAxesCallback(const DIDEVICEOBJECTINSTANCE* instance, VOID* context)
{
	HWND hDlg = (HWND)context;

	DIPROPRANGE propRange;
	propRange.diph.dwSize = sizeof(DIPROPRANGE);
	propRange.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	propRange.diph.dwHow = DIPH_BYID;
	propRange.diph.dwObj = instance->dwType;
	propRange.lMin = -50;
	propRange.lMax = +50;

	if (FAILED(s_devices[s_enumAxesCallbackIndex].device->SetProperty(DIPROP_RANGE, &propRange.diph)))
	{
		return DIENUM_STOP;
	}
}

Input_Windows::Input_Windows()
{
	init();
}

Input_Windows::~Input_Windows()
{
	for (GMuint i = 0; i < s_currentDeviceIndex; i++)
	{
		s_devices[i].device->Release();
		s_devices[i].device = NULL;
	}
	s_input->Release();
	s_currentDeviceIndex = 0;
}

bool Input_Windows::init()
{
	HRESULT hr;
	if (FAILED(hr = DirectInput8Create(GetModuleHandle(NULL), DIRECTINPUT_VERSION,
		IID_IDirectInput8, (VOID**)&s_input, NULL)))
	{
		gm_error("create direct input failed.");
		return false;
	}

	if (FAILED(hr = s_input->EnumDevices(DI8DEVCLASS_ALL, enumCallback,
		NULL, DIEDFL_ATTACHEDONLY)))
	{
		gm_error("enum direct input devices failed.");
		return false;
	}

	if (FAILED(setup()))
	{
		gm_error("set data format failed.");
		return false;
	}

	return true;
}

HRESULT Input_Windows::setup()
{
	HRESULT hr;
	for (GMuint i = 0; i < s_currentDeviceIndex; i++)
	{
		s_enumAxesCallbackIndex = i;

		GMGLWindow* window = static_cast<GMGLWindow*>(GameLoop::getInstance()->getHandler()->getGameMachine()->getWindow());
		if (FAILED(hr = s_devices[i].device->SetCooperativeLevel(
			window->getHWND(), DISCL_NONEXCLUSIVE | DISCL_FOREGROUND)))
		{
			return hr;
		}

		s_devices[i].caps.dwSize = sizeof(DIDEVCAPS);
		s_devices[i].device->GetCapabilities(&s_devices[i].caps);
		s_devices[i].device->GetDeviceInfo(&s_devices[i].info);
		hr = s_devices[i].device->EnumObjects(enumAxesCallback, NULL, DIDFT_AXIS);
		PANIC(hr, gm_error("enum objects failed"));

		switch (s_devices[i].type)
		{
		case DI8DEVTYPE_MOUSE:
			gm_info("found mouse.");
			hr = s_devices[i].device->SetDataFormat(&c_dfDIMouse);
			PANIC(hr, gm_error("set mouse data format failed"));
			break;
		case DI8DEVTYPE_KEYBOARD:
			gm_info("found keyboard.");
			hr = s_devices[i].device->SetDataFormat(&c_dfDIKeyboard);
			PANIC(hr, gm_error("set keyboard data format failed"));
			break;
		case DI8DEVTYPE_JOYSTICK:
			gm_info("found joystick.");
			hr = s_devices[i].device->SetDataFormat(&c_dfDIJoystick);
			PANIC(hr, gm_error("set joystick data format failed"));
			break;
		default:
			gm_warning("type of %d is not supported yet.", s_devices[i].type);
			break;
		}
	}

	return S_OK;
}

#endif