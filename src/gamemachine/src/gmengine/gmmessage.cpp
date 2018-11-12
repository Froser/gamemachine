#include "stdafx.h"
#include "gmmessage.h"

extern "C"
{
	GMKey GM_ASCIIToKey(GMbyte key)
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

	GMbyte GM_KeyToASCII(GMKey key)
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
	D(d);
	setType(type);
	d->message.object = this;
}