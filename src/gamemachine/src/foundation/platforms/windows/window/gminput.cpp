#include "stdafx.h"
#include "gminput.h"
#include <gamemachine.h>

namespace
{
	GMWParam getWindowsKey(GMKey key)
	{
		static HashMap<GMKey, GMWParam> s_keyMap;
		if (s_keyMap.empty())
		{
			s_keyMap[GMKey_Lbutton] = VK_LBUTTON;
			s_keyMap[GMKey_Rbutton] = VK_RBUTTON;
			s_keyMap[GMKey_Cancel] = VK_CANCEL;
			s_keyMap[GMKey_Mbutton] = VK_MBUTTON;
			s_keyMap[GMKey_Xbutton1] = VK_XBUTTON1;
			s_keyMap[GMKey_Xbutton2] = VK_XBUTTON2;
			s_keyMap[GMKey_Back] = VK_BACK;
			s_keyMap[GMKey_Tab] = VK_TAB;
			s_keyMap[GMKey_Clear] = VK_CLEAR;
			s_keyMap[GMKey_Return] = VK_RETURN;
			s_keyMap[GMKey_Shift] = VK_SHIFT;
			s_keyMap[GMKey_Control] = VK_CONTROL;
			s_keyMap[GMKey_Menu] = VK_MENU;
			s_keyMap[GMKey_Pause] = VK_PAUSE;
			s_keyMap[GMKey_Capital] = VK_CAPITAL;
			s_keyMap[GMKey_Kana] = VK_KANA;
			s_keyMap[GMKey_Hangul] = VK_HANGUL;
			s_keyMap[GMKey_Junja] = VK_JUNJA;
			s_keyMap[GMKey_Final] = VK_FINAL;
			s_keyMap[GMKey_Hanja] = VK_HANJA;
			s_keyMap[GMKey_Kanji] = VK_KANJI;
			s_keyMap[GMKey_Escape] = VK_ESCAPE;
			s_keyMap[GMKey_Convert] = VK_CONVERT;
			s_keyMap[GMKey_Nonconvert] = VK_NONCONVERT;
			s_keyMap[GMKey_Accept] = VK_ACCEPT;
			s_keyMap[GMKey_Modechange] = VK_MODECHANGE;
			s_keyMap[GMKey_Space] = VK_SPACE;
			s_keyMap[GMKey_Prior] = VK_PRIOR;
			s_keyMap[GMKey_Next] = VK_NEXT;
			s_keyMap[GMKey_End] = VK_END;
			s_keyMap[GMKey_Home] = VK_HOME;
			s_keyMap[GMKey_Left] = VK_LEFT;
			s_keyMap[GMKey_Up] = VK_UP;
			s_keyMap[GMKey_Right] = VK_RIGHT;
			s_keyMap[GMKey_Down] = VK_DOWN;
			s_keyMap[GMKey_Select] = VK_SELECT;
			s_keyMap[GMKey_Print] = VK_PRINT;
			s_keyMap[GMKey_Execute] = VK_EXECUTE;
			s_keyMap[GMKey_Snapshot] = VK_SNAPSHOT;
			s_keyMap[GMKey_Insert] = VK_INSERT;
			s_keyMap[GMKey_Delete] = VK_DELETE;
			s_keyMap[GMKey_Help] = VK_HELP;
			s_keyMap[GMKey_Lwin] = VK_LWIN;
			s_keyMap[GMKey_Rwin] = VK_RWIN;
			s_keyMap[GMKey_Apps] = VK_APPS;
			s_keyMap[GMKey_Sleep] = VK_SLEEP;
			s_keyMap[GMKey_Numpad0] = VK_NUMPAD0;
			s_keyMap[GMKey_Numpad1] = VK_NUMPAD1;
			s_keyMap[GMKey_Numpad2] = VK_NUMPAD2;
			s_keyMap[GMKey_Numpad3] = VK_NUMPAD3;
			s_keyMap[GMKey_Numpad4] = VK_NUMPAD4;
			s_keyMap[GMKey_Numpad5] = VK_NUMPAD5;
			s_keyMap[GMKey_Numpad6] = VK_NUMPAD6;
			s_keyMap[GMKey_Numpad7] = VK_NUMPAD7;
			s_keyMap[GMKey_Numpad8] = VK_NUMPAD8;
			s_keyMap[GMKey_Numpad9] = VK_NUMPAD9;
			s_keyMap[GMKey_Multiply] = VK_MULTIPLY;
			s_keyMap[GMKey_Add] = VK_ADD;
			s_keyMap[GMKey_Separator] = VK_SEPARATOR;
			s_keyMap[GMKey_Subtract] = VK_SUBTRACT;
			s_keyMap[GMKey_Decimal] = VK_DECIMAL;
			s_keyMap[GMKey_Divide] = VK_DIVIDE;
			s_keyMap[GMKey_F1] = VK_F1;
			s_keyMap[GMKey_F2] = VK_F2;
			s_keyMap[GMKey_F3] = VK_F3;
			s_keyMap[GMKey_F4] = VK_F4;
			s_keyMap[GMKey_F5] = VK_F5;
			s_keyMap[GMKey_F6] = VK_F6;
			s_keyMap[GMKey_F7] = VK_F7;
			s_keyMap[GMKey_F8] = VK_F8;
			s_keyMap[GMKey_F9] = VK_F9;
			s_keyMap[GMKey_F10] = VK_F10;
			s_keyMap[GMKey_F11] = VK_F11;
			s_keyMap[GMKey_F12] = VK_F12;
			s_keyMap[GMKey_F13] = VK_F13;
			s_keyMap[GMKey_F14] = VK_F14;
			s_keyMap[GMKey_F15] = VK_F15;
			s_keyMap[GMKey_F16] = VK_F16;
			s_keyMap[GMKey_F17] = VK_F17;
			s_keyMap[GMKey_F18] = VK_F18;
			s_keyMap[GMKey_F19] = VK_F19;
			s_keyMap[GMKey_F20] = VK_F20;
			s_keyMap[GMKey_F21] = VK_F21;
			s_keyMap[GMKey_F22] = VK_F22;
			s_keyMap[GMKey_F23] = VK_F23;
			s_keyMap[GMKey_F24] = VK_F24;
			s_keyMap[GMKey_Numlock] = VK_NUMLOCK;
			s_keyMap[GMKey_Scroll] = VK_SCROLL;
			s_keyMap[GMKey_Lshift] = VK_LSHIFT;
			s_keyMap[GMKey_Rshift] = VK_RSHIFT;
			s_keyMap[GMKey_Lcontrol] = VK_LCONTROL;
			s_keyMap[GMKey_Rcontrol] = VK_RCONTROL;
			s_keyMap[GMKey_Lmenu] = VK_LMENU;
			s_keyMap[GMKey_Rmenu] = VK_RMENU;
			s_keyMap[GMKey_Browser_back] = VK_BROWSER_BACK;
			s_keyMap[GMKey_Browser_forward] = VK_BROWSER_FORWARD;
			s_keyMap[GMKey_Browser_refresh] = VK_BROWSER_REFRESH;
			s_keyMap[GMKey_Browser_stop] = VK_BROWSER_STOP;
			s_keyMap[GMKey_Browser_search] = VK_BROWSER_SEARCH;
			s_keyMap[GMKey_Browser_favorites] = VK_BROWSER_FAVORITES;
			s_keyMap[GMKey_Browser_home] = VK_BROWSER_HOME;
			s_keyMap[GMKey_Volume_mute] = VK_VOLUME_MUTE;
			s_keyMap[GMKey_Volume_down] = VK_VOLUME_DOWN;
			s_keyMap[GMKey_Volume_up] = VK_VOLUME_UP;
			s_keyMap[GMKey_Media_next_track] = VK_MEDIA_NEXT_TRACK;
			s_keyMap[GMKey_Media_prev_track] = VK_MEDIA_PREV_TRACK;
			s_keyMap[GMKey_Media_stop] = VK_MEDIA_STOP;
			s_keyMap[GMKey_Media_play_pause] = VK_MEDIA_PLAY_PAUSE;
			s_keyMap[GMKey_Launch_mail] = VK_LAUNCH_MAIL;
			s_keyMap[GMKey_Launch_media_select] = VK_LAUNCH_MEDIA_SELECT;
			s_keyMap[GMKey_Launch_app1] = VK_LAUNCH_APP1;
			s_keyMap[GMKey_Launch_app2] = VK_LAUNCH_APP2;
			s_keyMap[GMKey_Oem_1] = VK_OEM_1;
			s_keyMap[GMKey_Oem_plus] = VK_OEM_PLUS;
			s_keyMap[GMKey_Oem_comma] = VK_OEM_COMMA;
			s_keyMap[GMKey_Oem_minus] = VK_OEM_MINUS;
			s_keyMap[GMKey_Oem_period] = VK_OEM_PERIOD;
			s_keyMap[GMKey_Oem_2] = VK_OEM_2;
			s_keyMap[GMKey_Oem_3] = VK_OEM_3;
			s_keyMap[GMKey_Oem_4] = VK_OEM_4;
			s_keyMap[GMKey_Oem_5] = VK_OEM_5;
			s_keyMap[GMKey_Oem_6] = VK_OEM_6;
			s_keyMap[GMKey_Oem_7] = VK_OEM_7;
			s_keyMap[GMKey_Oem_8] = VK_OEM_8;
			s_keyMap[GMKey_Oem_102] = VK_OEM_102;
			s_keyMap[GMKey_Processkey] = VK_PROCESSKEY;
			s_keyMap[GMKey_Packet] = VK_PACKET;
			s_keyMap[GMKey_Attn] = VK_ATTN;
			s_keyMap[GMKey_Crsel] = VK_CRSEL;
			s_keyMap[GMKey_Exsel] = VK_EXSEL;
			s_keyMap[GMKey_Ereof] = VK_EREOF;
			s_keyMap[GMKey_Play] = VK_PLAY;
			s_keyMap[GMKey_Zoom] = VK_ZOOM;
			s_keyMap[GMKey_Noname] = VK_NONAME;
			s_keyMap[GMKey_Pa1] = VK_PA1;
			s_keyMap[GMKey_Oem_Clear] = VK_OEM_CLEAR;
		}

		auto iter = s_keyMap.find(key);
		if (iter != s_keyMap.end())
			return iter->second;

		// 如果GMKey没有在其中，说明是个ASCII码
		return static_cast<GMWParam>(GM_KeyToASCII(key));
	}
}

