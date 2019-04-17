#include "stdafx.h"
#include "gminput.h"
#include <gamemachine.h>

namespace
{
	GMWParam getWindowsKey(GMKey key)
	{
		static HashMap<GMKey, GMWParam> s_keyMap;
		static std::once_flag s_flag;
		std::call_once(s_flag, [](auto& keyMap) {
			keyMap[GMKey_Lbutton] = VK_LBUTTON;
			keyMap[GMKey_Rbutton] = VK_RBUTTON;
			keyMap[GMKey_Cancel] = VK_CANCEL;
			keyMap[GMKey_Mbutton] = VK_MBUTTON;
			keyMap[GMKey_Xbutton1] = VK_XBUTTON1;
			keyMap[GMKey_Xbutton2] = VK_XBUTTON2;
			keyMap[GMKey_Back] = VK_BACK;
			keyMap[GMKey_Tab] = VK_TAB;
			keyMap[GMKey_Clear] = VK_CLEAR;
			keyMap[GMKey_Return] = VK_RETURN;
			keyMap[GMKey_Shift] = VK_SHIFT;
			keyMap[GMKey_Control] = VK_CONTROL;
			keyMap[GMKey_Menu] = VK_MENU;
			keyMap[GMKey_Pause] = VK_PAUSE;
			keyMap[GMKey_Capital] = VK_CAPITAL;
			keyMap[GMKey_Kana] = VK_KANA;
			keyMap[GMKey_Hangul] = VK_HANGUL;
			keyMap[GMKey_Junja] = VK_JUNJA;
			keyMap[GMKey_Final] = VK_FINAL;
			keyMap[GMKey_Hanja] = VK_HANJA;
			keyMap[GMKey_Kanji] = VK_KANJI;
			keyMap[GMKey_Escape] = VK_ESCAPE;
			keyMap[GMKey_Convert] = VK_CONVERT;
			keyMap[GMKey_Nonconvert] = VK_NONCONVERT;
			keyMap[GMKey_Accept] = VK_ACCEPT;
			keyMap[GMKey_Modechange] = VK_MODECHANGE;
			keyMap[GMKey_Space] = VK_SPACE;
			keyMap[GMKey_Prior] = VK_PRIOR;
			keyMap[GMKey_Next] = VK_NEXT;
			keyMap[GMKey_End] = VK_END;
			keyMap[GMKey_Home] = VK_HOME;
			keyMap[GMKey_Left] = VK_LEFT;
			keyMap[GMKey_Up] = VK_UP;
			keyMap[GMKey_Right] = VK_RIGHT;
			keyMap[GMKey_Down] = VK_DOWN;
			keyMap[GMKey_Select] = VK_SELECT;
			keyMap[GMKey_Print] = VK_PRINT;
			keyMap[GMKey_Execute] = VK_EXECUTE;
			keyMap[GMKey_Snapshot] = VK_SNAPSHOT;
			keyMap[GMKey_Insert] = VK_INSERT;
			keyMap[GMKey_Delete] = VK_DELETE;
			keyMap[GMKey_Help] = VK_HELP;
			keyMap[GMKey_Lwin] = VK_LWIN;
			keyMap[GMKey_Rwin] = VK_RWIN;
			keyMap[GMKey_Apps] = VK_APPS;
			keyMap[GMKey_Sleep] = VK_SLEEP;
			keyMap[GMKey_Numpad0] = VK_NUMPAD0;
			keyMap[GMKey_Numpad1] = VK_NUMPAD1;
			keyMap[GMKey_Numpad2] = VK_NUMPAD2;
			keyMap[GMKey_Numpad3] = VK_NUMPAD3;
			keyMap[GMKey_Numpad4] = VK_NUMPAD4;
			keyMap[GMKey_Numpad5] = VK_NUMPAD5;
			keyMap[GMKey_Numpad6] = VK_NUMPAD6;
			keyMap[GMKey_Numpad7] = VK_NUMPAD7;
			keyMap[GMKey_Numpad8] = VK_NUMPAD8;
			keyMap[GMKey_Numpad9] = VK_NUMPAD9;
			keyMap[GMKey_Multiply] = VK_MULTIPLY;
			keyMap[GMKey_Add] = VK_ADD;
			keyMap[GMKey_Separator] = VK_SEPARATOR;
			keyMap[GMKey_Subtract] = VK_SUBTRACT;
			keyMap[GMKey_Decimal] = VK_DECIMAL;
			keyMap[GMKey_Divide] = VK_DIVIDE;
			keyMap[GMKey_F1] = VK_F1;
			keyMap[GMKey_F2] = VK_F2;
			keyMap[GMKey_F3] = VK_F3;
			keyMap[GMKey_F4] = VK_F4;
			keyMap[GMKey_F5] = VK_F5;
			keyMap[GMKey_F6] = VK_F6;
			keyMap[GMKey_F7] = VK_F7;
			keyMap[GMKey_F8] = VK_F8;
			keyMap[GMKey_F9] = VK_F9;
			keyMap[GMKey_F10] = VK_F10;
			keyMap[GMKey_F11] = VK_F11;
			keyMap[GMKey_F12] = VK_F12;
			keyMap[GMKey_F13] = VK_F13;
			keyMap[GMKey_F14] = VK_F14;
			keyMap[GMKey_F15] = VK_F15;
			keyMap[GMKey_F16] = VK_F16;
			keyMap[GMKey_F17] = VK_F17;
			keyMap[GMKey_F18] = VK_F18;
			keyMap[GMKey_F19] = VK_F19;
			keyMap[GMKey_F20] = VK_F20;
			keyMap[GMKey_F21] = VK_F21;
			keyMap[GMKey_F22] = VK_F22;
			keyMap[GMKey_F23] = VK_F23;
			keyMap[GMKey_F24] = VK_F24;
			keyMap[GMKey_Numlock] = VK_NUMLOCK;
			keyMap[GMKey_Scroll] = VK_SCROLL;
			keyMap[GMKey_Lshift] = VK_LSHIFT;
			keyMap[GMKey_Rshift] = VK_RSHIFT;
			keyMap[GMKey_Lcontrol] = VK_LCONTROL;
			keyMap[GMKey_Rcontrol] = VK_RCONTROL;
			keyMap[GMKey_Lmenu] = VK_LMENU;
			keyMap[GMKey_Rmenu] = VK_RMENU;
			keyMap[GMKey_Browser_back] = VK_BROWSER_BACK;
			keyMap[GMKey_Browser_forward] = VK_BROWSER_FORWARD;
			keyMap[GMKey_Browser_refresh] = VK_BROWSER_REFRESH;
			keyMap[GMKey_Browser_stop] = VK_BROWSER_STOP;
			keyMap[GMKey_Browser_search] = VK_BROWSER_SEARCH;
			keyMap[GMKey_Browser_favorites] = VK_BROWSER_FAVORITES;
			keyMap[GMKey_Browser_home] = VK_BROWSER_HOME;
			keyMap[GMKey_Volume_mute] = VK_VOLUME_MUTE;
			keyMap[GMKey_Volume_down] = VK_VOLUME_DOWN;
			keyMap[GMKey_Volume_up] = VK_VOLUME_UP;
			keyMap[GMKey_Media_next_track] = VK_MEDIA_NEXT_TRACK;
			keyMap[GMKey_Media_prev_track] = VK_MEDIA_PREV_TRACK;
			keyMap[GMKey_Media_stop] = VK_MEDIA_STOP;
			keyMap[GMKey_Media_play_pause] = VK_MEDIA_PLAY_PAUSE;
			keyMap[GMKey_Launch_mail] = VK_LAUNCH_MAIL;
			keyMap[GMKey_Launch_media_select] = VK_LAUNCH_MEDIA_SELECT;
			keyMap[GMKey_Launch_app1] = VK_LAUNCH_APP1;
			keyMap[GMKey_Launch_app2] = VK_LAUNCH_APP2;
			keyMap[GMKey_Oem_1] = VK_OEM_1;
			keyMap[GMKey_Oem_plus] = VK_OEM_PLUS;
			keyMap[GMKey_Oem_comma] = VK_OEM_COMMA;
			keyMap[GMKey_Oem_minus] = VK_OEM_MINUS;
			keyMap[GMKey_Oem_period] = VK_OEM_PERIOD;
			keyMap[GMKey_Oem_2] = VK_OEM_2;
			keyMap[GMKey_Oem_3] = VK_OEM_3;
			keyMap[GMKey_Oem_4] = VK_OEM_4;
			keyMap[GMKey_Oem_5] = VK_OEM_5;
			keyMap[GMKey_Oem_6] = VK_OEM_6;
			keyMap[GMKey_Oem_7] = VK_OEM_7;
			keyMap[GMKey_Oem_8] = VK_OEM_8;
			keyMap[GMKey_Oem_102] = VK_OEM_102;
			keyMap[GMKey_Processkey] = VK_PROCESSKEY;
			keyMap[GMKey_Packet] = VK_PACKET;
			keyMap[GMKey_Attn] = VK_ATTN;
			keyMap[GMKey_Crsel] = VK_CRSEL;
			keyMap[GMKey_Exsel] = VK_EXSEL;
			keyMap[GMKey_Ereof] = VK_EREOF;
			keyMap[GMKey_Play] = VK_PLAY;
			keyMap[GMKey_Zoom] = VK_ZOOM;
			keyMap[GMKey_Noname] = VK_NONAME;
			keyMap[GMKey_Pa1] = VK_PA1;
			keyMap[GMKey_Oem_Clear] = VK_OEM_CLEAR;
		}, s_keyMap);
		
		auto iter = s_keyMap.find(key);
		if (iter != s_keyMap.end())
			return iter->second;

		// 如果GMKey没有在其中，说明是个ASCII码
		return static_cast<GMWParam>(GM_KeyToASCII(key));
	}

