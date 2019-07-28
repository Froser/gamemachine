#include "stdafx.h"
#include "gmmessage.h"

extern "C"
{
	gm::GMKey GM_ASCIIToKey(gm::GMbyte key)
	{
		switch (key)
		{
			case 27:
				return gm::GMKey_Escape;
			case 13:
				return gm::GMKey_Return;
			case 9:
				return gm::GMKey_Tab;
			case 32:
				return gm::GMKey_Space;
			case 8:
				return gm::GMKey_Back;
			case 127:
				return gm::GMKey_Delete;
			default:
				return static_cast<gm::GMKey>(gm::GMKey_ASCII + key);
		}
	}

	gm::GMbyte GM_KeyToASCII(gm::GMKey key)
	{
		switch (key)
		{
			case gm::GMKey_Escape:
				return 27;
			case gm::GMKey_Return:
				return 13;
			case gm::GMKey_Tab:
				return 9;
			case gm::GMKey_Space:
				return 32;
			case gm::GMKey_Back:
				return 8;
			case gm::GMKey_Delete:
				return 46;
			default:
				return static_cast<gm::GMbyte>(key - gm::GMKey_ASCII);
		}
	}
}

BEGIN_NS

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