static GMString xinputDlls[] = {
	"xinput9_1_0.dll",
	"xinput1_4.dll",
	"xinput1_3.dll",
};

GMXInputWrapper::GMXInputWrapper()
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
		gm_warning(gm_dbg_wrap("cannot find xinput dll, xinput disabled."));
		::FreeLibrary(m_module);
		return;
	}

	m_xinputGetState = (XInputGetState_Delegate)GetProcAddress(m_module, "XInputGetState");
	m_xinputSetState = (XInputSetState_Delegate)GetProcAddress(m_module, "XInputSetState");
}

DWORD GMXInputWrapper::XInputGetState(DWORD dwUserIndex, XINPUT_STATE* pState)
{
	if (m_xinputGetState)
		return m_xinputGetState(dwUserIndex, pState);
	else
		gm_warning(gm_dbg_wrap("cannot invoke XInputGetState"));
	return ERROR_DLL_INIT_FAILED;
}

DWORD GMXInputWrapper::XInputSetState(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration)
{
	if (m_xinputSetState)
		return m_xinputSetState(dwUserIndex, pVibration);
	else
		gm_warning(gm_dbg_wrap("cannot invoke XInputGetState"));
	return ERROR_DLL_INIT_FAILED;
}

GMXInputWrapper::~GMXInputWrapper()
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
		const GMRect& rect = d->window->getWindowStates().renderRect;
		::SetCursorPos(rect.x + rect.width / 2, rect.y + rect.height / 2);
		::ShowCursor(FALSE);
	}
	else
	{
		::ShowCursor(TRUE);
	}

	d->detectingMode = enable;
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

bool GMInput::keydown(GMKey key)
{
	D(d);
	return !!(d->keyState[getWindowsKey(key)] & 0x80);
}

// 表示一个键是否按下一次，长按只算是一次
bool GMInput::keyTriggered(GMKey key)
{
	D(d);
	return !(d->lastKeyState[getWindowsKey(key)] & 0x80) && (keydown(key));
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
		if (ks.keyTriggered(GMKey_Lbutton))
			state.triggerButton |= GMMouseButton_Left;
		if (ks.keyTriggered(GMKey_Rbutton))
			state.triggerButton |= GMMouseButton_Right;
		if (ks.keyTriggered(GMKey_Mbutton))
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