	GMString xinputDlls[] = {
		"xinput9_1_0.dll",
		"xinput1_4.dll",
		"xinput1_3.dll",
	};

	const GMString& localeToCode(GMKeyboardLayout layout)
	{
		Array<GMString, GMKL_Unknown> s_arr;
		static std::once_flag s_flag;
		std::call_once(s_flag, [](auto& arr){
			arr[GMKL_Albanian] = L"0000041C";
			arr[GMKL_Arabic_101] = L"00000401";
			arr[GMKL_Arabic_102] = L"00010401";
			arr[GMKL_Arabic_102Azerty] = L"00020401";
			arr[GMKL_Armenian_eastern] = L"0000042B";
			arr[GMKL_Armenian_Western] = L"0001042B";
			arr[GMKL_Assamese__inscript] = L"0000044D";
			arr[GMKL_Azeri_Cyrillic] = L"0000082C";
			arr[GMKL_Azeri_Latin] = L"0000042C";
			arr[GMKL_Bashkir] = L"0000046D";
			arr[GMKL_Belarusian] = L"00000423";
			arr[GMKL_Belgian_French] = L"0000080C";
			arr[GMKL_Belgian_period] = L"00000813";
			arr[GMKL_Belgian_comma] = L"0001080C";
			arr[GMKL_Bengali] = L"00000445";
			arr[GMKL_Bengali__inscriptlegacy] = L"00010445";
			arr[GMKL_Bengali__inscript] = L"00020445";
			arr[GMKL_Bosnian_cyrillic] = L"0000201A";
			arr[GMKL_Bulgarian] = L"00030402";
			arr[GMKL_Bulgarian_typewriter] = L"00000402";
			arr[GMKL_Bulgarian_latin] = L"00010402";
			arr[GMKL_Bulgarian_phonetic] = L"00020402";
			arr[GMKL_Bulgarian_phonetictraditional] = L"00040402";
			arr[GMKL_Canada_Multilingual] = L"00011009";
			arr[GMKL_Canada_French] = L"00001009";
			arr[GMKL_Canada_Frenchlegacy] = L"00000C0C";
			arr[GMKL_Chinese_traditional_uskeyboard] = L"00000404";
			arr[GMKL_Chinese_simplified_uskeyboard] = L"00000804";
			arr[GMKL_Chinese_traditional_hongkongs_uskeyboard] = L"00000C04";
			arr[GMKL_Chinese_simplified_singapore_uskeyboard] = L"00001004";
			arr[GMKL_Chinese_traditional_macaos_uskeyboard] = L"00001404";
			arr[GMKL_Czech] = L"00000405";
			arr[GMKL_Czech_programmers] = L"00020405";
			arr[GMKL_Czech_qwerty] = L"00010405";
			arr[GMKL_Croatian] = L"0000041A";
			arr[GMKL_Deanagari__inscript] = L"00000439";
			arr[GMKL_Danish] = L"00000406";
			arr[GMKL_Divehi_phonetic] = L"00000465";
			arr[GMKL_Divehi_typewriter] = L"00010465";
			arr[GMKL_Dutch] = L"00000413";
			arr[GMKL_Estonian] = L"00000425";
			arr[GMKL_Faeroese] = L"00000438";
			arr[GMKL_Finnish] = L"0000040B";
			arr[GMKL_Finnish_withsami] = L"0001083B";
			arr[GMKL_French] = L"0000040C";
			arr[GMKL_Gaelic] = L"00011809";
			arr[GMKL_Georgian] = L"00000437";
			arr[GMKL_Georgian_ergonomic] = L"00020437";
			arr[GMKL_Georgian_qwerty] = L"00010437";
			arr[GMKL_German] = L"00000407";
			arr[GMKL_German_ibm] = L"00010407";
			arr[GMKL_Greenlandic] = L"0000046F";
			arr[GMKL_Hausa] = L"00000468";
			arr[GMKL_Hebrew] = L"0000040D";
			arr[GMKL_Hindi_traditional] = L"00010439";
			arr[GMKL_Greek] = L"00000408";
			arr[GMKL_Greek_220] = L"00010408";
			arr[GMKL_Greek_220latin] = L"00030408";
			arr[GMKL_Greek_319] = L"00020408";
			arr[GMKL_Greek_319latin] = L"00040408";
			arr[GMKL_Greek_latin] = L"00050408";
			arr[GMKL_Greek_polyonic] = L"00060408";
			arr[GMKL_Gujarati] = L"00000447";
			arr[GMKL_Hungarian] = L"0000040E";
			arr[GMKL_Hungarian_101key] = L"0001040E";
			arr[GMKL_Icelandic] = L"0000040F";
			arr[GMKL_Igbo] = L"00000470";
			arr[GMKL_Inuktitut__latin] = L"0000085D";
			arr[GMKL_Inuktitut__naqittaut] = L"0001045D";
			arr[GMKL_Irish] = L"00001809";
			arr[GMKL_Italian] = L"00000410";
			arr[GMKL_Italian_142] = L"00010410";
			arr[GMKL_Japanese] = L"00000411";
			arr[GMKL_Kannada] = L"0000044B";
			arr[GMKL_Kazakh] = L"0000043F";
			arr[GMKL_Khmer] = L"00000453";
			arr[GMKL_Korean] = L"00000412";
			arr[GMKL_Kyrgyz_cyrillic] = L"00000440";
			arr[GMKL_Lao] = L"00000454";
			arr[GMKL_Latin_america] = L"0000080A";
			arr[GMKL_Latvian] = L"00000426";
			arr[GMKL_Latvian_qwerty] = L"00010426";
			arr[GMKL_Lithuanian] = L"00010427";
			arr[GMKL_Lithuanian_ibm] = L"00000427";
			arr[GMKL_Lithuanian_standard] = L"00020427";
			arr[GMKL_Luxembourgish] = L"0000046E";
			arr[GMKL_Macedonian_fyrom] = L"0000042F";
			arr[GMKL_Macedonian_fyrom_standard] = L"0001042F";
			arr[GMKL_Malayalam] = L"0000044C";
			arr[GMKL_Maltese_47_key] = L"0000043A";
			arr[GMKL_Maltese_48_key] = L"0001043A";
			arr[GMKL_Marathi] = L"0000044E";
			arr[GMKL_Maroi] = L"00000481";
			arr[GMKL_Mongolian_cyrillic] = L"00000450";
			arr[GMKL_Mongolian_mongolianscript] = L"00000850";
			arr[GMKL_Nepali] = L"00000461";
			arr[GMKL_Norwegian] = L"00000414";
			arr[GMKL_Norwegian_withsami] = L"0000043B";
			arr[GMKL_Oriya] = L"00000448";
			arr[GMKL_Pashto_afghanistan] = L"00000463";
			arr[GMKL_Persian] = L"00000429";
			arr[GMKL_Polish_programmers] = L"00000415";
			arr[GMKL_Polish_214] = L"00010415";
			arr[GMKL_Portuguese] = L"00000816";
			arr[GMKL_Portuguese_brazillianabnt] = L"00000416";
			arr[GMKL_Portuguese_brazillianabnt2] = L"00010416";
			arr[GMKL_Punjabi] = L"00000446";
			arr[GMKL_Romanian_standard] = L"00010418";
			arr[GMKL_Romanian_legacy] = L"00000418";
			arr[GMKL_Romanian_programmers] = L"00020418";
			arr[GMKL_Russian] = L"00000419";
			arr[GMKL_Russian_typewriter] = L"00010419";
			arr[GMKL_Sami_extendedfinland_sweden] = L"0002083B";
			arr[GMKL_Sami_extendednorway] = L"0001043B";
			arr[GMKL_Serbian_cyrillic] = L"00000C1A";
			arr[GMKL_Serbian_latin] = L"0000081A";
			arr[GMKL_Sesotho_saLeboa] = L"0000046C";
			arr[GMKL_Setswana] = L"00000432";
			arr[GMKL_Sinhala] = L"0000045B";
			arr[GMKL_Sinhala__Wij9] = L"0001045B";
			arr[GMKL_Slovak] = L"0000041B";
			arr[GMKL_Slovak_qwerty] = L"0001041B";
			arr[GMKL_Slovenian] = L"00000424";
			arr[GMKL_Sorbian_extended] = L"0001042E";
			arr[GMKL_Sorbian_standard] = L"0002042E";
			arr[GMKL_Sorbian_standardlegacy] = L"0000042E";
			arr[GMKL_Spanish] = L"0000040A";
			arr[GMKL_Spanish_variation] = L"0001040A";
			arr[GMKL_Swedish] = L"0000041D";
			arr[GMKL_Swedish_withsami] = L"0000083B";
			arr[GMKL_Swiss_german] = L"00000807";
			arr[GMKL_Swiss_french] = L"0000100C";
			arr[GMKL_Syriac] = L"0000045A";
			arr[GMKL_Syriac_phonetic] = L"0001045A";
			arr[GMKL_Tajik] = L"00000428";
			arr[GMKL_Tamil] = L"00000449";
			arr[GMKL_Tatar] = L"00000444";
			arr[GMKL_Telugu] = L"0000044A";
			arr[GMKL_Thai_Kedmanee] = L"0000041E";
			arr[GMKL_Thai_Kedmaneenon_shiftlock] = L"0002041E";
			arr[GMKL_Thai_Pattachote] = L"0001041E";
			arr[GMKL_Thai_Pattachotenon_shiftlock] = L"0003041E";
			arr[GMKL_Tibetan_prc] = L"00000451";
			arr[GMKL_Turkish_F] = L"0001041F";
			arr[GMKL_Turkish_Q] = L"0000041F";
			arr[GMKL_Turkmen] = L"00000442";
			arr[GMKL_Ukrainian] = L"00000422";
			arr[GMKL_Ukrainian_enhanced] = L"00020422";
			arr[GMKL_United_Kingdom] = L"00000809";
			arr[GMKL_United_KingdomExtended] = L"00000452";
			arr[GMKL_United_States] = L"00000409";
			arr[GMKL_United_States_dvorak] = L"00010409";
			arr[GMKL_United_States_dvoraklefthand] = L"00030409";
			arr[GMKL_United_States_dvorakrighthand] = L"00050409";
			arr[GMKL_United_States_india] = L"00004009";
			arr[GMKL_United_States_international] = L"00020409";
			arr[GMKL_Urdu] = L"00000420";
			arr[GMKL_Uyghur] = L"00010480";
			arr[GMKL_Uyghur_legacy] = L"00000480";
			arr[GMKL_Uzbek_cyrillic] = L"00000843";
			arr[GMKL_Vietnamese] = L"0000042A";
			arr[GMKL_Yakut] = L"00000485";
			arr[GMKL_Yoruba] = L"0000046A";
			arr[GMKL_Wolof] = L"00000488";
		}, s_arr);

		return s_arr[layout];
	}
}

