#ifndef __GMINPUT_H__
#define __GMINPUT_H__
#include <gmmessage.h>

#if _MSC_VER
#	include <Xinput.h>
#else
#	include <xinput_gm.h>
#endif

BEGIN_NS
enum GMKeyboardLayout
{
	GMKL_Albanian,
	GMKL_Arabic_101,
	GMKL_Arabic_102,
	GMKL_Arabic_102Azerty,
	GMKL_Armenian_eastern,
	GMKL_Armenian_Western,
	GMKL_Assamese__inscript,
	GMKL_Azeri_Cyrillic,
	GMKL_Azeri_Latin,
	GMKL_Bashkir,
	GMKL_Belarusian,
	GMKL_Belgian_French,
	GMKL_Belgian_period,
	GMKL_Belgian_comma,
	GMKL_Bengali,
	GMKL_Bengali__inscriptlegacy,
	GMKL_Bengali__inscript,
	GMKL_Bosnian_cyrillic,
	GMKL_Bulgarian,
	GMKL_Bulgarian_typewriter,
	GMKL_Bulgarian_latin,
	GMKL_Bulgarian_phonetic,
	GMKL_Bulgarian_phonetictraditional,
	GMKL_Canada_Multilingual,
	GMKL_Canada_French,
	GMKL_Canada_Frenchlegacy,
	GMKL_Chinese_traditional_uskeyboard,
	GMKL_Chinese_simplified_uskeyboard,
	GMKL_Chinese_traditional_hongkongs_uskeyboard,
	GMKL_Chinese_simplified_singapore_uskeyboard,
	GMKL_Chinese_traditional_macaos_uskeyboard,
	GMKL_Czech,
	GMKL_Czech_programmers,
	GMKL_Czech_qwerty,
	GMKL_Croatian,
	GMKL_Deanagari__inscript,
	GMKL_Danish,
	GMKL_Divehi_phonetic,
	GMKL_Divehi_typewriter,
	GMKL_Dutch,
	GMKL_Estonian,
	GMKL_Faeroese,
	GMKL_Finnish,
	GMKL_Finnish_withsami,
	GMKL_French,
	GMKL_Gaelic,
	GMKL_Georgian,
	GMKL_Georgian_ergonomic,
	GMKL_Georgian_qwerty,
	GMKL_German,
	GMKL_German_ibm,
	GMKL_Greenlandic,
	GMKL_Hausa,
	GMKL_Hebrew,
	GMKL_Hindi_traditional,
	GMKL_Greek,
	GMKL_Greek_220,
	GMKL_Greek_220latin,
	GMKL_Greek_319,
	GMKL_Greek_319latin,
	GMKL_Greek_latin,
	GMKL_Greek_polyonic,
	GMKL_Gujarati,
	GMKL_Hungarian,
	GMKL_Hungarian_101key,
	GMKL_Icelandic,
	GMKL_Igbo,
	GMKL_Inuktitut__latin,
	GMKL_Inuktitut__naqittaut,
	GMKL_Irish,
	GMKL_Italian,
	GMKL_Italian_142,
	GMKL_Japanese,
	GMKL_Kannada,
	GMKL_Kazakh,
	GMKL_Khmer,
	GMKL_Korean,
	GMKL_Kyrgyz_cyrillic,
	GMKL_Lao,
	GMKL_Latin_america,
	GMKL_Latvian,
	GMKL_Latvian_qwerty,
	GMKL_Lithuanian,
	GMKL_Lithuanian_ibm,
	GMKL_Lithuanian_standard,
	GMKL_Luxembourgish,
	GMKL_Macedonian_fyrom,
	GMKL_Macedonian_fyrom_standard,
	GMKL_Malayalam,
	GMKL_Maltese_47_key,
	GMKL_Maltese_48_key,
	GMKL_Marathi,
	GMKL_Maroi,
	GMKL_Mongolian_cyrillic,
	GMKL_Mongolian_mongolianscript,
	GMKL_Nepali,
	GMKL_Norwegian,
	GMKL_Norwegian_withsami,
	GMKL_Oriya,
	GMKL_Pashto_afghanistan,
	GMKL_Persian,
	GMKL_Polish_programmers,
	GMKL_Polish_214,
	GMKL_Portuguese,
	GMKL_Portuguese_brazillianabnt,
	GMKL_Portuguese_brazillianabnt2,
	GMKL_Punjabi,
	GMKL_Romanian_standard,
	GMKL_Romanian_legacy,
	GMKL_Romanian_programmers,
	GMKL_Russian,
	GMKL_Russian_typewriter,
	GMKL_Sami_extendedfinland_sweden,
	GMKL_Sami_extendednorway,
	GMKL_Serbian_cyrillic,
	GMKL_Serbian_latin,
	GMKL_Sesotho_saLeboa,
	GMKL_Setswana,
	GMKL_Sinhala,
	GMKL_Sinhala__Wij9,
	GMKL_Slovak,
	GMKL_Slovak_qwerty,
	GMKL_Slovenian,
	GMKL_Sorbian_extended,
	GMKL_Sorbian_standard,
	GMKL_Sorbian_standardlegacy,
	GMKL_Spanish,
	GMKL_Spanish_variation,
	GMKL_Swedish,
	GMKL_Swedish_withsami,
	GMKL_Swiss_german,
	GMKL_Swiss_french,
	GMKL_Syriac,
	GMKL_Syriac_phonetic,
	GMKL_Tajik,
	GMKL_Tamil,
	GMKL_Tatar,
	GMKL_Telugu,
	GMKL_Thai_Kedmanee,
	GMKL_Thai_Kedmaneenon_shiftlock,
	GMKL_Thai_Pattachote,
	GMKL_Thai_Pattachotenon_shiftlock,
	GMKL_Tibetan_prc,
	GMKL_Turkish_F,
	GMKL_Turkish_Q,
	GMKL_Turkmen,
	GMKL_Ukrainian,
	GMKL_Ukrainian_enhanced,
	GMKL_United_Kingdom,
	GMKL_United_KingdomExtended,
	GMKL_United_States,
	GMKL_United_States_dvorak,
	GMKL_United_States_dvoraklefthand,
	GMKL_United_States_dvorakrighthand,
	GMKL_United_States_india,
	GMKL_United_States_international,
	GMKL_Urdu,
	GMKL_Uyghur,
	GMKL_Uyghur_legacy,
	GMKL_Uzbek_cyrillic,
	GMKL_Vietnamese,
	GMKL_Yakut,
	GMKL_Yoruba,
	GMKL_Wolof,
	GMKL_Unknown,
};

