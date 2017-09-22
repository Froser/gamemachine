#include "stdafx.h"
#include "gmuiinput.h"

static gm::GMString xinputDlls[] = {
	"xinput9_1_0.dll",
	"xinput1_4.dll",
	"xinput1_3.dll",
};

XInputWrapper::XInputWrapper()
	: m_xinputGetState(nullptr)
	, m_xinputSetState(nullptr)
	, m_module(0)
{
	for (gm::GMint i = 0; i < 3; i++)
	{
		m_module = LoadLibrary(xinputDlls[i].toStdWString().c_str());
		if (m_module)
			break;
	}

	if (!m_module)
	{
		gm_warning(_L("cannot find xinput dll, xinput disabled."));
		FreeLibrary(m_module);
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
		gm_warning(_L("cannot invoke XInputGetState"));
	return ERROR_DLL_INIT_FAILED;
}

DWORD XInputWrapper::XInputSetState(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration)
{
	if (m_xinputSetState)
		return m_xinputSetState(dwUserIndex, pVibration);
	else
		gm_warning(_L("cannot invoke XInputGetState"));
	return ERROR_DLL_INIT_FAILED;
}

XInputWrapper::~XInputWrapper()
{
	::FreeLibrary(m_module);
}

GMInput::GMInput(gm::IWindow* window)
{
	D(d);
	d->window = window;
	initMouse();
}

void GMInput::update()
{
	D(d);
	::GetKeyboardState(d->lastKeyState);
}

void GMInput::initMouse()
{
	D(d);
	gm::GMRect rect = d->window->getWindowRect();
	::SetCursorPos(rect.x + rect.width / 2, rect.y + rect.height / 2);
	d->mouseReady = true;
	setMouseEnable(true);
}

void GMInput::setMouseEnable(bool enable)
{
	D(d);
	d->mouseEnabled = enable;
}

gm::GMJoystickState GMInput::joystickState()
{
	D(d);
	XINPUT_STATE state;
	gm::GMJoystickState result = { false };

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
	D(d);
	HIMC hImc = ImmGetContext(d->window->getWindowHandle());
	::ImmSetOpenStatus(hImc, enabled);
}

void GMInput::joystickVibrate(gm::GMushort leftMotorSpeed, gm::GMushort rightMotorSpeed)
{
	D(d);
	XINPUT_VIBRATION v = { leftMotorSpeed, rightMotorSpeed };
	d->xinput.XInputSetState(0, &v);
}

gm::IKeyboardState& GMInput::getKeyboardState()
{
	GM_PROFILE(getKeyboardState);
	D(d);
	GetKeyboardState(d->keyState);

	return *this;
}

gm::GMMouseState GMInput::mouseState()
{
	D(d);
	gm::GMMouseState state = { 0 };

	POINT pos;
	::GetCursorPos(&pos);

	{
		POINT p = pos;
		::ScreenToClient(d->window->getWindowHandle(), &p);
		state.posX = p.x;
		state.posY = p.y;

		IKeyboardState& ks = getKeyboardState();
		state.button = GMMouseButton_None;
		if (ks.keydown(VK_LBUTTON))
			state.button |= GMMouseButton_Left;
		if (ks.keydown(VK_RBUTTON))
			state.button |= GMMouseButton_Right;
		if (ks.keydown(VK_MBUTTON))
			state.button |= GMMouseButton_Middle;
	}

	if (!d->mouseReady)
	{
		gm_error(_L("Mouse is not ready. Please call initMouse() first, "
			"otherwise deltaX and deltaY won't be valid."));
	}
	else
	{
		if (d->mouseEnabled)
		{
			gm::GMRect rect = d->window->getWindowRect();
			const gm::GMint centerX = rect.x + rect.width / 2;
			const gm::GMint centerY = rect.y + rect.height / 2;
			::SetCursorPos(centerX, centerY);
			state.deltaX = pos.x - centerX;
			state.deltaY = pos.y - centerY;
		}
		else
		{
			state.deltaX = state.deltaY = 0;
		}
	}
	return state;
}