namespace gm
{
	class JoystickStateImpl : public IJoystickState
	{
	public:
		JoystickStateImpl(GMInput* host) { m_host = host; }
		virtual void vibrate(GMushort leftMotorSpeed, GMushort rightMotorSpeed) override;
		virtual GMJoystickState state() override;
	private:
		GMInput* m_host = nullptr;
	};

	class MouseStateImpl : public IMouseState
	{
	public:
		MouseStateImpl(GMInput* host) { m_host = host; }
		virtual GMMouseState state() override;
		virtual void setDetectingMode(bool center) override;
	private:
		GMInput* m_host = nullptr;
	};

	class KeyboardStateImpl : public IKeyboardState
	{
	public:
		KeyboardStateImpl(GMInput* host) { m_host = host; }
		virtual bool keydown(GMKey key) override;
		virtual bool keyTriggered(GMKey key) override;
	private:
		GMInput* m_host = nullptr;
	};

	class IMStateImpl : public IIMState
	{
	public:
		virtual void activate(GMKeyboardLayout layout);
	};
}

GMXInputWrapper::GMXInputWrapper()
	: m_xinputGetState(nullptr)
	, m_xinputSetState(nullptr)
	, m_module(0)
{
	for (GMint32 i = 0; i < 3; i++)
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
	d->joystickImpl = new JoystickStateImpl(this);
	d->mouseImpl = new MouseStateImpl(this);
	d->keyboardImpl = new KeyboardStateImpl(this);
	d->imImpl = new IMStateImpl();
}