struct IWindow;
struct GMJoystickState
{
	bool valid;
	GMWord buttons;
	GMbyte leftTrigger;
	GMbyte rightTrigger;
	GMshort thumbLX;
	GMshort thumbLY;
	GMshort thumbRX;
	GMshort thumbRY;
};

struct GMWheelState
{
	bool wheeled = false;
	GMshort delta = 0;
};

struct GMMouseState
{
	GMint32 deltaX;
	GMint32 deltaY;
	GMint32 posX;
	GMint32 posY;
	GMMouseButton downButton;
	GMMouseButton upButton;
	GMMouseButton triggerButton;
	bool wheeled;
	GMint32 wheeledDelta;
	bool moving;
};

GM_INTERFACE(IJoystickState)
{
	virtual void vibrate(GMushort leftMotorSpeed, GMushort rightMotorSpeed) = 0;
	virtual GMJoystickState state() = 0;
};

GM_INTERFACE(IKeyboardState)
{
	//! 返回某个键是否此时被按下。
	/*!
	  \param key 待测试的键。
	  \return 返回是否此键被按下。
	*/
	virtual bool keydown(GMKey key) = 0;

	//! 表示一个键是否按下一次，长按只算是一次
	/*!
	  \param key 按下的键。
	  \return 返回是否此键被按下一次。
	*/
	virtual bool keyTriggered(GMKey key) = 0;
};

GM_INTERFACE(IMouseState)
{
	virtual void setDetectingMode(bool enable) = 0;
	virtual GMMouseState state() = 0;
};

GM_INTERFACE(IIMState)
{
	virtual void activate(GMKeyboardLayout layout) = 0;
};

GM_INTERFACE(IInput)
{
	virtual void update() = 0;
	virtual IKeyboardState& getKeyboardState() = 0;
	virtual IJoystickState& getJoystickState() = 0;
	virtual IMouseState& getMouseState() = 0;
	virtual IIMState& getIMState() = 0;
	virtual void handleSystemEvent(GMSystemEvent* event) = 0;
};
END_NS
#endif