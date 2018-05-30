#include "stdafx.h"
#include "gminput.h"
#include <gamemachine.h>

static GMString xinputDlls[] = {
	"xinput9_1_0.dll",
	"xinput1_4.dll",
	"xinput1_3.dll",
};

XInputWrapper::XInputWrapper()
	: m_xinputGetState(nullptr)
	, m_xinputSetState(nullptr)
	, m_module(0)
{
	for (GMint i = 0; i < 3; i++)
	{
		m_module = LoadLibrary(xinputDlls[i].toStdWString().c_str());
		if (m_module)
			break;
	}

	if (!m_module)
	{
		gm_warning(L"cannot find xinput dll, xinput disabled.");
		::FreeLibrary(m_module);
		return;
	}

	m_xinputGetState = (XInputGetState_Delegate)GetProcAddress(m_module, "XInputGetState");
	m_xinputSetState = (XInputSetState_Delegate)GetProcAddress(m_module, "XInputSetState");
}

DWORD XInputWrapper::XInputGetState(DWORD dwUserIndex, XINPUT_STATE* pState)
{
	if (m_xinputGetState)
		return m_xinputGetState(dwUserIndex, pState);
	else
		gm_warning(L"cannot invoke XInputGetState");
	return ERROR_DLL_INIT_FAILED;
}

DWORD XInputWrapper::XInputSetState(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration)
{
	if (m_xinputSetState)
		return m_xinputSetState(dwUserIndex, pVibration);
	else
		gm_warning(L"cannot invoke XInputGetState");
	return ERROR_DLL_INIT_FAILED;
}

XInputWrapper::~XInputWrapper()
{
	::FreeLibrary(m_module);
}

GMInput::GMInput(IWindow* window)
{
	D(d);
	d->window = window;
}

void GMInput::update()
{
	D(d);
	::GetKeyboardState(d->lastKeyState);
	// restore
	d->wheelState.wheeled = false;
	d->mouseState.downButton = d->mouseState.upButton = GMMouseButton_None;
	d->mouseState.moving = false;
}

void GMInput::setDetectingMode(bool enable)
{
	D(d);
	if (enable)
	{
		const GMRect& rect = GM.getGameMachineRunningStates().renderRect;
		::SetCursorPos(rect.x + rect.width / 2, rect.y + rect.height / 2);
		::ShowCursor(FALSE);
	}
	else
	{
		::ShowCursor(TRUE);
	}

	d->detectingMode = enable;
}

void GMInput::setCursor(GMCursorType type)
{
	D(d);
	LPCWSTR cursor = nullptr;
	switch (type)
	{
	case GMCursorType::Arrow:
		cursor = IDC_ARROW;
		break;
	case GMCursorType::IBeam:
		cursor = IDC_IBEAM;
		break;
	case GMCursorType::Wait:
		cursor = IDC_WAIT;
		break;
	case GMCursorType::Cross:
		cursor = IDC_CROSS;
		break;
	case GMCursorType::UpArrow:
		cursor = IDC_UPARROW;
		break;
	case GMCursorType::Hand:
		cursor = IDC_HAND;
		break;
	case GMCursorType::Custom:
		::ShowCursor(FALSE);
		return;
	default:
		GM_ASSERT(false);
		break;
	}
	::ShowCursor(TRUE);
	::SetClassLongPtr(d->window->getWindowHandle(), GCLP_HCURSOR, (LONG_PTR)::LoadCursor(NULL, cursor));
}

GMJoystickState GMInput::joystickState()
{
	D(d);
	XINPUT_STATE state;
	GMJoystickState result = { false };

	if (d->xinput.XInputGetState(0, &state) == ERROR_SUCCESS)
	{
		result.valid = true;
		result.leftTrigger = state.Gamepad.bLeftTrigger;
		result.rightTrigger = state.Gamepad.bRightTrigger;
		result.thumbLX = state.Gamepad.sThumbLX;
		result.thumbLY = state.Gamepad.sThumbLY;
		result.thumbRX = state.Gamepad.sThumbRX;
		result.thumbRY = state.Gamepad.sThumbRY;
		result.buttons = state.Gamepad.wButtons;
	}

	return std::move(result);
}

void GMInput::setIMEState(bool enabled)
{
	// TODO
}

void GMInput::joystickVibrate(GMushort leftMotorSpeed, GMushort rightMotorSpeed)
{
	D(d);
	XINPUT_VIBRATION v = { leftMotorSpeed, rightMotorSpeed };
	d->xinput.XInputSetState(0, &v);
}

IKeyboardState& GMInput::getKeyboardState()
{
	GM_PROFILE("getKeyboardState");
	D(d);
	GetKeyboardState(d->keyState);
	return *this;
}

void GMInput::msgProc(GMSystemEvent* event)
{
	GMSystemEventType type = event->getType();
	switch (type)
	{
	case GMSystemEventType::MouseWheel:
	{
		GMSystemMouseWheelEvent* e = gm_cast<GMSystemMouseWheelEvent*>(event);
		recordWheel(true, e->getDelta());
		break;
	}
	case GMSystemEventType::MouseMove:
	case GMSystemEventType::MouseDown:
	case GMSystemEventType::MouseUp:
	{
		GMSystemMouseEvent* e = gm_cast<GMSystemMouseEvent*>(event);
		if (type == GMSystemEventType::MouseMove)
		{
			recordMouseMove();
		}
		else if (type == GMSystemEventType::MouseDown)
		{
			recordMouseDown(e->getButton());
		}
		else
		{
			GM_ASSERT(type == GMSystemEventType::MouseUp);
			recordMouseUp(e->getButton());
		}
		break;
	}
	}
}

GMMouseState GMInput::mouseState()
{
	D(d);
	GMMouseState& state = d->mouseState;
	state.wheel = d->wheelState;

	POINT pos;
	::GetCursorPos(&pos);

	{
		POINT p = pos;
		::ScreenToClient(d->window->getWindowHandle(), &p);
		state.posX = p.x;
		state.posY = p.y;

		IKeyboardState& ks = getKeyboardState();
		state.triggerButton = GMMouseButton_None;
		if (ks.keyTriggered(VK_LBUTTON))
			state.triggerButton |= GMMouseButton_Left;
		if (ks.keyTriggered(VK_RBUTTON))
			state.triggerButton |= GMMouseButton_Right;
		if (ks.keyTriggered(VK_MBUTTON))
			state.triggerButton |= GMMouseButton_Middle;
	}

	if (d->detectingMode)
	{
		GMRect rect = d->window->getWindowRect();
		const GMint centerX = rect.x + rect.width / 2;
		const GMint centerY = rect.y + rect.height / 2;
		::SetCursorPos(centerX, centerY);
		state.deltaX = pos.x - centerX;
		state.deltaY = pos.y - centerY;
	}
	else
	{
		state.deltaX = state.deltaY = 0;
	}
	return state;
}