GMInput::~GMInput()
{
	D(d);
	GM_delete(d->mouseImpl);
	GM_delete(d->joystickImpl);
	GM_delete(d->keyboardImpl);
	GM_delete(d->imImpl);
}

void GMInput::update()
{
	D(d);
	::GetKeyboardState(d->lastKeyState);
	// restore
	d->wheelState.wheeled = false;
	d->wheelState.delta = 0;
	d->mouseState.downButton = d->mouseState.upButton = GMMouseButton_None;
	d->mouseState.moving = false;
}

IKeyboardState& GMInput::getKeyboardState()
{
	D(d);
	GetKeyboardState(d->keyState);
	GM_ASSERT(d->keyboardImpl);
	return *d->keyboardImpl;
}

IJoystickState& GMInput::getJoystickState()
{
	D(d);
	GM_ASSERT(d->joystickImpl);
	return *d->joystickImpl;
}

IMouseState& GMInput::getMouseState()
{
	D(d);
	GM_ASSERT(d->mouseImpl);
	return *d->mouseImpl;
}

IIMState& gm::GMInput::getIMState()
{
	D(d);
	GM_ASSERT(d->imImpl);
	return *d->imImpl;
}

void GMInput::handleSystemEvent(GMSystemEvent* event)
{
	GMSystemEventType type = event->getType();
	switch (type)
	{
	case GMSystemEventType::MouseWheel:
	{
		GMSystemMouseWheelEvent* e = static_cast<GMSystemMouseWheelEvent*>(event);
		recordWheel(true, e->getDelta());
		break;
	}
	case GMSystemEventType::MouseMove:
	case GMSystemEventType::MouseDown:
	case GMSystemEventType::MouseUp:
	{
		GMSystemMouseEvent* e = static_cast<GMSystemMouseEvent*>(event);
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

//////////////////////////////////////////////////////////////////////////
void MouseStateImpl::setDetectingMode(bool enable)
{
	D_OF(d, m_host);
	if (enable)
	{
		const GMRect& rect = d->window->getWindowRect();
		::SetCursorPos(rect.x + rect.width / 2, rect.y + rect.height / 2);
		::ShowCursor(FALSE);
	}
	else
	{
		::ShowCursor(TRUE);
	}

	d->detectingMode = enable;
}

GMMouseState MouseStateImpl::state()
{
	D_OF(d, m_host);
	GMMouseState& state = d->mouseState;
	state.wheeled = d->wheelState.wheeled;
	state.wheeledDelta = static_cast<GMint32>(d->wheelState.delta);

	POINT pos;
	::GetCursorPos(&pos);

	{
		POINT p = pos;
		::ScreenToClient(d->window->getWindowHandle(), &p);
		state.posX = p.x;
		state.posY = p.y;

		IKeyboardState& ks = m_host->getKeyboardState();
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
		const GMint32 centerX = rect.x + rect.width / 2;
		const GMint32 centerY = rect.y + rect.height / 2;
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

GMJoystickState JoystickStateImpl::state()
{
	D_OF(d, m_host);
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

void JoystickStateImpl::vibrate(GMushort leftMotorSpeed, GMushort rightMotorSpeed)
{
	D_OF(d, m_host);
	XINPUT_VIBRATION v = { leftMotorSpeed, rightMotorSpeed };
	d->xinput.XInputSetState(0, &v);
}

bool KeyboardStateImpl::keydown(GMKey key)
{
	D_OF(d, m_host);
	return !!(d->keyState[getWindowsKey(key)] & 0x80);
}

// 表示一个键是否按下一次，长按只算是一次
bool KeyboardStateImpl::keyTriggered(GMKey key)
{
	D_OF(d, m_host);
	return !(d->lastKeyState[getWindowsKey(key)] & 0x80) && (keydown(key));
}

void IMStateImpl::activate(GMKeyboardLayout layout)
{
	std::wstring code = localeToCode(layout).toStdWString();
	LoadKeyboardLayout(code.c_str(), KLF_ACTIVATE);
}