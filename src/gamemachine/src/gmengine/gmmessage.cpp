#include "stdafx.h"
#include "gmmessage.h"

BEGIN_NS

extern "C"
{
	GM_EXPORT GMKey GM_ASCIIToKey(GMbyte key)
	{
		switch (key)
		{
			case 27:
				return GMKey_Escape;
			case 13:
				return GMKey_Return;
			case 9:
				return GMKey_Tab;
			case 32:
				return GMKey_Space;
			case 8:
				return GMKey_Back;
			case 127:
				return GMKey_Delete;
			default:
				return static_cast<GMKey>(GMKey_ASCII + key);
		}
	}

	GM_EXPORT GMbyte GM_KeyToASCII(GMKey key)
	{
		switch (key)
		{
			case GMKey_Escape: 
				return 27;
			case GMKey_Return: 
				return 13;
			case GMKey_Tab: 
				return 9;
			case GMKey_Space: 
				return 32;
			case GMKey_Back: 
				return 8;
			case GMKey_Delete: 
				return 46;
			default:
				return static_cast<GMbyte>(key - GMKey_ASCII);
		}
	}
}

GMSystemEvent::GMSystemEvent(GMSystemEventType type)
{
	setType(type);
	message.object = this;
}

GMSystemKeyEvent::GMSystemKeyEvent(GMSystemEventType type, GMKey key, GMModifier modifier) : GMSystemEvent(type)
{
	this->key = key;
	this->modifier = modifier;
}

GMSystemCharEvent::GMSystemCharEvent(GMSystemEventType type, GMKey key, GMwchar character, GMModifier modifier) : GMSystemKeyEvent(type, key, modifier)
{
	this->character = character;
}

GMSystemMouseEvent::GMSystemMouseEvent(GMSystemEventType type, const GMPoint& pt, GMMouseButton button, GMMouseButton buttons, GMModifier modifier) : GMSystemEvent(type)
{
	this->point = pt;
	this->button = button;
	this->buttons = buttons;
	this->modifier = modifier;
}

GMSystemMouseWheelEvent::GMSystemMouseWheelEvent(const GMPoint& pt, GMMouseButton button, GMMouseButton buttons, GMModifier modifier, GMshort delta) : GMSystemMouseEvent(GMSystemEventType::MouseWheel, pt, button, buttons, modifier)
{
	this->delta = delta;
}

GMSystemCaptureChangedEvent::GMSystemCaptureChangedEvent(GMWindowHandle capturedWnd) : GMSystemEvent(GMSystemEventType::CaptureChanged)
{
	setCapturedWindow(capturedWnd);
}

END_NS